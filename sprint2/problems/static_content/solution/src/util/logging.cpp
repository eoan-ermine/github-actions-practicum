#include "logging.hpp"

#include <boost/date_time.hpp>
#include <boost/json.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>

namespace logging = boost::log;
namespace json = boost::json;

BOOST_LOG_ATTRIBUTE_KEYWORD(additional_data, "AdditionalData", boost::json::value)
BOOST_LOG_ATTRIBUTE_KEYWORD(timestamp, "TimeStamp", boost::posix_time::ptime)

namespace util {

void LogFormatter(const logging::record_view &rec, logging::formatting_ostream &stream) {
    json::value value = {{"timestamp", to_iso_extended_string(*rec[timestamp])},
                         {"message", *rec[logging::expressions::smessage]},
                         {"data", *rec[additional_data]}};
    stream << json::serialize(value);
}

void LogStart(std::string_view address, unsigned int port) {
    boost::json::value custom_data{{"address", address}, {"port", port}};
    BOOST_LOG_TRIVIAL(info) << boost::log::add_value(additional_data, custom_data) << "server started";
}

void LogExit(int code) {
    boost::json::value custom_data{{"code", code}};
    BOOST_LOG_TRIVIAL(info) << boost::log::add_value(additional_data, custom_data) << "server exited";
}

void LogExit(int code, std::string_view exception) {
    boost::json::value custom_data{{"code", code}, {"exception", exception}};
    BOOST_LOG_TRIVIAL(info) << boost::log::add_value(additional_data, custom_data) << "server exited";
}

void LogRequest(std::string_view address, std::string_view uri, std::string_view method) {
    boost::json::value custom_data{{"address", address}, {"uri", uri}, {"method", method}};
    BOOST_LOG_TRIVIAL(info) << boost::log::add_value(additional_data, custom_data) << "request received";
}

void LogResponse(int response_time, int code, std::string_view content_type) {
    boost::json::value custom_data{{"response_time", response_time}, {"code", code}, {"content_type", content_type}};
    BOOST_LOG_TRIVIAL(info) << boost::log::add_value(additional_data, custom_data) << "response sent";
}

void LogError(int code, std::string_view text, std::string_view where) {
    boost::json::value custom_data{{"code", code}, {"text", text}, {"where", where}};
    BOOST_LOG_TRIVIAL(info) << boost::log::add_value(additional_data, custom_data) << "error";
}

} // namespace util