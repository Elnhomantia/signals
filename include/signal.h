#ifndef SIGNAL_H
#define SIGNAL_H

#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>

template<typename... Args>
class Signal;

/**
 * @brief The Connection class is a de-connection manager.
 * It's a result of all connect function from @ref Signal.
 * The generated Connection must be kept, else it could disconnect automatically because of its destructor.
 * @tparam Args Signal parameters.
 */
template<typename... Args>
class [[nodiscard ("rvalue must be kept, else will directly disconnect")]] Connection
{
friend class Signal<Args...>;
using idType = Signal<Args...>::idType;

public:
    /**
     * @brief Connection base constructor.
     */
    Connection() = default;

    /**
     * @brief Deleted. No copy.
     */
    Connection(const Connection &) = delete;

    /**
     * @brief Connection move constructor.
     * @param other Another connection object. Will loose it's properties.
     */
    Connection(Connection && other) noexcept : id(other.id), sig(other.sig)
    {
        other.sig = nullptr;
    }

    /**
     * @brief Deleted. No copy.
     */
    Connection & operator=(const Connection &) = delete;

    /**
     * @brief operator= Move operator.
     * @param other Another connection object. Will loose it's properties.
     * @return Itself afet move.
     */
    Connection & operator=(Connection && other) noexcept
    {
        if(this != &other)
        {
            delete this->sig;
            this->id = other.id;
            this->sig = other.sig;
            other.sig = nullptr;
        }
        return *this;
    }

    /**
     * @brief Class destructor will automatically disconnect when deleted/unpiled.
     */
    ~Connection()
    {
        this->disconnect();
    }

    /**
     * @brief disconnect Unregister method from signal. It won't be call again during an @ref Signal::emit().
     */
    void disconnect()
    {
        if(this->sig != nullptr)
        {
            this->sig->disconnect(this->id);
        }
    }

    /**
     * @brief Let you block a method so it won't be called during @ref Signal::emit().
     */
    void block()
    {
        this->sig->setBlocked(this->id, true);
    }

    /**
     * @brief Let you unblock a method so it will be called again during @ref Signal::emit().
     */
    void unblock()
    {
        this->sig->setBlocked(this->id, false);
    }

private:

    /**
     * @brief Connection contructor for @ref Signal class.
     * @param s Signal.
     * @param id Signal id.
     */
    Connection(Signal<Args...> * s, idType id) : sig(s), id(id) {}

    /**
     * @brief id Method id.
     */
    idType id;
    /**
     * @brief sig Pointer to signal.
     */
    Signal<Args...> * sig;

};

namespace SignalConcepts {

    /**
     * @brief Verify that a method is invocable with @ref Signal arguments.
     * @tparam Method Method to verify.
     * @tparam AllArgs Parameters of the method.
     */
    template <typename Method, typename... AllArgs>
    concept ValidMethod =
        std::is_invocable_v<std::remove_reference_t<Method>, AllArgs...>
    ;

/**
     * @brief Verify that a method is invocable with @ref Signal arguments et that it's a class method.
     * @tparam T The class that must contain the method.
     * @tparam Method Method to verify.
     * @tparam AllArgs Parameters of the method.
     */
    template <typename T, typename Method, typename ...AllArgs>
    concept ValidClassMethod =
        std::is_invocable_v<std::remove_reference_t<Method>, T*, AllArgs...>
     && std::is_member_function_pointer_v<std::decay_t<Method>>
    ;
}

/**
 * @brief The Signal class. This is the Qt way to make the observer/observable pattern.
 * @tparam Args All arguments that will be emited by the signal.
 */
template<typename... Args>
class Signal
{
friend class Connection<Args...>;
/**
 * @brief Alias for signal id type.
 */
using idType = unsigned int;

/**
 * @brief Representation to know if a method should be called when a signal is emitted. See @ref Signal::emit().
 */
struct MethodType
{
    std::function<void(Args...)> func;
    bool isBlocked = false;
};

public:
    /**
     * @brief Signal Default constructor.
     */
    Signal() = default;

