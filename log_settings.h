#ifndef log_settings_h_included
#define log_settings_h_included

//
// log_settings.h
//
// A very simple settings singleton declaration


#include <string>
#include <map>
#include <mutex>

class LogSettings {
private:
    LogSettings();
    void setDefaults();
private:
    std::string m_config_file;
    std::map<std::string, std::string> m_properties;
    std::mutex m_mutex;
public:
    static LogSettings& getInstance() {
        static LogSettings ls;
        return ls;
    }
    static bool loadSettings();
    static bool loadSettings(const std::string& config_file);

    void set_path(const std::string& key, const std::string& value) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_properties[key] = value;
    }

    std::string get_path(const std::string& key) {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_properties[key];
    }

};

#endif