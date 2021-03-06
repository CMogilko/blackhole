#pragma once

#include <gmock/gmock.h>

#include <blackhole/attribute.hpp>
#include <blackhole/logger.hpp>

namespace blackhole {
namespace testing {
namespace mock {

class logger_t : public ::blackhole::logger_t {
public:
    logger_t();
    ~logger_t();

    MOCK_METHOD2(log, void(severity_t, const message_t&));
    MOCK_METHOD3(log, void(severity_t, const message_t&, attribute_pack&));
    MOCK_METHOD3(log, void(severity_t, const lazy_message_t&, attribute_pack&));
    MOCK_METHOD0(manager, scope::manager_t&());
};

}  // namespace mock
}  // namespace testing
}  // namespace blackhole
