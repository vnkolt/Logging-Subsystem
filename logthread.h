#ifndef logthread_h_included
#define logthread_h_included

#include <thread>
#include <condition_variable>
#include <mutex>
#include <fstream>

#include <deque>
#include <map>

enum class LogType
{
    None,
    Prices,
    Orders,
    Trades,
    Last
};

std::string LogKey(LogType lt);

struct log_message {
    std::string msg;
    std::string ts;
    std::thread::id thread_id;
};

//
// 1. Logging should support writing to multiple files in a multithreading environment.
// For example, one thread can write in the first.log file, and another thread can write in the second.log file at the same time.
//
// class logWorker represents separate thread to log separate entity
//
class logWorker {
private:
    std::thread m_worker;
    std::condition_variable m_cv;
    std::mutex m_mutex;

    bool m_quit;
    bool m_reopen;
    std::deque<log_message> m_messages;
    std::string m_filepath;
    std::ofstream m_logfile;

private:
    void run();
    void write(const log_message& msg);
public:
    explicit logWorker(const std::string& filepath):m_quit(false), m_reopen(false) {
        m_filepath = filepath;
    }
    ~logWorker() {
        m_quit = true;
        m_cv.notify_one();
        if (m_worker.joinable()) {
            m_worker.join();
        }
    }

    const std::string& filepath() const { return m_filepath; }

    void start();
    void reopen(const std::string& filepath);

    std::string timestamp();
    void log(const std::string& msg);
};

class logManager {
public:
    static logManager& instance() {
        static logManager logger;
        return logger;
    }
private:
    logManager() {
    }
    ~logManager() {
    }

    void log(logWorker* pLogWorker, const std::string& msg);

    std::string buildLogfilename(LogType type);

public:
    void reopen();
    void log(LogType lt, const std::string& msg);

private:
    std::mutex m_mutex;
    std::map<LogType, std::unique_ptr<logWorker>> m_loggers;
};

//
// 7. The logging library should provide a simple and convenient way for formatting complex messages.
//
void log(LogType lt, const std::string& msg);
void log_format(LogType lt, const char* fmt, ...);

#endif