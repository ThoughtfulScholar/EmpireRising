#include "../include/Simulation.hpp"
#include <iostream>

void Simulation::run() {
    LoginUI loginScreen;
    bool worldInitialized = false;

    while (!window.ShouldClose()) {
        window.BeginDrawing();
        window.ClearBackground(BLACK);

        switch (currentState) {
            case GameState::LOGIN:
                loginScreen.update();
                loginScreen.draw();
                if (loginScreen.isAuthenticated()) {
                    player = Player(loginScreen.getPlayerName(), 1000); // 1000 Aur start
                    if (!worldInitialized) {
                        initWorld();
                        worldInitialized = true;
                    }
                    currentState = GameState::PLAYING;
                }
                break;

            case GameState::PLAYING:
                handleInput();
                drawUI();
                break;

            case GameState::VICTORIE:
                DrawRectangle(0, 0, 1600, 1000, {0, 50, 0, 255});
                DrawText("VICTORIE TOTALA!", 550, 450, 60, GOLD);
                DrawText(TextFormat("Comandantul %s a unit imperiul in %i zile.", 
                         player.getName().c_str(), clock.getDay()), 450, 530, 25, RAYWHITE);
                DrawText("Apasati [ESC] pentru iesire", 650, 650, 20, GRAY);
                break;

            case GameState::DEFEAT:
                DrawRectangle(0, 0, 1600, 1000, {50, 0, 0, 255});
                DrawText("INFRANGERE!", 600, 450, 60, RED);
                DrawText("Resursele au fost epuizate sau timpul a expirat.", 500, 530, 25, RAYWHITE);
                DrawText("Apasati [ESC] pentru iesire", 650, 650, 20, GRAY);
                break;
        }

        window.EndDrawing();
    }
}
