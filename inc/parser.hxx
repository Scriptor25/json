#pragma once

#include <istream>

#include <json.hxx>

namespace json {

class Parser final {
public:
  Parser(std::istream &stream);

  Node Parse();

protected:
  Node ParseNull();
  Node ParseBoolean();
  Node ParseNumber();
  Node ParseString();
  Node ParseArray();
  Node ParseObject();

  void Get();

  bool At(char c) const;
  bool Skip(char c);
  bool SkipWhitespace();

private:
  std::istream &m_Stream;
  int m_Buffer;
};

} // namespace json