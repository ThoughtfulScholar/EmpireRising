#include "../include/LoginUI.hpp"
#include <cstring>

LoginUI::LoginUI() : letterCount(0), authenticated(false) {
    std::memset(nameInput, 0, sizeof(nameInput));
}

void LoginUI::update() {
    // Captură caractere pentru nume
    int key = GetCharPressed();
    while (key > 0) {
        if ((key >= 32) && (key <= 125) && (letterCount < 31)) {
            nameInput[letterCount] = (char)key;
            nameInput[letterCount + 1] = '\0';
            letterCount++;
        }
        key = GetCharPressed();
    }

    // Ștergere (Backspace)
    if (IsKeyPressed(KEY_BACKSPACE)) {
        letterCount--;
        if (letterCount < 0) letterCount = 0;
        nameInput[letterCount] = '\0';
    }

    // Finalizare (Enter)
    if (IsKeyPressed(KEY_ENTER) && letterCount > 0) {
        authenticated = true;
    }
}

void LoginUI::draw() const {
    // Fundal
    DrawRectangle(0, 0, 1600, 1000, {20, 20, 25, 255});
    
    // Titlu și instrucțiuni
    DrawText("EMPIRE RISING: RECRUITMENT & REBELLION", 450, 300, 35, GOLD);
    DrawText("INTRODUCETI NUMELE COMANDANTULUI:", 550, 420, 20, RAYWHITE);
    
    // Câmp Input
    DrawRectangle(600, 470, 400, 50, LIGHTGRAY);
    DrawRectangleLines(600, 470, 400, 50, authenticated ? GREEN : GOLD);
    DrawText(nameInput, 610, 485, 25, BLACK);
    
    // Footer
    DrawText("APASATI [ENTER] PENTRU A INCEPE", 630, 550, 18, GRAY);
    
    if (letterCount == 0) {
        DrawText("_", 610 + MeasureText(nameInput, 25), 485, 25, DARKGRAY);
    }
}
