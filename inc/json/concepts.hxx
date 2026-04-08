#pragma once

#include <json/forward.hxx>

#include <concepts>
#include <optional>
#include <set>
#include <type_traits>

namespace json
{
    template<typename T>
    concept node = std::same_as<std::decay_t<T>, Node>;

    template<typename T>
    concept node_value = std::same_as<std::decay_t<T>, NodeValue>;

    template<typename T, typename V>
    struct in_variant_t : std::false_type
    {
    };
    
    template<typename T, typename... E>
    struct in_variant_t<T, std::variant<E...>> : std::disjunction<std::is_same<T, E>...>
    {
    };

    template<typename T, typename V>
    concept in_variant = in_variant_t<T, V>::value;

    template<typename T>
    concept primitive = in_variant<std::decay_t<T>, NodeValue>;

    template<typename T>
    concept assignable = !node<T> && !node_value<T> && !primitive<T>;

    template<typename T>
    concept integral = std::integral<std::decay_t<T>> && !primitive<T>;

    template<typename T>
    concept floating_point = std::floating_point<std::decay_t<T>> && !primitive<T>;

    template<typename>
    struct is_vector_t : std::false_type
    {
    };

    template<typename... Args>
    struct is_vector_t<std::vector<Args...>> : std::true_type
    {
    };

    template<typename T>
    concept vector = is_vector_t<std::decay_t<T>>::value && !primitive<T>;

    template<typename>
    struct is_set_t : std::false_type
    {
    };

    template<typename... Args>
    struct is_set_t<std::set<Args...>> : std::true_type
    {
    };

    template<typename T>
    concept set = is_set_t<std::decay_t<T>>::value;

    template<typename>
    struct is_array_t : std::false_type
    {
    };

    template<typename T, std::size_t N>
    struct is_array_t<std::array<T, N>> : std::true_type
    {
    };

    template<typename T>
    concept array = is_array_t<std::decay_t<T>>::value;

    template<typename>
    struct is_map_t : std::false_type
    {
    };

    template<typename... Args>
    struct is_map_t<std::map<Args...>> : std::true_type
    {
    };

    template<typename T>
    concept map = is_map_t<std::decay_t<T>>::value && !primitive<T>;

    template<typename>
    struct is_optional_t : std::false_type
    {
    };

    template<typename... Args>
    struct is_optional_t<std::optional<Args...>> : std::true_type
    {
    };

    template<typename T>
    concept optional = is_optional_t<std::decay_t<T>>::value;

    template<typename>
    struct is_variant_t : std::false_type
    {
    };

    template<typename... Args>
    struct is_variant_t<std::variant<Args...>> : std::true_type
    {
    };

    template<typename T>
    concept variant = is_variant_t<std::decay_t<T>>::value;

    template<typename>
    struct serializer
    {
    };

    template<typename T>
    concept enable_from_json = requires(Node &node, T &value)
    {
        serializer<std::decay_t<T>>::from_json(node, value);
    };

    template<typename T>
    concept enable_to_json = requires(Node &node, T &&value)
    {
        serializer<std::decay_t<T>>::to_json(node, std::forward<T>(value));
    };
}
