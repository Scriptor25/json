#include <cstdlib>
#include <iomanip>

#include <json.hxx>

enum json_format {
  compact,
  pretty,
};

struct json_context {
  json_format format;
  unsigned depth;
};

static json_context &get_json_context(std::ostream &stream) {
  static int index = std::ios_base::xalloc();

  auto context = reinterpret_cast<json_context *>(stream.pword(index));
  if (!context) {
    context = new json_context({.format = compact, .depth = 0});
    stream.pword(index) = context;
  }

  return *context;
}

static std::ostream &depth_space(std::ostream &stream) {
  auto &context = get_json_context(stream);

  for (unsigned i = 0; i < context.depth; ++i) {
    stream << "  ";
  }

  return stream;
}

json::NullNode::NullNode(Node &&node) {
  if (!IsNull(node))
    throw std::runtime_error("error");

  node = {};
}

json::NullNode::NullNode(const Node &node) {
  if (!IsNull(node))
    throw std::runtime_error("error");
}

std::ostream &json::NullNode::Print(std::ostream &stream) const {
  return stream << "null";
}

json::BooleanNode::BooleanNode(bool value) : Value(value) {}

json::BooleanNode::BooleanNode(Node &&node) {
  if (!IsBoolean(node))
    throw std::runtime_error("error");

  Value = AsBoolean(node).Value;
  node = {};
}

json::BooleanNode::BooleanNode(const Node &node) {
  if (!IsBoolean(node))
    throw std::runtime_error("error");

  Value = AsBoolean(node).Value;
}

std::ostream &json::BooleanNode::Print(std::ostream &stream) const {
  return stream << (Value ? "true" : "false");
}

json::NumberNode::NumberNode(long double value) : Value(value) {}

json::NumberNode::NumberNode(Node &&node) {
  if (!IsNumber(node))
    throw std::runtime_error("error");

  Value = AsNumber(node).Value;
  node = {};
}

json::NumberNode::NumberNode(const Node &node) {
  if (!IsNumber(node))
    throw std::runtime_error("error");

  Value = AsNumber(node).Value;
}

std::ostream &json::NumberNode::Print(std::ostream &stream) const {
  return stream << std::scientific << Value;
}

json::StringNode::StringNode(const char *value) : Value(value) {}

json::StringNode::StringNode(std::string &&value) : Value(std::move(value)) {}

json::StringNode::StringNode(const std::string &value) : Value(value) {}

json::StringNode::StringNode(Node &&node) {
  if (!IsString(node))
    throw std::runtime_error("error");

  Value = std::move(AsString(node).Value);
  node = {};
}

json::StringNode::StringNode(const Node &node) {
  if (!IsString(node))
    throw std::runtime_error("error");

  Value = AsString(node).Value;
}

static std::size_t utf8_char_length(unsigned char c) {
  if ((c & 0b10000000) == 0b00000000)
    return 1;
  if ((c & 0b11100000) == 0b11000000)
    return 2;
  if ((c & 0b11110000) == 0b11100000)
    return 3;
  if ((c & 0b11111000) == 0b11110000)
    return 4;
  return 1;
}

static char32_t utf8_code_point(const std::string &s, std::size_t &i) {
  auto o = i;
  auto l = utf8_char_length(s[i]);
  i += l;

  switch (l) {
  case 1:
    return s[o];
  case 2:
    return (s[o] & 0x1F) << 6 | (s[o + 1] & 0x3F);
  case 3:
    return (s[o] & 0x0F) << 12 | (s[o + 1] & 0x3F) << 6 | (s[o + 2] & 0x3F);
  case 4:
    return (s[o] & 0x07) << 18 | (s[o + 1] & 0x3F) << 12 |
           (s[o + 2] & 0x3F) << 6 | (s[o + 3] & 0x3F);

  default:
    return s[o];
  }
}

std::ostream &json::StringNode::Print(std::ostream &stream) const {
  stream << '"';

  for (std::size_t i = 0; i < Value.size();) {

    auto c = utf8_code_point(Value, i);

    switch (c) {
    case '"':
      stream << "\\\"";
      break;
    case '\\':
      stream << "\\\\";
      break;
    case '\b':
      stream << "\\b";
      break;
    case '\f':
      stream << "\\f";
      break;
    case '\n':
      stream << "\\n";
      break;
    case '\r':
      stream << "\\r";
      break;
    case '\t':
      stream << "\\t";
      break;
    default:
      if (0x20 <= c && c < 0x7F)
        stream << static_cast<char>(c);
      else
        stream << "\\u" << std::hex << std::setw(4) << std::setfill('0')
               << static_cast<int>(c);
      break;
    }
  }

  return stream << '"';
}

json::ArrayNode::ArrayNode(std::size_t count) : Elements(count) {}

json::ArrayNode::ArrayNode(std::vector<Node> &&elements)
    : Elements(std::move(elements)) {}

json::ArrayNode::ArrayNode(const std::vector<Node> &elements)
    : Elements(elements) {}

json::ArrayNode::ArrayNode(Node &&node) {
  if (!IsArray(node))
    throw std::runtime_error("error");

  Elements = std::move(AsArray(node).Elements);
  node = {};
}

json::ArrayNode::ArrayNode(const Node &node) {
  if (!IsArray(node))
    throw std::runtime_error("error");

  Elements = AsArray(node).Elements;
}

