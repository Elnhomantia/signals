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
    Connection(Signal<Args...> * s, unsigned int id) : sig(s), id(id) {}

    bool isConnected;
    unsigned int id;
    Signal<Args...> * sig;

};

template<typename... Args>
class Signal
{
friend class Connection<Args...>;

struct SlotType{
    std::function<void(Args...)> func;
    bool isBlocked = false;
};

public:
    Signal();

    template<typename F>
    Connection<Args...> connect(F&& slot)
    {
        static_assert(std::is_invocable_r_v<void, F, Args...>,
                      "Slot function must be invocable with signal arguments and return void.");

        std::lock_guard<std::mutex> lock(mtx);
        unsigned int id = this->nextId++;
        this->slots.emplace(id, std::forward<F>(slot));
        return Connection(this, id);
    }

    template<typename T, typename Method>
    Connection<Args...> connect(T * instance, Method method)
    {
        static_assert(std::is_member_function_pointer_v<Method>,
                      "Second argument must be a pointer to a member function.");

        auto bound = [instance, method](Args... args) {(instance->*method)(args...);};

        return connect(bound);
    }

    template<typename T, typename Method>
    Connection<Args...> connect(std::shared_ptr<T> instance, Method method)
    {
        static_assert(std::is_invocable_r_v<void, Method, T*, Args...>,
                      "Slot function must be invocable with signal arguments and return void.");
        static_assert(std::is_member_function_pointer_v<Method>,
                      "Second argument must be a pointer to a member function.");

        std::lock_guard<std::mutex> lock(mtx);
        std::weak_ptr<T> wp(instance);
        unsigned int id = this->nextId++;

        auto bound = [wp, method, id, this](Args... args) {
            if(auto sp = wp.lock())
            {
                ((*sp).*method)(args...);
            }
            else
            {
                this->disconnect(id);
            }
        };

        this->slots.emplace(id, std::move(bound));
        return Connection<Args...>(this, id);
    }

    template<typename T, typename... BoundArgs>
    Connection<Args...> connect(void(T::*method)(BoundArgs..., Args...), T* instance, BoundArgs&&... boundArgs)
    {
        auto bound = std::bind_front(method, instance, std::forward<BoundArgs>(boundArgs)...);
        return connect(bound);
    }

    void emit(Args... args)
    {
        std::unique_lock<std::mutex> lock(mtx);
        auto copy = this->slots;
        lock.unlock();
        for(auto & [id, slot]: copy)
        {
            if(!slot.isBlocked)
            {
                slot.func(args...);
            }
        }
    }
    void disconnectAll()
    {
        std::lock_guard<std::mutex> lock(mtx);
        this->slots.erase();
    }

private:
    unsigned int nextId = 0;
    std::mutex mtx;
    std::unordered_map<unsigned int, SlotType> slots;

    void disconnect(unsigned int id)
    {
        std::lock_guard<std::mutex> lock(mtx);
        this->slots.erase(id);
    }


    void setBlocked(unsigned int id, bool blocked)
    {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = this->slots.find(id);
        if(it != this->slots.end())
        {
            it.second.isBlocked = blocked;
        }
    }
};

#endif // SIGNAL_H
