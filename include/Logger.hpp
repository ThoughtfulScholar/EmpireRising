#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <vector>
#include <string>

class Logger {
private:
    std::vector<std::string> messages;

public:
    Logger() = default;

    void addLog(const std::string& msg) {
        messages.push_back(msg);
        if (messages.size() > 6) {
            messages.erase(messages.begin());
        }
    }

    [[nodiscard]] const std::vector<std::string>& getLogs() const {
        return messages;
    }
};

#endif