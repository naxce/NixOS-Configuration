#include <iostream>
#include <vector>
#include <string>
#include <raylib.h>
#include <unistd.h>
#include <stdlib.h>

struct Notification {
    std::string appName;
    std::string message;
    float lifetime;
};

std::vector<Notification> notifications;

void AddNotification(std::string app, std::string msg) {
    notifications.push_back({app, msg, 5.0f});
}

void DrawUI(int w, int h) {
    DrawRectangle(0, h - 60, w, 60, Fade(BLACK, 0.95f));
    DrawLine(0, h - 60, w, h - 60, CYAN);

    DrawText("ZENITH", 15, h - 45, 25, CYAN);

    Rectangle btnVesktop = { 130, (float)h - 50, 100, 40 };
    if (CheckCollisionPointRec(GetMousePosition(), btnVesktop)) {
        DrawRectangleRec(btnVesktop, GRAY);
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) system("vesktop &");
    }
    DrawText("VESKTOP", btnVesktop.x + 10, btnVesktop.y + 12, 15, WHITE);

    Rectangle btnCider = { 240, (float)h - 50, 80, 40 };
    if (CheckCollisionPointRec(GetMousePosition(), btnCider)) {
        DrawRectangleRec(btnCider, GRAY);
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) system("cider &");
    }
    DrawText("CIDER", btnCider.x + 15, btnCider.y + 12, 15, WHITE);

    Rectangle btnDisplay = { 330, (float)h - 50, 160, 40 };
    bool hoverDisplay = CheckCollisionPointRec(GetMousePosition(), btnDisplay);
    DrawRectangleRec(btnDisplay, hoverDisplay ? SKYBLUE : DARKBLUE);
    DrawText("DISPLAY CONFIG", btnDisplay.x + 15, btnDisplay.y + 12, 15, WHITE);
    if (hoverDisplay && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) system("wdisplays &");

    DrawText(TextFormat("%i FPS", GetFPS()), w - 100, h - 38, 16, MAROON);
}

void DrawNotifications() {
    for (size_t i = 0; i < notifications.size(); i++) {
        float yPos = 20 + (i * 80);
        DrawRectangle(GetScreenWidth() - 270, yPos, 250, 70, Fade(BLACK, 0.8f));
        DrawRectangleLines(GetScreenWidth() - 270, yPos, 250, 70, CYAN);
        DrawText(notifications[i].appName.c_str(), GetScreenWidth() - 260, yPos + 10, 15, CYAN);
        DrawText(notifications[i].message.c_str(), GetScreenWidth() - 260, yPos + 35, 12, WHITE);
    }
}

int main() {
    setenv("WLR_NO_HARDWARE_CURSORS", "1", 1);
    setenv("LIBVA_DRIVER_NAME", "nvidia", 1);
    setenv("__GLX_VENDOR_LIBRARY_NAME", "nvidia", 1);
    setenv("GBM_BACKEND", "nvidia-drm", 1);

    InitWindow(0, 0, "Zenith WM");
    ToggleFullscreen();
    SetTargetFPS(200);

    while (!WindowShouldClose()) {
        if (IsKeyDown(KEY_LEFT_SUPER)) {
            if (IsKeyPressed(KEY_T)) system("kitty &");
            if (IsKeyPressed(KEY_F)) system("firefox &");
            if (IsKeyPressed(KEY_G)) system("gamemoderun steam &");
            if (IsKeyPressed(KEY_B)) system("blueman-manager &");
            if (IsKeyPressed(KEY_V)) system("pavucontrol &");
            if (IsKeyPressed(KEY_X)) system("pkill zenith");
        }

        for (auto it = notifications.begin(); it != notifications.end();) {
            it->lifetime -= GetFrameTime();
            if (it->lifetime <= 0) it = notifications.erase(it);
            else ++it;
        }

        BeginDrawing();
            ClearBackground(BLANK);
            DrawUI(GetScreenWidth(), GetScreenHeight());
            DrawNotifications();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}