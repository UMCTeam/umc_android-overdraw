//
// Created by Administrator on 2018/8/21 0021.
//

#ifndef NODE_OPENCV_LOGGER_H
#define NODE_OPENCV_LOGGER_H

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace ncv {
    using namespace spdlog;

    class Logger {
    public:
        Logger() {
            m_plogger = stderr_color_mt("uao");
        }

        ~Logger() {

        }

        static Logger* GetInstance() {
            if (m_instance == nullptr) {
                m_instance = new Logger();
            }
            return  m_instance;
        }

        void info (std:: string msg) {
            m_plogger->info(msg);
        }

        void warn (std::string msg) {
            m_plogger->warn(msg);
        }

    public:
        static Logger* m_instance;
    private:
        std::shared_ptr<logger> m_plogger;

    };

    Logger* Logger::m_instance = nullptr;
}

#endif //NODE_OPENCV_LOGGER_H
