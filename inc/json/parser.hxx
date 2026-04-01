#pragma once

#include <json/json.hxx>
#include <json/utf8.hxx>

namespace json
{
    class Parser final
    {
    public:
        explicit Parser(std::istream &stream);

        Node Parse();

    protected:
        Node ParseNull();
        Node ParseBoolean();
        Node ParseNumber();
        Node ParseString();
        Node ParseArray();
        Node ParseObject();

        void Get();
        char32_t Pop();

        unsigned char PopHalfByte();
        unsigned char PopByte();

        [[nodiscard]] bool At(char32_t c) const;

        bool Skip(char32_t c);
        bool Skip(std::string_view s);
        bool SkipWhitespace();

    private:
        utf8::istream m_Stream;
        char32_t m_Buffer;
    };
}
