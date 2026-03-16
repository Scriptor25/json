#pragma once

#include <map>
#include <ostream>
#include <string>
#include <variant>
#include <vector>

namespace json {

struct NullNode;
struct BooleanNode;
struct NumberNode;
struct StringNode;
struct ArrayNode;
struct ObjectNode;

using Node = std::variant<std::monostate, NullNode, BooleanNode, NumberNode,
                          StringNode, ArrayNode, ObjectNode>;

struct NullNode {
  NullNode() = default;

  NullNode(Node &&node);
  NullNode(const Node &node);

  std::ostream &Print(std::ostream &stream) const;
};

struct BooleanNode {
  BooleanNode() = default;

  BooleanNode(bool value);

  BooleanNode(Node &&node);
  BooleanNode(const Node &node);

  std::ostream &Print(std::ostream &stream) const;

  bool Value{};
};

struct NumberNode {
  NumberNode() = default;

  NumberNode(long double value);

  NumberNode(Node &&node);
  NumberNode(const Node &node);

  std::ostream &Print(std::ostream &stream) const;

  long double Value{};
};

struct StringNode {
  StringNode() = default;

  StringNode(const char *value);

  StringNode(std::string &&value);
  StringNode(const std::string &value);

  StringNode(Node &&node);
  StringNode(const Node &node);

  std::ostream &Print(std::ostream &stream) const;

  std::string Value{};
};

struct ArrayNode {
  ArrayNode() = default;

  ArrayNode(std::size_t count);

  ArrayNode(std::vector<Node> &&value);
  ArrayNode(const std::vector<Node> &value);

  ArrayNode(Node &&node);
  ArrayNode(const Node &node);

  std::ostream &Print(std::ostream &stream) const;

  std::vector<Node> Elements;
};

struct ObjectNode {
  ObjectNode() = default;

  ObjectNode(std::map<std::string, Node> &&value);
  ObjectNode(const std::map<std::string, Node> &value);

  ObjectNode(Node &&node);
  ObjectNode(const Node &node);

  std::ostream &Print(std::ostream &stream) const;

  std::map<std::string, Node> Elements;
};

bool IsUndefined(const Node &node);
bool IsNull(const Node &node);
bool IsBoolean(const Node &node);
bool IsNumber(const Node &node);
bool IsString(const Node &node);
bool IsArray(const Node &node);
bool IsObject(const Node &node);

const NullNode &AsNull(const Node &node);
const BooleanNode &AsBoolean(const Node &node);
const NumberNode &AsNumber(const Node &node);
const StringNode &AsString(const Node &node);
const ArrayNode &AsArray(const Node &node);
const ObjectNode &AsObject(const Node &node);

NullNode &AsNull(Node &node);
BooleanNode &AsBoolean(Node &node);
NumberNode &AsNumber(Node &node);
StringNode &AsString(Node &node);
ArrayNode &AsArray(Node &node);
ObjectNode &AsObject(Node &node);

std::ostream &compact(std::ostream &stream);
std::ostream &pretty(std::ostream &stream);

} // namespace json

std::ostream &operator<<(std::ostream &stream, const json::Node &node);
