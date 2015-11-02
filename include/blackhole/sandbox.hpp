#pragma once

#include <array>
#include <iostream>
#include <string>

#include "blackhole/cpp17/optional.hpp"

#include "blackhole/detail/string.hpp"

#define FMT_HEADER_ONLY
#include <cppformat/format.h>

using blackhole::detail::constexpr_string;

constexpr
std::size_t
parse_argument(const constexpr_string& string, std::size_t& pos) {
    char c = string[pos];
    if (c == '}') {
        return 2;
    } else {
        throw std::out_of_range("only {} is supported for compile-time analysys for now, sorry");
    }
}

namespace blackhole { inline namespace v2 { namespace detail {

namespace cppformat = fmt;

/// Returns the number of literals which will be produced by formatter internally.
constexpr
std::size_t
literal_count(const constexpr_string& format) {
    // Always has at least one literal.
    std::size_t result = 1;

    if (format.size() == 0) {
        return result;
    }

    std::size_t id = 0;
    bool last_literal_has_next_placeholder = false;
    bool has_some_bytes = false;

    while (id < format.size()) {
        char curr = format[id];
        if (curr == '{') {
            ++id;

            if (id >= format.size()) {
                throw std::out_of_range("unmatched '{' in format");
            }

            curr = format[id];

            if (curr == '{') {
                ++id;
                ++result;
                continue;
            } else {
                const auto size = ::parse_argument(format, id);
                id += size - 1;
                ++result;
                has_some_bytes = false;
                last_literal_has_next_placeholder = true;
                continue;
            }
        } else if (curr == '}') {
            ++id;
            if (id >= format.size()) {
                throw std::out_of_range("single '}' encountered in format string");
            }

            curr = format[id];
            if (curr == '}') {
                ++id;
                ++result;
                has_some_bytes = false;
                continue;
            } else {
                throw std::out_of_range("single '}' encountered in format string");
            }
        }

        ++id;
        has_some_bytes = true;
    }

    return result;
}

}}} // namespace blackhole::v2::detail

constexpr std::size_t count_placeholders(const constexpr_string& string) {
    std::size_t counter = 0;

    std::size_t i = 0;

    while (i != string.size()) {
        char c = string[i];

        if (c == '{') {
            i++;

            if (i == string.size()) {
                throw std::out_of_range("unmatched '{' in format");
            }
            c = string[i];

            if (c == '{') {
                i++;
                continue;
            } else {
                // save literal.
                parse_argument(string, i);
                counter++;
                // parse_argument.
            }
        } else if (c == '}') {
            // expect_closed_brace()
            i++;
            if (i == string.size()) {
                throw std::out_of_range("single '}' encountered in format string");
            }

            c = string[i];
            if (c == '}') {
                i++;
                continue;
            } else {
                throw std::out_of_range("single '}' encountered in format string");
            }
        }

        i++;
    }

    return counter;
}

constexpr std::size_t count_literals(const constexpr_string& string) {
    std::size_t counter = 0;
    int state = 0;
    std::size_t i = 0;

    while (i != string.size()) {
        char c = string[i];

        if (c == '{') {
            i++;

            if (i == string.size()) {
                throw std::out_of_range("unmatched '{' in format");
            }

            c = string[i];
            if (c == '{') {
                i++;
                continue;
            } else {
                // save literal.
                parse_argument(string, i);
                counter++;
                state = 1;
                // parse_argument.
            }
        } else if (c == '}') {
            // expect_closed_brace()
            i++;
            if (i == string.size()) {
                throw std::out_of_range("single '}' encountered in format string");
            }

            c = string[i];
            if (c == '}') {
                i++;
                continue;
            } else {
                throw std::out_of_range("single '}' encountered in format string");
            }
        }

        i++;
        state = 0;
    }

    if (state == 0) {
        counter++;
    }
    return counter;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

struct literal_t {
    constexpr_string data;
    bool placeholder_next;

    constexpr
    literal_t(constexpr_string data, bool placeholder_next):
        data(data),
        placeholder_next(placeholder_next)
    {}

    friend std::ostream&
    operator<<(std::ostream& stream, const literal_t& value) {
        stream << "literal_t(data='" << value.data << "', placeholder_next=" << value.placeholder_next << ")";
        return stream;
    }
};

constexpr
literal_t
next_literal(const constexpr_string& string) {
    size_t pos = 0;

    while (pos < string.size()) {
        if (string[pos] == '{') {
            // Possible placeholder begin.
            pos++;
            if (pos == string.size()) {
                // End of string - emit unclosed '{'.
                throw std::out_of_range("unmatched '{' in format");
            }

            if (string[pos] == '{') {
                // Matched '{{' - emit as single '{'.
                pos++;
                return {string.substr(0, pos - 1), false};
            }

            // This is a placeholder, emit full literal minis '{' character.
            auto lit = string.substr(0, pos - 1);
            pos += 1;
            return {lit, true};
        } else if (string[pos] == '}') {
            pos++;
            if (pos == string.size()) {
                // End of string - emit unexpected '}'.
                throw std::out_of_range("single '}' encountered in format string");
            }

            if (string[pos] == '}') {
                // Matched '}}' - emit as single '}'.
                pos++;
                return {string.substr(0, pos - 1), false};
            }
        }

        pos++;
    }

    return {string, false};
}

namespace blackhole { inline namespace v2 { namespace detail {

template<class T>
struct format_traits {
    static inline
    void
    format(fmt::MemoryWriter& writer, const T& val) {
        writer << val;
    }
};

template<std::size_t N>
struct formatter {
    const literal_t literal;
    const formatter<N - 1> next;

public:
    constexpr
    formatter(const constexpr_string& format) :
        literal(::next_literal(format)),
        // NOTE: 2 == placeholder.width.
        next(format.substr(literal.placeholder_next ? literal.data.size() + 2 : literal.data.size() + 1))
    {}

    template<typename T, typename... Args>
    constexpr
    void
    format(fmt::MemoryWriter& writer, const T& arg, const Args&... args) const {
        writer << fmt::StringRef(literal.data, literal.data.size());

        if (literal.placeholder_next) {
            format_traits<T>::format(writer, arg);
            next.format(writer, args...);
        } else {
            next.format(writer, arg, args...);
        }
    }

    template<class Stream>
    Stream&
    repr(Stream& stream) const {
        stream << "formatter(" << literal << ", ";
        next.repr(stream);
        stream << ")";
        return stream;
    }
};

template<>
struct formatter<0>;

template<>
struct formatter<1> {
    const literal_t literal;

public:
    constexpr
    formatter(const constexpr_string& format) :
        literal({format, false})
    {}

    // TODO: I can calcualte attributes count here to avoid unecessary Args... paramteter, cause
    // it's always empty.
    template<typename... Args>
    constexpr
    void
    format(fmt::MemoryWriter& writer, const Args&...) const {
        writer << fmt::StringRef(literal.data, literal.data.size());
    }

    template<class Stream>
    Stream&
    repr(Stream& stream) const {
        stream << "formatter(" << literal << ")";
        return stream;
    }
};

}}} // namespace blackhole::v2::detail