    /**
     * @brief Connect a static method or a lambda to a signal.
     * @param method Static method or lambda.
     * @return A @ref Connection. Must be kept or the signal might be automatically disconected.
     *
     * @code{.cpp}
     * void f(int){ ... }
     *
     * void main() {
     *  Signal<int> s;
     *  Foo foo;
     *  Connection c = s.connect(&f);
     *  //or
     *  Connection c = s.connect(f);
     *  //or
     *  Connection c = s.connect([](int){ ... });
     * }
     * @endcode
     */
    template<typename Method>
    requires SignalConcepts::ValidMethod<Method, Args...>
    Connection<Args...> connect(Method&& method) noexcept
    {
        idType id = this->getNewId();
        this->addMethod(id, std::move(method));
        return Connection(this, id);
    }

    /**
     * @brief Connect a class method to a signal.
     * @param instance Class object.
     * @param method Class method.
     * @return A @ref Connection. Must be kept or the signal might be automatically disconected.
     *
     * @code{.cpp}
     * class Foo {
     *  void f(int){ ... }
     * }
     *
     * void main() {
     *  Signal<int> s;
     *  Foo foo;
     *  Connection c = s.connect(&foo, &Foo::f);
     * }
     * @endcode
     */
    template<typename T, typename Method>
    requires SignalConcepts::ValidClassMethod<T, Method, Args...>
    Connection<Args...> connect(T* instance, Method&& method) noexcept
    {
        auto bound = [instance, method](Args... args) -> void { (instance->*method)(args...); };

        return connect(std::move(bound));
    }

