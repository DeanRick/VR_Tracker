#ifndef UTILS_H
#define UTILS_H

#define UNPACK_VECTOR(V) V.x, V.y, V.z
#define UNPACK_VECTOR_ARRAY(V) V[0], V[1], V[2]
#define UNPACK_QUATERNION(Q) Q.x, Q.y, Q.z, Q.w

#include <type_traits>

template <class T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
constexpr T operator|(T lhs, T rhs) {
    return static_cast<T>(
        static_cast<typename std::underlying_type<T>::type>(lhs)
        | static_cast<typename std::underlying_type<T>::type>(rhs)
    );
}

template <class T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
constexpr T operator&(T lhs, T rhs) {
    return static_cast<T>(
        static_cast<typename std::underlying_type<T>::type>(lhs)
        & static_cast<typename std::underlying_type<T>::type>(rhs)
    );
}

template <class T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
constexpr bool any(T t) {
    return static_cast<typename std::underlying_type<T>::type>(t) != 0;
}

#endif