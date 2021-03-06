#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <blackhole/cpp17/string_view.hpp>
#include <blackhole/sink/syslog.hpp>

#include <blackhole/detail/procname.hpp>

namespace blackhole {
inline namespace v1 {
namespace sink {
namespace {

using sink::syslog_t;

using ::testing::Return;
using ::testing::StrictMock;

TEST(syslog_t, Default) {
    syslog_t syslog;

    EXPECT_EQ(detail::procname().to_string(), syslog.identity());
    EXPECT_EQ(LOG_PID, syslog.option());
    EXPECT_EQ(LOG_USER, syslog.facility());
}

}  // namespace
}  // namespace sink
}  // namespace v1
}  // namespace blackhole
