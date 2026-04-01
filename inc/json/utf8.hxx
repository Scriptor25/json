#pragma once

#include <istream>

namespace json::utf8
{
    template<typename T>
        requires std::same_as<std::decay_t<T>, std::u32string>
    std::string encode(T &&str)
    {
        std::string result;
        result.reserve(str.size());

        for (const auto c : str)
        {
            if (c <= 0x7F)
            {
                result.push_back(static_cast<char>(c));
                continue;
            }

            if (c <= 0x7FF)
            {
                result.push_back(static_cast<char>(0xC0 | c >> 6));
                result.push_back(static_cast<char>(0x80 | c & 0x3F));
                continue;
            }

            if (c <= 0xFFFF)
            {
                result.push_back(static_cast<char>(0xE0 | c >> 12));
                result.push_back(static_cast<char>(0x80 | c >> 6 & 0x3F));
                result.push_back(static_cast<char>(0x80 | c & 0x3F));
                continue;
            }

            if (c <= 0x10FFFF)
            {
                result.push_back(static_cast<char>(0xF0 | c >> 18));
                result.push_back(static_cast<char>(0x80 | c >> 12 & 0x3F));
                result.push_back(static_cast<char>(0x80 | c >> 6 & 0x3F));
                result.push_back(static_cast<char>(0x80 | c & 0x3F));
                continue;
            }

            result.push_back('\xEF');
            result.push_back('\xBF');
            result.push_back('\xBD');
        }

        return result;
    }

    template<typename T>
        requires std::same_as<std::decay_t<T>, std::string>
    std::u32string decode(T &&str)
    {
        std::u32string result;
        result.reserve(str.size());

        for (auto it = str.begin(); it != str.end();)
        {
            const auto b0 = *it++;

            const auto c0 = static_cast<unsigned char>(b0);

            if (c0 < 0x80)
            {
                result.push_back(c0);
                continue;
            }

            auto cont = [&](int &b) -> bool
            {
                if (it == str.end())
                    return false;

                b = *it++;
                return static_cast<unsigned char>(b) >> 6 == 0x2;
            };

            if (c0 >> 5 == 0x6)
            {
                int b1;
                if (!cont(b1))
                    continue;

                const auto c1 = static_cast<unsigned char>(b1);

                result.push_back(
                    (c0 & 0x1F) << 6
                    | c1 & 0x3F);
                continue;
            }

            if (c0 >> 4 == 0xE)
            {
                int b1, b2;
                if (!cont(b1) || !cont(b2))
                    continue;

                const auto c1 = static_cast<unsigned char>(b1);
                const auto c2 = static_cast<unsigned char>(b2);

                result.push_back(
                    (c0 & 0x0F) << 12
                    | (c1 & 0x3F) << 6
                    | c2 & 0x3F);
                continue;
            }

            if (c0 >> 3 == 0x1E)
            {
                int b1, b2, b3;
                if (!cont(b1) || !cont(b2) || !cont(b3))
                    continue;

                const auto c1 = static_cast<unsigned char>(b1);
                const auto c2 = static_cast<unsigned char>(b2);
                const auto c3 = static_cast<unsigned char>(b3);

                result.push_back(
                    (c0 & 0x07) << 18
                    | (c1 & 0x3F) << 12
                    | (c2 & 0x3F) << 6
                    | c3 & 0x3F);
                continue;
            }
        }

        return result;
    }

    class streambuf : public std::basic_streambuf<char32_t>
    {
    public:
        explicit streambuf(std::streambuf *source)
            : source(source)
        {
        }

    protected:
        int_type underflow() override
        {
            if (filled)
                return traits_type::to_int_type(buffer);

            char32_t codepoint;
            if (!read_codepoint(codepoint))
                return traits_type::eof();

            buffer = codepoint;
            filled = true;

            setg(&buffer, &buffer, &buffer + 1);

            return traits_type::to_int_type(buffer);
        }

        int_type uflow() override
        {
            filled = false;
            return underflow();
        }

        bool read_codepoint(char32_t &codepoint) const
        {
            const auto b0 = source->sbumpc();
            if (b0 == EOF)
                return false;

            const auto c0 = static_cast<unsigned char>(b0);

            if (c0 < 0x80)
            {
                codepoint = c0;
                return true;
            }

            auto cont = [&](int &b) -> bool
            {
                b = source->sbumpc();
                return b != EOF && static_cast<unsigned char>(b) >> 6 == 0x2;
            };

            if (c0 >> 5 == 0x6)
            {
                int b1;
                if (!cont(b1))
                    return false;

                const auto c1 = static_cast<unsigned char>(b1);

                codepoint = (c0 & 0x1F) << 6
                            | c1 & 0x3F;
                return true;
            }

            if (c0 >> 4 == 0xE)
            {
                int b1, b2;
                if (!cont(b1) || !cont(b2))
                    return false;

                const auto c1 = static_cast<unsigned char>(b1);
                const auto c2 = static_cast<unsigned char>(b2);

                codepoint = (c0 & 0x0F) << 12
                            | (c1 & 0x3F) << 6
                            | c2 & 0x3F;
                return true;
            }

            if (c0 >> 3 == 0x1E)
            {
                int b1, b2, b3;
                if (!cont(b1) || !cont(b2) || !cont(b3))
                    return false;

                const auto c1 = static_cast<unsigned char>(b1);
                const auto c2 = static_cast<unsigned char>(b2);
                const auto c3 = static_cast<unsigned char>(b3);

                codepoint = (c0 & 0x07) << 18
                            | (c1 & 0x3F) << 12
                            | (c2 & 0x3F) << 6
                            | c3 & 0x3F;
                return true;
            }

            return false;
        }

    private:
        std::streambuf *source;

        char32_t buffer{};
        bool filled{};
    };

    class istream : public std::basic_istream<char32_t>
    {
    public:
        explicit istream(const std::istream &source)
            : std::basic_istream<char32_t>(&buf),
              buf(source.rdbuf())
        {
        }

    private:
        streambuf buf;
    };
}
