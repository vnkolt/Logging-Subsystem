//
//
// logthread.cpp
//
#include "logthread.h"
#include "log_settings.h"

#include <filesystem>
#include <vector>
#include <stdarg.h>

#include "Helpers.h"

namespace fs = std::filesystem;

static std::string LogKey(LogType lt) {
    switch (lt) {
        case LogType::Prices:
            return "prices";
        case LogType::Orders:
            return "orders";
        case LogType::Trades:
            return "trades";
    }
    return "Undefined";
}

void logWorker::run() {
    while (!m_quit) {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_messages.empty() && !m_quit) {
            m_cv.wait(lock);
        }

        if (!m_messages.empty()) {
            while (!m_messages.empty()) {
                log_message message = m_messages.front();
                m_messages.pop_front();
                lock.unlock();
                write(message);
                lock.lock();
            }
            m_logfile.flush();
        }
    }
}

void logWorker::start() {
    m_worker = std::thread(&logWorker::run, this);
}

void logWorker::reopen(const std::string& filepath) {
    m_filepath = filepath;
    m_reopen = true;
}

void logWorker::write(const log_message& msg) {
    if (m_reopen) {
        m_logfile.flush();
        m_logfile.close();
        m_reopen = false;
    }

    if ( !m_logfile.is_open() ) {
        m_logfile.open(m_filepath, std::ofstream::out | std::ofstream::app);
    }

    if (m_logfile.is_open()) {
        // 9. Format of the log record :
        //
        // yyyy-MM-dd HH:mm:ss.fff, <thread id> : <message>, where :
        // yyyy-MM-dd HH:mm:ss.fff - UTC date and time of a message, when it was formatted by the code
        // <thread ID> - an ID of a thread that initiated writing to a log file
        // <message> - is any message
        //
        m_logfile << msg.ts << " " << msg.thread_id << " " << msg.msg << std::endl;
    }
}

std::string logWorker::timestamp() {

    struct tm tm;
    __time64_t long_time;
    _time64(&long_time);
    _localtime64_s(&tm, &long_time);

    auto duration = std::chrono::system_clock::now().time_since_epoch();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

    char buffer[] = "YYYY-MM-DD HH:mm:ss.fff";
    sprintf_s(buffer, "%04d-%02d-%02d %02d:%02d:%02d.%03d",
        tm.tm_year + 1900, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, (int)(ms % 1000));

    return buffer;
}

void logWorker::log(const std::string& msg) {
    log_message lmsg;

    const auto now = std::chrono::system_clock::now();

    lmsg.msg = msg;
    lmsg.thread_id = std::this_thread::get_id();

    // set a timestamp when creating a message
    lmsg.ts = timestamp();

    std::unique_lock<std::mutex> lock(m_mutex);
    m_messages.push_back(lmsg);
    lock.unlock();
    m_cv.notify_one();
}

//
// 4. For pointing a log file, you should use a relative path, for example, "/core/execution/orders" in
// this case "<root>\core\execution\yyyy-MM-dd_orders.log" file will be created.
// Where <root> is an absolute path to the root directory of all logs.
//
std::string logManager::buildLogfilename(LogType lt) {
    std::string key = LogKey(lt);
    std::string relative_path = LogSettings::getInstance().get_path(key);
    std::string root = LogSettings::getInstance().get_path("root");

    struct tm tm;
    __time64_t long_time;
    _time64(&long_time);
    _localtime64_s(&tm, &long_time);

    char filename[128];
    sprintf_s(filename, "%04d-%02d-%02d_%s.log", tm.tm_year + 1900, tm.tm_mon, tm.tm_mday, key.c_str());

    std::string path = root + "/" + relative_path;

    // build canonical file path using right slashes
    replaceAll(path, "\\", "/");
    replaceAll(path, "//", "/");

    // 5. If any directory or file doesn't exist, the logging subsystem has to create them.
    // create log path if it does not exist
    fs::create_directories(path);

    std::string full_path = path + "/" + filename;

    return full_path;

}
void logManager::log(logWorker* pLogWorker, const std::string& msg) {
    pLogWorker->log(msg);
}

//
// 10. You should provide a way of changing a path to the root of all logs on the fly (without restarting the application).
//
void logManager::reopen() {
    std::unique_lock<std::mutex> lock(m_mutex);

    std::vector<LogType> v = { LogType::Prices, LogType::Orders, LogType::Trades };

    for (auto lt : v) {
        auto it = m_loggers.find(lt);

        if (it != m_loggers.end()) { // perhaps we need to reopen this logger
            std::string logfilename = buildLogfilename(lt);
            if (it->second->filepath() != logfilename) { // reopen only if filepath has been changed
                it->second->reopen(logfilename);
            }
        }
    }
}

void logManager::log(LogType lt, const std::string& msg) {
    std::unique_lock<std::mutex> lock(m_mutex);

    auto it = m_loggers.find(lt);
    if (it == m_loggers.end() ) {
        std::string logfilename = buildLogfilename(lt);
        m_loggers.insert(std::make_pair(lt, std::move(new logWorker(logfilename))));
        m_loggers[lt].get()->start();
    }

    log(m_loggers[lt].get(), msg);
}


void log(LogType lt, const std::string& msg) {
    logManager::instance().log(lt, msg);
}

//
// 7. The logging library should provide a simple and convenient way for formatting complex messages.
//
void log_format(LogType lt, const char* fmt, ...) {
    int final_n, n = ((int)strlen(fmt)) * 2; /* Reserve two times as much as the length of the fmt */
    std::unique_ptr<char[]> formatted;
    va_list args;
    while (1) {
        formatted.reset(new char[n]); /* Wrap the plain char array into the unique_ptr */
        strcpy_s(&formatted[0], n, fmt);
        va_start(args, fmt);
        final_n = vsnprintf(&formatted[0], n, fmt, args);
        va_end(args);
        if (final_n < 0 || final_n >= n)
            n += abs(final_n - n + 1);
        else
            break;
    }
    std::string msg = formatted.get();
    log(lt, msg);
}