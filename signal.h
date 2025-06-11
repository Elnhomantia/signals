#ifndef SIGNAL_H
#define SIGNAL_H

#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>

template<typename... Args>
class Signal;

template<typename... Args>
class Connection
{
friend class Signal<Args...>;
using idType = Signal<Args...>::idType;

public:
    Connection() = default;
    Connection(const Connection &) = delete;
    Connection(Connection && other) noexcept : id(other.id), sig(other.sig), isConnected(other.isConnected)
    {
        other.sig = nullptr;
    }

    Connection & operator=(const Connection &) = delete;
    Connection & operator=(Connection && other) noexcept
    {
        if(this != &other)
        {
            delete this->sig;
            this->isConnected = other.isConnected;
            this->id = other.id;
            this->sig = other.sig;
            other.sig = nullptr;
        }
        return *this;
    }

    ~Connection()
    {
        this->disconnect();
    }

    void disconnect()
    {
        if(this->isConnected && (this->sig != nullptr))
        {
            this->sig->disconnect(this->id);
            this->isConnected = false;
        }
    }

    void block()
    {
        this->sig->setBlocked(this->id, true);
    }
    void unblock()
    {
        this->sig->setBlocked(this->id, false);
    }

private:
    Connection(Signal<Args...> * s, idType id) : sig(s), id(id) {}

    bool isConnected;
    idType id;
    Signal<Args...> * sig;

};

namespace SignalConcepts {

    template <typename Method, typename... AllArgs>
    concept ValidMethod =
        std::is_invocable_v<std::remove_reference_t<Method>, AllArgs...>
    ;

    template <typename T, typename Method, typename ...AllArgs>
    concept ValidClassMethod =
        std::is_invocable_v<std::remove_reference_t<Method>, T*, AllArgs...> &&
        std::is_member_function_pointer_v<std::decay_t<Method>>
    ;
}

template<typename... Args>
class Signal
{
friend class Connection<Args...>;
using idType = unsigned int;



struct MethodType
{
    std::function<void(Args...)> func;
    bool isBlocked = false;
};

public:
    Signal() = default;

    /**
     * @brief Connect a free method or a lambda to a signal.
     * @param method Free method.
     */
    template<typename Method>
    requires SignalConcepts::ValidMethod<Method, Args...>
    Connection<Args...> connect(Method&& method) noexcept
    {
        std::lock_guard<std::mutex> lock(mtx);
        idType id = this->getNewId();
        this->methods.emplace(id, std::forward<Method>(method));
        return Connection(this, id);
    }

    /**
     * @brief Connect a class method to a signal.
     * @param instance Class object.
     * @param method Class method.
     */
    template<typename T, typename Method>
    requires SignalConcepts::ValidClassMethod<T, Method, Args...>
    Connection<Args...> connect(T* instance, Method&& method) noexcept
    {
        auto bound = [instance, method](Args... args) -> void {(instance->*method)(args...);};

        return connect(bound);
    }

    /**
     * @brief Connect a class method to a signal. Will auto disconnect if instance is not valid anymore.
     * @param instance Shared pointer to the class object.
     * @param method The class method.
     */
    template<typename T, typename Method>
    requires SignalConcepts::ValidClassMethod<T, Method, Args...>
    Connection<Args...> connect(std::shared_ptr<T> instance, Method&& method) noexcept
    {
        std::lock_guard<std::mutex> lock(mtx);
        std::weak_ptr<T> wp(instance);
        idType id = this->getNewId();

        auto bound = [wp, method, id, this](Args... args) {
            if(auto sp = wp.lock())
            {
                //instance can't become invalide here, we made a shared from a weak ptr.
                //=> we can be the last to reference the instance
                //=> instance will be destroy at the end of this fonction in that case
                //+ this lambda will be disconected next time.
                ((*sp).*method)(args...);
            }
            else
            {
                this->disconnect(id);
            }
        };

        this->methods.emplace(id, std::move(bound));
        return Connection<Args...>(this, id);
    }

    /**
     * @brief Connect a method and bound its aguments from left to right.
     * @param method Free method or lambda. Must return void and have the same parameters as the signal.
     * @param boundArgs Values to bound arguments to.
     */
    template<typename Method, typename... BoundArgs>
    requires SignalConcepts::ValidMethod<Method, Args..., BoundArgs...>
    Connection<Args...> connect(Method&& method, BoundArgs&&... boundArgs) noexcept
    {
        auto bound = std::bind_front(method, std::forward<BoundArgs>(boundArgs)...);
        return connect(std::move(bound));
    }

    /**
     * @brief Connect a class method and bound its aguments from left to right.
     * @param instance Class instance.
     * @param method Class method.
     * @param boundArgs Values to bound arguments to.
     */
    template<typename T, typename Method, typename... BoundArgs>
    requires SignalConcepts::ValidClassMethod<T, Method, Args..., BoundArgs...>
    Connection<Args...> connect(T* instance, Method&& method, BoundArgs&&... boundArgs)
    {
        auto bound = std::bind_front(method, instance, std::forward<BoundArgs>(boundArgs)...);
        return connect(std::move(bound));
    }

    /**
     * @brief Connect a class method and bound its aguments from left to right. Will auto disconnect if instance is not valid anymore.
     * @param instance Shared pointer to class instance.
     * @param method Class method.
     * @param boundArgs Values to bound arguments to.
     */
    template<typename T, typename Method, typename... BoundArgs>
    requires SignalConcepts::ValidClassMethod<T, Method, Args..., BoundArgs...>
    Connection<Args...> connect(std::shared_ptr<T> instance, Method&& method, BoundArgs&&... boundArgs)
    {
        auto bound = std::bind_front(method, std::forward<BoundArgs>(boundArgs)...);
        return connect(instance,[bound = std::move(bound)](T* self, Args... args){
            std::invoke(bound, self, args...);
        });
    }

    /**
     * @brief emit Call all connected methods.
     * @param args Parameters of registered methods.
     */
    void emit(Args... args)
    {
        std::unique_lock<std::mutex> lock(mtx);
        auto copy = this->methods;
        lock.unlock();
        for(auto & [id, method]: copy)
        {
            if(!method.isBlocked)
            {
                method.func(args...);
            }
        }
    }

    /**
     * @brief disconnectAll Disconnect all methods
     */
    void disconnectAll()
    {
        std::lock_guard<std::mutex> lock(mtx);
        this->methods.erase();
    }

private:
    idType nextId = 0;
    std::mutex mtx;
    std::unordered_map<idType, MethodType> methods;

    idType getNewId() { return this->nextId++; }

    void disconnect(idType id)
    {
        std::lock_guard<std::mutex> lock(mtx);
        this->methods.erase(id);
    }


    void setBlocked(idType id, bool blocked)
    {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = this->methods.find(id);
        if(it != this->methods.end())
        {
            it.second.isBlocked = blocked;
        }
    }
};

#endif // SIGNAL_H
