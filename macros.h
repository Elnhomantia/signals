#ifndef SIGNAL_MACROS_H
#define SIGNAL_MACROS_H

#define SIGNAL_CONNECT_FORWARD(name, ...) \
    template<typename Method> \
    [[nodiscard("rvalue must be kept, else will auto disconnect immediatly.")]] \
    inline Connection<__VA_ARGS__> connect_##name(Method&& method) { \
        return name##_.connect(std::forward<Method>(method)); \
    } \
 \
    template<typename T, typename Method> \
    [[nodiscard("rvalue must be kept, else will auto disconnect immediatly.")]] \
    inline Connection<__VA_ARGS__> connect_##name(T* instance, Method&& method) { \
        return name##_.connect(instance, std::forward<Method>(method)); \
    } \
 \
    template<typename T, typename Method> \
    [[nodiscard("rvalue must be kept, else will auto disconnect immediatly.")]] \
    inline Connection<__VA_ARGS__> connect_##name(std::shared_ptr<T>& instance, Method&& method) { \
        return name##_.connect(instance, std::forward<Method>(method)); \
    } \
 \
    template<typename Method, typename... BoundArgs> \
    [[nodiscard("rvalue must be kept, else will auto disconnect immediatly.")]] \
    inline Connection<__VA_ARGS__> connect_##name(Method&& method, BoundArgs&&... boundArgs) { \
        return name##_.connect(std::forward<Method>(method), std::forward<BoundArgs>(boundArgs)...); \
} \
 \
    template<typename T, typename Method, typename... BoundArgs> \
    [[nodiscard("rvalue must be kept, else will auto disconnect immediatly.")]] \
    inline Connection<__VA_ARGS__> connect_##name(T* instance, Method&& method, BoundArgs&&... boundArgs) { \
        return name##_.connect(instance, std::forward<Method>(method), std::forward<BoundArgs>(boundArgs)...); \
} \
 \
    template<typename T, typename Method, typename... BoundArgs> \
    [[nodiscard("rvalue must be kept, else will auto disconnect immediatly.")]] \
    inline Connection<__VA_ARGS__> connect_##name(std::shared_ptr<T>& instance, Method&& method, BoundArgs&&... boundArgs) { \
        return name##_.connect(instance, std::forward<Method>(method), std::forward<BoundArgs>(boundArgs)...); \
} \


#define public_signal(name, ...) \
public: \
SIGNAL_CONNECT_FORWARD(name, __VA_ARGS__) \
private: \
    Signal<__VA_ARGS__> name##_;

#define private_signal(name, ...) \
protected: \
    Signal<__VA_ARGS__> name##_;

#define protected_signal(name, ...) \
private: \
    Signal<__VA_ARGS__> name##_;

#endif //SIGNAL_MACROS_H