    /**
     * @brief Connect a class method to a signal. Will auto disconnect if instance is not valid anymore.
     * @param instance Shared pointer to the class object.
     * @param method The class method.
     * @return A @ref Connection. Must be kept or the signal might be automatically disconected.
     *
     * @code{.cpp}
     * class Foo {
     *  void f(int){ ... }
     * }
     *
     * void main() {
     *  Signal<int> s;
     *  std::shared_ptr<Foo> foo = std::make_shared<Foo>();
     *  Connection c = s.connect(foo, &Foo::f);
     * }
     * @endcode
     */
    template<typename T, typename Method>
    requires SignalConcepts::ValidClassMethod<T, Method, Args...>
    Connection<Args...> connect(std::shared_ptr<T>& instance, Method&& method) noexcept
    {
        std::weak_ptr<T> wp(instance);
        idType id = this->getNewId();

        auto bound = [wp, method = std::forward<Method>(method), id, this](Args... args) {
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

        this->addMethod(id, std::move(bound));
        return Connection<Args...>(this, id);
    }

    /**
     * @brief Connect a method and bound its aguments from left to right.
     * @param method Static method or lambda. Must return void and have parameters to bind and then the same parameters as the signal.
     * @param boundArgs Values to bound arguments to. Be aware that if you want to bound a ref you should cast it explicitly with std::ref().
     * @return A @ref Connection. Must be kept or the signal might be automatically disconected.
     *
     * @code{.cpp}
     * void f(string, int){ ... }
     *
     * void main() {
     *  Signal<int> s;
     *  Foo foo;
     *  Connection c = s.connect(&f, "bar");
     *  //or
     *  Connection c = s.connect(f, "bar");
     *  //or
     *  Connection c = s.connect([](int){ ... }, "bar");
     * }
     * @endcode
     */
    template<typename Method, typename... BoundArgs>
    requires SignalConcepts::ValidMethod<Method, BoundArgs..., Args...>
    Connection<Args...> connect(Method&& method, BoundArgs&&... boundArgs) noexcept
    {
        auto bound = std::bind_front(method, std::forward<BoundArgs>(boundArgs)...);
        return connect(std::move(bound));
    }

    /**
     * @brief Connect a class method and bound its aguments from left to right.
     * @param instance Class instance.
     * @param method Class method. Must return void and have parameters to bind and then the same parameters as the signal.
     * @param boundArgs Values to bound arguments to. Be aware that if you want to bound a ref you should cast it explicitly with std::ref().
     * @return A @ref Connection. Must be kept or the signal might be automatically disconected.
     *
     * @code{.cpp}
     * class Foo {
     *  void f(string, int){ ... }
     * }
     *
     * void main() {
     *  Signal<int> s;
     *  Foo foo;
     *  Connection c = s.connect(&foo, &foo::f, "bar");
     * }
     * @endcode
     */
    template<typename T, typename Method, typename... BoundArgs>
    requires SignalConcepts::ValidClassMethod<T, Method, BoundArgs..., Args...>
    Connection<Args...> connect(T* instance, Method&& method, BoundArgs&&... boundArgs)
    {
        auto bound = std::bind_front(method, instance, std::forward<BoundArgs>(boundArgs)...);
        return connect(std::move(bound));
    }

    /**
     * @brief Connect a class method and bound its aguments from left to right.
     *  Will auto disconnect if instance is not valid anymore.
     * @param instance Shared pointer to class instance.
     * @param method Class method. Must return void and have parameters to bind and then the same parameters as the signal.
     * @param boundArgs Values to bound arguments to. Be aware that if you want to bound a ref you should cast it explicitly with std::ref().
     * @return A @ref Connection. Must be kept or the signal might be automatically disconected.
     *
     * @code{.cpp}
     * class Foo {
     *  void f(string, int){ ... }
     * }
     *
     * void main() {
     *  Signal<int> s;
     *  std::shared_ptr<Foo> foo = std::make_shared<Foo>();
     *  Connection c = s.connect(foo, &foo::f, "bar");
     * }
     * @endcode
     */
    template<typename T, typename Method, typename... BoundArgs>
    requires SignalConcepts::ValidClassMethod<T, Method, BoundArgs..., Args...>
    Connection<Args...> connect(std::shared_ptr<T>& instance, Method&& method, BoundArgs&&... boundArgs)
    {
        std::weak_ptr<T> wp(instance);
        idType id = this->getNewId();

        auto bound = [
            wp,
            method = std::move(method),
            ...boundArgs = std::forward<BoundArgs>(boundArgs),
            id, this]
            (Args... args)
        {
            if(auto sp = wp.lock())
            {
                ((*sp).*method)(boundArgs..., args...);
            }
            else
            {
                this->disconnect(id);
            }
        };
        this->addMethod(id, std::move(bound));
        return Connection<Args...>(this, id);
    }

    /**
     * @brief emit Call all connected methods.
     * @param args Signal parameters, same type as template.
     * @code
     * void main() {
     *  Signal<int> s;
     *  s.emit(1);
     * }
     * @endcode
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
     * @code
     * void main() {
     *  Signal<int> s;
     *  s.disconnectAll();
     * }
     * @endcode
     */
    void disconnectAll()
    {
        std::lock_guard<std::mutex> lock(mtx);
        this->methods.erase();
    }

private:
    /**
     * @brief nextId The next id to use. Won't backtrack if a connection if deleted for example.
     * Don't manipulate, use @ref Signal::getNewId()
     */
    idType nextId = 0;
    /**
     * @brief mtx Mutex for thread safety.
     */
    std::mutex mtx;
    /**
     * @brief Store methods with an id for de-connection.
     * Don't manipulate, use @ref Signal::addMethod(const idType id, Method&& method).
     */
    std::unordered_map<idType, MethodType> methods;

    /**
     * @brief Get next free id in a thread safe manner.
     * @return A free id.
     */
    idType getNewId()
    {
        std::unique_lock<std::mutex> lock(mtx);
        return this->nextId++;
    }

    /**
     * @brief Disconnect the method with the id key.
     * @param id Id of the method.
     */
    void disconnect(const idType id)
    {
        std::lock_guard<std::mutex> lock(mtx);
        this->methods.erase(id);
    }


    /**
     * @brief Add a method to be called by next @ref Signal::emit().
     * @param id Id of the method.
     * @param method Static function or lambda.
     */
    template <typename Method>
    void addMethod(const idType id, Method&& method)
    {
        std::unique_lock<std::mutex> lock(mtx);
        this->methods.emplace(id, MethodType(method));
    }


    /**
     * @brief Change if a method is blocked by id. a blocked method won't be called by @ref Signal::emit().
     * @param id Id of the method.
     * @param blocked true/false.
     */
    void setBlocked(const idType id, const bool blocked)
    {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = this->methods.find(id);
        if(it != this->methods.end())
        {
            it.second.isBlocked = blocked;
        }
    }
};

#include "macros.h"

#endif // SIGNAL_H