std::ostream &json::ArrayNode::Print(std::ostream &stream) const {
  auto &context = get_json_context(stream);

  switch (context.format) {
  case json_format::pretty: {
    stream << '[';
    if (Elements.size() > 1) {
      stream << '\n';
    }
    context.depth++;
    for (auto it = Elements.begin(); it != Elements.end(); ++it) {
      if (it != Elements.begin()) {
        stream << ',' << '\n';
      }
      if (Elements.size() > 1) {
        stream << depth_space;
      }
      stream << *it;
    }
    context.depth--;
    if (Elements.size() > 1) {
      stream << '\n' << depth_space;
    }
    return stream << ']';
  }

  case json_format::compact:
  default: {
    stream << '[';
    context.depth++;
    for (auto it = Elements.begin(); it != Elements.end(); ++it) {
      if (it != Elements.begin()) {
        stream << ',';
      }
      stream << *it;
    }
    context.depth--;
    return stream << ']';
  }
  }
}

json::ObjectNode::ObjectNode(std::map<std::string, Node> &&elements)
    : Elements(std::move(elements)) {}

json::ObjectNode::ObjectNode(const std::map<std::string, Node> &elements)
    : Elements(elements) {}

json::ObjectNode::ObjectNode(Node &&node) {
  if (!IsObject(node))
    throw std::runtime_error("error");

  Elements = std::move(AsObject(node).Elements);
  node = {};
}

json::ObjectNode::ObjectNode(const Node &node) {
  if (!IsObject(node))
    throw std::runtime_error("error");

  Elements = AsObject(node).Elements;
}

std::ostream &json::ObjectNode::Print(std::ostream &stream) const {
  auto &context = get_json_context(stream);

  switch (context.format) {
  case json_format::pretty: {
    stream << '{' << '\n';
    context.depth++;
    for (auto it = Elements.begin(); it != Elements.end(); ++it) {
      if (it != Elements.begin()) {
        stream << ',' << '\n';
      }
      stream << depth_space << StringNode(it->first) << ": " << it->second;
    }
    context.depth--;
    return stream << '\n' << depth_space << '}';
  }

  case json_format::compact:
  default: {
    stream << '{';
    context.depth++;
    for (auto it = Elements.begin(); it != Elements.end(); ++it) {
      if (it != Elements.begin()) {
        stream << ',';
      }
      stream << StringNode(it->first) << ':' << it->second;
    }
    context.depth--;
    return stream << '}';
  }
  }
}

bool json::IsUndefined(const Node &node) {
  return std::holds_alternative<std::monostate>(node);
}

bool json::IsNull(const Node &node) {
  return std::holds_alternative<NullNode>(node);
}

bool json::IsBoolean(const Node &node) {
  return std::holds_alternative<BooleanNode>(node);
}

bool json::IsNumber(const Node &node) {
  return std::holds_alternative<NumberNode>(node);
}

bool json::IsString(const Node &node) {
  return std::holds_alternative<StringNode>(node);
}

bool json::IsArray(const Node &node) {
  return std::holds_alternative<ArrayNode>(node);
}

bool json::IsObject(const Node &node) {
  return std::holds_alternative<ObjectNode>(node);
}

const json::NullNode &json::AsNull(const Node &node) {
  return std::get<NullNode>(node);
}

const json::BooleanNode &json::AsBoolean(const Node &node) {
  return std::get<BooleanNode>(node);
}

const json::NumberNode &json::AsNumber(const Node &node) {
  return std::get<NumberNode>(node);
}

const json::StringNode &json::AsString(const Node &node) {
  return std::get<StringNode>(node);
}

const json::ArrayNode &json::AsArray(const Node &node) {
  return std::get<ArrayNode>(node);
}

const json::ObjectNode &json::AsObject(const Node &node) {
  return std::get<ObjectNode>(node);
}

json::NullNode &json::AsNull(Node &node) { return std::get<NullNode>(node); }

json::BooleanNode &json::AsBoolean(Node &node) {
  return std::get<BooleanNode>(node);
}

json::NumberNode &json::AsNumber(Node &node) {
  return std::get<NumberNode>(node);
}

json::StringNode &json::AsString(Node &node) {
  return std::get<StringNode>(node);
}

json::ArrayNode &json::AsArray(Node &node) { return std::get<ArrayNode>(node); }

json::ObjectNode &json::AsObject(Node &node) {
  return std::get<ObjectNode>(node);
}

std::ostream &operator<<(std::ostream &stream, const json::Node &node) {
  if (json::IsUndefined(node))
    return stream << "<undefined>";

  if (json::IsNull(node))
    return json::AsNull(node).Print(stream);
  if (json::IsBoolean(node))
    return json::AsBoolean(node).Print(stream);
  if (json::IsNumber(node))
    return json::AsNumber(node).Print(stream);
  if (json::IsString(node))
    return json::AsString(node).Print(stream);
  if (json::IsArray(node))
    return json::AsArray(node).Print(stream);
  if (json::IsObject(node))
    return json::AsObject(node).Print(stream);

  return stream << "<error>";
}

std::ostream &json::compact(std::ostream &stream) {
  auto &context = get_json_context(stream);
  context.format = json_format::compact;
  return stream;
}

std::ostream &json::pretty(std::ostream &stream) {
  auto &context = get_json_context(stream);
  context.format = json_format::pretty;
  return stream;
}
