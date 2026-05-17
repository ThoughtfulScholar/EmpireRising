#include "../include/LoginUI.h"

void LoginUI::update() {
    int key = GetCharPressed();
    while (key > 0) {
        if ((key >= 32) && (key <= 125) && (letterCount < 31)) {
            nameInput[letterCount] = (char)key;
            nameInput[letterCount + 1] = '\0';
            letterCount++;
        }
        key = GetCharPressed();
    }
    if (IsKeyPressed(KEY_BACKSPACE)) {
        letterCount--;
        if (letterCount < 0) letterCount = 0;
        nameInput[letterCount] = '\0';
    }
    if (IsKeyPressed(KEY_ENTER) && letterCount > 0) authenticated = true;
}

void LoginUI::draw() const {
    // MODIFICARE 1: Mărim fundalul negru să acopere ecranul nou de 1800x1000
    DrawRectangle(0, 0, 1800, 1000, {20, 20, 25, 255});

    // MODIFICARE 2: Adăugăm +100 la X pentru ca textele și căsuța să fie centrate pe noul ecran
    // Titlul principal (de la 450 la 550)
    DrawText("EMPIRE RISING: RECRUITMENT & REBELLION", 550, 300, 35, GOLD);

    // Subtitlul (de la 550 la 650)
    DrawText("INTRODUCETI NUMELE COMANDANTULUI:", 650, 420, 20, RAYWHITE);

    // Căsuța de input (de la 600 la 700)
    DrawRectangle(700, 470, 400, 50, LIGHTGRAY);
    DrawRectangleLines(700, 470, 400, 50, authenticated ? GREEN : GOLD);

    // Textul introdus de jucător (de la 610 la 710, ca să rămână în interiorul căsuței mutate)
    DrawText(nameInput, 710, 485, 25, BLACK);

    // Textul de ajutor de jos (de la 630 la 730)
    DrawText("APASATI [ENTER] PENTRU A INCEPE", 730, 550, 18, GRAY);
}