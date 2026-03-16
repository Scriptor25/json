#include "json.hxx"
#include <iostream>

int main() {
  json::Node node = json::ObjectNode({
      {
          "foo",
          json::ArrayNode({
              json::NullNode(),
              json::BooleanNode(true),
              json::NumberNode(123),
          }),
      },
      {"bar", json::StringNode("Hello world!")},
      {
          "sub",
          json::ObjectNode({
              {"foo", json::BooleanNode(false)},
              {
                  "bar",
                  json::ArrayNode(),
              },
          }),
      },
  });

  std::cout << json::compact << node << std::endl;

  std::cout << json::pretty << node << std::endl;

  return 0;
}