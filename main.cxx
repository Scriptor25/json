#include <iostream>
#include <sstream>

#include <json.hxx>

const auto json_string = R"({ "foo": [ null, true, 123 ], "bar": "Hello world!", "sub": { "foo": false, "bar": [] } })";

struct foo_t
{
    struct foo_tuple_t
    {
        std::optional<std::string> _0;
        bool _1{};
        long double _2{};
    } foo;

    std::string bar;

    struct sub_object_t
    {
        bool foo{};
        std::vector<std::string> bar;
    } sub;
};

template<>
bool from_json(const json::Node &node, foo_t::foo_tuple_t &value)
{
    if (!json::IsArray(node))
        return false;

    auto &array_node = json::AsArray(node);
    if (array_node.size() != 3)
        return false;

    auto ok = true;

    ok &= array_node[0] >> value._0;
    ok &= array_node[1] >> value._1;
    ok &= array_node[2] >> value._2;

    return ok;
}

template<>
bool from_json(const json::Node &node, foo_t::sub_object_t &value)
{
    if (!json::IsObject(node))
        return false;

    auto &object_node = json::AsObject(node);

    auto ok = true;

    ok &= object_node["foo"] >> value.foo;
    ok &= object_node["bar"] >> value.bar;

    return ok;
}

template<>
bool from_json(const json::Node &node, foo_t &value)
{
    if (!json::IsObject(node))
        return false;

    auto &object_node = json::AsObject(node);

    auto ok = true;

    ok &= object_node["foo"] >> value.foo;
    ok &= object_node["bar"] >> value.bar;
    ok &= object_node["sub"] >> value.sub;

    return ok;
}

int main()
{
    std::istringstream json_stream(json_string);

    json::Node node;
    json_stream >> node;

    foo_t foo;
    node >> foo;

    std::cout << json::compact << node << std::endl;
    std::cout << json::pretty << node << std::endl;

    return 0;
}
