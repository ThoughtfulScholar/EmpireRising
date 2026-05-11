#include "../include/Simulation.hpp"
#include "../include/UnitFactory.hpp"

void Simulation::drawUI() {
    const int sbX = 1200;
    const int mapOffX = 50, mapOffY = 50;

    // 1. HARTĂ ȘI ENTITĂȚI
    worldMap.draw(mapOffX, mapOffY);

    // 2. CETĂȚI
    for (size_t i = 0; i < regions.size(); ++i) {
        const auto& city = regions[i].getCity();
        auto [cx, cy] = city.getPos();
        int sx = mapOffX + cx * 40 + 20;
        int sy = mapOffY + cy * 40 + 20;

        if (i == (size_t)activeRegionIdx) {
            DrawCircleLines(sx, sy, 26, GOLD);
        }

        Color cCol = city.isOccupied() ? GREEN : MAROON;
        DrawCircle(sx, sy, 15, cCol);
        DrawCircleLines(sx, sy, 16, RAYWHITE);
        DrawText(city.getName().c_str(), sx - MeasureText(city.getName().c_str(), 12) / 2, sy + 22, 12, RAYWHITE);
    }

    // 3. INAMICI ȘI JUCĂTOR
    for (const auto& enemy : roamingEnemies) enemy.draw(mapOffX, mapOffY);
    auto [px, py] = player.getPos();
    DrawCircle(mapOffX + px * 40 + 20, mapOffY + py * 40 + 20, 12, GOLD);

    // 4. SIDEBAR - FUNDAL
    DrawRectangle(sbX, 0, 1600 - sbX, 1000, {25, 25, 30, 255});
    DrawLine(sbX, 0, sbX, 1000, GOLD);

    // --- SECȚIUNE STATUS ---
    DrawText("STATUS CAMPANIE", sbX + 20, 15, 18, SKYBLUE);
    DrawText(TextFormat("Ziua: %i / %i", clock.getDay(), MAX_DAYS), sbX + 20, 40, 22, WHITE);
    DrawText(TextFormat("Aur: %i", player.getGold()), sbX + 20, 125, 22, GOLD);

    // --- SECȚIUNE CETATE SELECTATĂ ---
    auto& selRegion = regions[activeRegionIdx];
    City& selCity = selRegion.getCity();
    DrawText(selCity.getName().c_str(), sbX + 20, 235, 24, GOLD);

    if (selCity.isOccupied()) {
        DrawText(TextFormat("Pop.: %i | Niv: %i", selCity.getPopulation(), selCity.getCityLevel()), sbX + 20, 270, 17, LIGHTGRAY);
        DrawText("GARNIZOANA:", sbX + 20, 355, 16, SKYBLUE);
        int gy = 380;
        for (auto const& [name, count] : selCity.getGarrisonCounts()) {
            DrawText(TextFormat("%i x %s", count, name.c_str()), sbX + 40, gy, 16, WHITE);
            gy += 22;
        }
    } else {
        DrawText("STARE: OCUPAT DE INAMICI", sbX + 20, 270, 17, MAROON);
    }

    // --- WAR JOURNAL ---
    DrawRectangle(sbX + 15, 815, 370, 175, {20, 20, 20, 200});
    DrawText("WAR JOURNAL", sbX + 20, 825, 18, GOLD);
    auto allMessages = logger.getMessages();
    int logY = 855;
    int mCount = 0;
    for (auto it = allMessages.rbegin(); it != allMessages.rend() && mCount < 6; ++it, ++mCount) {
        DrawText(it->c_str(), sbX + 25, logY, 13, (mCount == 0 ? RAYWHITE : GRAY));
        logY += 19;
    }

    // 5. OVERLAYS (Recrutare / Garnizoană)
    if (showRecruitment) {
        DrawRectangle(0, 0, 1600, 1000, {0, 0, 0, 200});
        DrawRectangle(400, 250, 800, 500, {30, 30, 35, 255});
        DrawRectangleLines(400, 250, 800, 500, GOLD);
        DrawText("RECRUTARE TRUPE (1-5)", 630, 280, 25, GOLD);
        // Aici s-ar apela un loop pentru afișarea GameData (economisesc spațiu pentru brevitate)
    }

    // 6. FOOTER INFO
    DrawRectangle(0, 930, 1200, 70, {15, 15, 20, 220});
    DrawText("[W,A,S,D] Navigare | [N] Zi Noua | [F] Lupta | [R] Recrutare | [TAB] Oras", 30, 945, 18, RAYWHITE);
}
