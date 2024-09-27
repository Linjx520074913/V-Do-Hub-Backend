#ifndef LOG_SINK_H_
#define LOG_SINK_H_

#include "spdlog.h"
#include "sinks/base_sink.h"
#include <functional>
#include <string>
#include <iostream>

typedef std::function<void(std::string)> logcallback;

template <typename Mutex>
class LogSink : public spdlog::sinks::base_sink<Mutex>
{
public:

    LogSink(logcallback cb = nullptr)
    {
        
        cb_ = cb;

    }

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override
    {

        // log_msg is a struct containing the log entry info like level, timestamp, thread id etc.
        // msg.raw contains pre formatted log

        // If needed (very likely but not mandatory), the sink formats the message before sending it to its final destination:
        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
        string str = fmt::to_string(formatted);

        if (cb_ != nullptr) {
            cb_(fmt::to_string(formatted));
        }

    }

    void flush_() override
    {
        std::cout << std::flush;
    }

private:

    logcallback cb_;

};

#include "details/null_mutex.h"
#include <mutex>
using log_sink_mt = LogSink<std::mutex>;
using log_sink_st = LogSink<spdlog::details::null_mutex>;

#endif // ! LOG_SINK_H_