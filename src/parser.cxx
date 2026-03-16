#include "json.hxx"
#include <parser.hxx>

json::Parser::Parser(std::istream &stream)
    : m_Stream(stream), m_Buffer(stream.get()) {}

json::Node json::Parser::Parse() {
  Node node;

  SkipWhitespace();

  switch (m_Buffer) {
  case 'n':
    node = ParseNull();
    break;
  case 'f':
  case 't':
    node = ParseBoolean();
    break;
  case '-':
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    node = ParseNumber();
    break;
  case '"':
    node = ParseString();
    break;
  case '[':
    node = ParseArray();
    break;
  case '{':
    node = ParseObject();
    break;
  }

  SkipWhitespace();

  return node;
}

json::Node json::Parser::ParseNull() {
  if (!Skip('n'))
    return {};
  if (!Skip('u'))
    return {};
  if (!Skip('l'))
    return {};
  if (!Skip('l'))
    return {};
  return NullNode();
}

json::Node json::Parser::ParseBoolean() {

  if (Skip('f')) {
    if (!Skip('a'))
      return {};
    if (!Skip('l'))
      return {};
    if (!Skip('s'))
      return {};
    if (!Skip('e'))
      return {};
    return BooleanNode(false);
  }

  if (!Skip('t'))
    return {};
  if (!Skip('r'))
    return {};
  if (!Skip('u'))
    return {};
  if (!Skip('e'))
    return {};
  return BooleanNode(true);
}

json::Node json::Parser::ParseNumber() {}

json::Node json::Parser::ParseString() {}

json::Node json::Parser::ParseArray() {}

json::Node json::Parser::ParseObject() {}

void json::Parser::Get() { m_Buffer = m_Stream.get(); }

bool json::Parser::At(char c) const { return m_Buffer == c; }

bool json::Parser::Skip(char c) {
  auto skip = m_Buffer == c;
  if (skip)
    Get();
  return skip;
}

bool json::Parser::SkipWhitespace() {}
