#ifndef LOGIN_UI_HPP
#define LOGIN_UI_HPP

#include <string>
#include <raylib-cpp.hpp>

class LoginUI {
private:
    char nameInput[32];
    int letterCount;
    bool authenticated;

public:
    LoginUI();
    
    void update();
    void draw() const;

    [[nodiscard]] bool isAuthenticated() const { return authenticated; }
    [[nodiscard]] std::string getPlayerName() const { return nameInput; }
};

#endif
