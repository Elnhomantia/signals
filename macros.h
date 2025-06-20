#ifndef SIGNAL_MACROS_H
#define SIGNAL_MACROS_H

/**
 * @def SIGNAL_CONNECT_FORWARD
 * @brief Project all connect function from signal.
 * Projected functions will be named : connect_<name>
 * @param name Name of an existing signal.
 * @param ... Parameter pack of the signal.
 */
#define SIGNAL_CONNECT_FORWARD(name, ...) \
    template<typename Method> \
    inline Connection<__VA_ARGS__> connect_##name(Method&& method) { \
        return name.connect(std::forward<Method>(method)); \
    } \
 \
    template<typename T, typename Method> \
    inline Connection<__VA_ARGS__> connect_##name(T* instance, Method&& method) { \
        return name.connect(instance, std::forward<Method>(method)); \
    } \
 \
    template<typename T, typename Method> \
    inline Connection<__VA_ARGS__> connect_##name(std::shared_ptr<T>& instance, Method&& method) { \
        return name.connect(instance, std::forward<Method>(method)); \
    } \
 \
    template<typename Method, typename... BoundArgs> \
    inline Connection<__VA_ARGS__> connect_##name(Method&& method, BoundArgs&&... boundArgs) { \
        return name.connect(std::forward<Method>(method), std::forward<BoundArgs>(boundArgs)...); \
} \
 \
    template<typename T, typename Method, typename... BoundArgs> \
    inline Connection<__VA_ARGS__> connect_##name(T* instance, Method&& method, BoundArgs&&... boundArgs) { \
        return name.connect(instance, std::forward<Method>(method), std::forward<BoundArgs>(boundArgs)...); \
} \
 \
    template<typename T, typename Method, typename... BoundArgs> \
    inline Connection<__VA_ARGS__> connect_##name(std::shared_ptr<T>& instance, Method&& method, BoundArgs&&... boundArgs) { \
        return name.connect(instance, std::forward<Method>(method), std::forward<BoundArgs>(boundArgs)...); \
}

/**
 * @def public_signal
 * @brief Define a new signal as private and forward all connect functions as public.
 * Projected functions will be named : connect_<name>
 * @param name Name that will be given to the signal
 * @param ... Parameter pack of the signal.
 */
#define public_signal(name, ...) \
public: \
    SIGNAL_CONNECT_FORWARD(name, __VA_ARGS__) \
private: \
    Signal<__VA_ARGS__> name;

/**
 * @def public_signal
 * @brief Define a new signal as private and forward all connect functions as protected.
 * Projected functions will be named : connect_<name>
 * @param name Name that will be given to the signal
 * @param ... Parameter pack of the signal.
 */
#define protected_signal(name, ...) \
protected: \
    SIGNAL_CONNECT_FORWARD(name, __VA_ARGS__) \
private: \
    Signal<__VA_ARGS__> name;

#endif //SIGNAL_MACROS_H
