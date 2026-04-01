#pragma once

#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json
{
    struct Node;

    using Index = std::size_t;
    using Key = std::string;

    using Undefined = std::monostate;
    using Null = std::nullptr_t;
    using Boolean = bool;
    using Number = long double;
    using String = std::string;
    using Array = std::vector<Node>;
    using Object = std::map<Key, Node>;

    using NodeValue = std::variant<
        Undefined,
        Null,
        Boolean,
        Number,
        String,
        Array,
        Object
    >;
}
