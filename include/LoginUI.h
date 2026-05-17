#ifndef LOGINUI_H
#define LOGINUI_H

#include <string>
#include <raylib-cpp.hpp>

class LoginUI {
private:
    char nameInput[32] = "\0";
    int letterCount = 0;
    bool authenticated = false;

public:
    void update();
    void draw() const;

    [[nodiscard]] bool isAuthenticated() const { return authenticated; }
    [[nodiscard]] std::string getPlayerName() const { return nameInput; }
};

#endif // LOGINUI_H