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
    DrawRectangle(0, h - 60, w, 60, Fade((Color){0, 0, 0, 255}, 0.95f));
    DrawLine(0, h - 60, w, h - 60, (Color){0, 255, 255, 255});

    DrawText("ZENITH", 15, h - 45, 25, (Color){0, 255, 255, 255});

    int dummy;

    Rectangle btnVesktop = { 130, (float)h - 50, 100, 40 };
    if (CheckCollisionPointRec(GetMousePosition(), btnVesktop)) {
        DrawRectangleRec(btnVesktop, (Color){130, 130, 130, 255});
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) dummy = system("vesktop &");
    }
    DrawText("VESKTOP", btnVesktop.x + 10, btnVesktop.y + 12, 15, (Color){255, 255, 255, 255});

    Rectangle btnCider = { 240, (float)h - 50, 80, 40 };
    if (CheckCollisionPointRec(GetMousePosition(), btnCider)) {
        DrawRectangleRec(btnCider, (Color){130, 130, 130, 255});
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) dummy = system("cider &");
    }
    DrawText("CIDER", btnCider.x + 15, btnCider.y + 12, 15, (Color){255, 255, 255, 255});

    Rectangle btnDisplay = { 330, (float)h - 50, 160, 40 };
    bool hoverDisplay = CheckCollisionPointRec(GetMousePosition(), btnDisplay);
    DrawRectangleRec(btnDisplay, hoverDisplay ? (Color){102, 191, 255, 255} : (Color){0, 82, 172, 255});
    DrawText("DISPLAY CONFIG", btnDisplay.x + 15, btnDisplay.y + 12, 15, (Color){255, 255, 255, 255});
    if (hoverDisplay && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) dummy = system("wdisplays &");

    DrawText(TextFormat("%i FPS", GetFPS()), w - 100, h - 38, 16, (Color){190, 33, 55, 255});
    (void)dummy;
}

void DrawNotifications() {
    for (size_t i = 0; i < notifications.size(); i++) {
        float yPos = 20 + (i * 80);
        DrawRectangle(GetScreenWidth() - 270, yPos, 250, 70, Fade((Color){0, 0, 0, 255}, 0.8f));
        DrawRectangleLines(GetScreenWidth() - 270, yPos, 250, 70, (Color){0, 255, 255, 255});
        DrawText(notifications[i].appName.c_str(), GetScreenWidth() - 260, yPos + 10, 15, (Color){0, 255, 255, 255});
        DrawText(notifications[i].message.c_str(), GetScreenWidth() - 260, yPos + 35, 12, (Color){255, 255, 255, 255});
    }
}

int main() {
    setenv("RAYLIB_LIBPATH", "wayland", 1);
    setenv("WLR_NO_HARDWARE_CURSORS", "1", 1);
    setenv("SDL_VIDEODRIVER", "wayland", 1);
    setenv("_JAVA_AWT_WM_NONREPARENTING", "1", 1);
    setenv("LIBVA_DRIVER_NAME", "nvidia", 1);
    setenv("__GLX_VENDOR_LIBRARY_NAME", "nvidia", 1);
    setenv("GBM_BACKEND", "nvidia-drm", 1);

    InitWindow(0, 0, "Zenith WM");
    ToggleFullscreen();
    SetTargetFPS(200);

    int dummy;

    while (!WindowShouldClose()) {
        if (IsKeyDown(KEY_LEFT_SUPER)) {
            if (IsKeyPressed(KEY_T)) dummy = system("kitty &");
            if (IsKeyPressed(KEY_F)) dummy = system("firefox &");
            if (IsKeyPressed(KEY_G)) dummy = system("gamemoderun steam &");
            if (IsKeyPressed(KEY_B)) dummy = system("blueman-manager &");
            if (IsKeyPressed(KEY_V)) dummy = system("pavucontrol &");
            if (IsKeyPressed(KEY_X)) dummy = system("pkill zenith");
        }

        for (auto it = notifications.begin(); it != notifications.end();) {
            it->lifetime -= GetFrameTime();
            if (it->lifetime <= 0) it = notifications.erase(it);
            else ++it;
        }

        BeginDrawing();
            ClearBackground((Color){0, 0, 0, 0});
            DrawUI(GetScreenWidth(), GetScreenHeight());
            DrawNotifications();
        EndDrawing();
    }

    (void)dummy;
    CloseWindow();
    return 0;
}