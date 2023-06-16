//
// log_settings.cpp
//
// A very simple settings singleton implementastion

/*************************************************

 We don't use for configuration file something like XML or JSON format.
 Configuration content is a set of strings like

 key=value

 for example:

root=c:\dev\DevBrother\Logger\x64\Debug\Logger
prices=/core/execution/prices
orders=/core/execution/orders

where

root defines root path for log files;
prices defines relative path of log file for prices;
orders defines relative path of log file for orders;
trades defines relative path of log file for trades;

 By default key "root" points to subdirectory DevBrother in system temp directory.
 By default keys "prices", "trades", and "orders point to correspond sub-directories in root

*************************************************/
#include "log_settings.h"
#include <vector>
#include <regex>
#include <fstream>

#include <string>
#include <filesystem>

namespace fs = std::filesystem;

std::vector<std::string> split(const std::string str, const std::string regex_str) {
    std::regex regexz(regex_str);
    return { std::sregex_token_iterator(str.begin(), str.end(), regexz, -1),
            std::sregex_token_iterator() };
}

// Default constructor loads default settings
LogSettings::LogSettings() {
    setDefaults();
}

void LogSettings::setDefaults() {
    std::string root = fs::temp_directory_path().u8string();
    root += "DevBrother";

    m_properties["root"] = root;
    m_properties["prices"] = "/prices";
    m_properties["trades"] = "/trades";
    m_properties["orders"] = "/orders";

}

bool LogSettings::loadSettings() {

    if (!fs::exists(LogSettings::getInstance().m_config_file)) {
        return false;
    }

    std::ifstream ifs(LogSettings::getInstance().m_config_file);

    std::string line;
    while (std::getline(ifs, line)) {
        std::vector<std::string> tokens = split(line, "=");
        if (2 == tokens.size()) { // path_key=path_value
            LogSettings::getInstance().set_path(tokens[0], tokens[1]);
        }
    }
    return true;

}

bool LogSettings::loadSettings(const std::string& config_file) {
    LogSettings::getInstance().m_config_file = config_file;
    return loadSettings();
}
