#pragma once

#include <string>

namespace blackhole {
inline namespace v1 {
namespace cpp17 {

template<typename Char, typename Traits>
class basic_string_view;

typedef basic_string_view<char, std::char_traits<char>> string_view;

}  // namespace cpp17

using cpp17::string_view;

class record_t;

class sink_t {
public:
    sink_t() = default;
    sink_t(const sink_t& other) = default;
    sink_t(sink_t&& other) = default;

    virtual ~sink_t() {}

    auto operator=(const sink_t& other) -> sink_t& = default;
    auto operator=(sink_t&& other) -> sink_t& = default;

    virtual auto emit(const record_t& record, const string_view& message) -> void = 0;
};

}  // namespace v1
}  // namespace blackhole
