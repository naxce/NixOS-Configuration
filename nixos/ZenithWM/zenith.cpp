#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <raylib.h>
#include <unistd.h>
#include <stdlib.h>

struct DesktopIcon {
    Rectangle rect;
    std::string label;
    std::string command;
    Color color;
};

std::vector<DesktopIcon> icons;
bool showSettings = false;

void AddIcon(std::string name, std::string cmd, float x, float y, Color col) {
    icons.push_back({ {x, y, 100, 100}, name, cmd, col });
}

void SetupIcons() {
    icons.clear();
    AddIcon("VESKTOP", "vesktop &", 50, 50, (Color){88, 101, 242, 255});
    AddIcon("STEAM", "gamemoderun steam &", 50, 180, (Color){23, 26, 33, 255});
    AddIcon("BROWSER", "firefox &", 50, 310, (Color){255, 113, 67, 255});
    AddIcon("KITTY", "kitty &", 50, 440, (Color){40, 40, 40, 255});
}

void DrawTaskbar(int w, int h) {
    int dummy;
    DrawRectangle(0, h - 45, w, 45, Fade((Color){10, 10, 15, 255}, 0.95f));
    DrawLine(0, h - 45, w, h - 45, (Color){0, 255, 255, 255});

    Rectangle showDesktopBtn = { 5, (float)h - 40, 40, 35 };
    bool hoverSD = CheckCollisionPointRec(GetMousePosition(), showDesktopBtn);
    DrawRectangleRec(showDesktopBtn, hoverSD ? (Color){0, 255, 255, 100} : (Color){40, 40, 45, 255});
    DrawText("#", showDesktopBtn.x + 15, showDesktopBtn.y + 8, 20, WHITE);

    if (hoverSD && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        // Skrót dla Sway: minimalizuje wszystkie okna na aktualnym workspace
        dummy = system("swaymsg [app_id=\".*\"] floating enable; swaymsg [app_id=\".*\"] move scratchpad");
    }

    time_t now = time(0);
    char* dt = ctime(&now);
    std::string timeStr = std::string(dt).substr(11, 5);
    DrawText(timeStr.c_str(), w - 80, h - 32, 20, (Color){0, 255, 255, 255});

    Rectangle settingsBtn = { (float)w - 200, (float)h - 40, 110, 35 };
    bool hoverSet = CheckCollisionPointRec(GetMousePosition(), settingsBtn);
    DrawRectangleRec(settingsBtn, hoverSet ? (Color){0, 255, 255, 100} : (Color){40, 40, 45, 255});
    DrawText("SETTINGS", settingsBtn.x + 10, settingsBtn.y + 10, 16, WHITE);

    if (hoverSet && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) showSettings = !showSettings;
    (void)dummy;
}

void DrawSettingsGUI(int w, int h) {
    if (!showSettings) return;

    Rectangle panel = { (float)w - 320, (float)h - 400, 300, 340 };
    DrawRectangleRec(panel, (Color){20, 20, 25, 240});
    DrawRectangleLinesEx(panel, 2, (Color){0, 255, 255, 255});
    DrawText("ZENITH CONTROL PANEL", panel.x + 20, panel.y + 20, 18, (Color){0, 255, 255, 255});

    if (MouseButton({ panel.x + 20, panel.y + 60, 260, 40 }, "REFRESH DISPLAYS")) system("wdisplays &");
    if (MouseButton({ panel.x + 20, panel.y + 110, 260, 40 }, "RESTART ZENITH")) system("pkill zenith");
    if (MouseButton({ panel.x + 20, panel.y + 160, 260, 40 }, "CLEAN RAM (Drop Caches)")) system("pkexec sync; echo 3 | pkexec tee /proc/sys/vm/drop_caches");

    DrawText("NixOS Unstable", panel.x + 20, panel.y + 280, 15, GRAY);
    DrawText("NVIDIA RTX Mode: ON", panel.x + 20, panel.y + 300, 15, GREEN);
}

bool MouseButton(Rectangle rect, std::string text) {
    bool hover = CheckCollisionPointRec(GetMousePosition(), rect);
    DrawRectangleRec(rect, hover ? (Color){0, 255, 255, 150} : (Color){40, 40, 45, 255});
    DrawRectangleLinesEx(rect, 1, (Color){0, 255, 255, 255});
    DrawText(text.c_str(), rect.x + 15, rect.y + 12, 16, WHITE);
    return (hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON));
}

int main() {
    setenv("WLR_NO_HARDWARE_CURSORS", "1", 1);
    setenv("LIBVA_DRIVER_NAME", "nvidia", 1);
    setenv("__GLX_VENDOR_LIBRARY_NAME", "nvidia", 1);
    setenv("GBM_BACKEND", "nvidia-drm", 1);

    InitWindow(1920, 1080, "Zenith WM");
    int m = 0;
    SetWindowSize(GetMonitorWidth(m), GetMonitorHeight(m));
    ToggleFullscreen();
    SetTargetFPS(200);
    SetupIcons();

    int dummy;

    while (!WindowShouldClose()) {
        if (IsKeyDown(KEY_LEFT_SUPER) && IsKeyPressed(KEY_D)) {
            dummy = system("swaymsg [app_id=\".*\"] floating enable; swaymsg [app_id=\".*\"] move scratchpad");
        }
        if (IsKeyDown(KEY_LEFT_SUPER) && IsKeyPressed(KEY_X)) break;

        BeginDrawing();
        ClearBackground((Color){10, 10, 12, 255});

        for (auto& icon : icons) {
            bool hover = CheckCollisionPointRec(GetMousePosition(), icon.rect);
            DrawRectangleRec(icon.rect, hover ? Fade(icon.color, 0.6f) : Fade(icon.color, 0.3f));
            DrawRectangleLinesEx(icon.rect, 2, hover ? (Color){0, 255, 255, 255} : (Color){100, 100, 100, 150});
            DrawText(icon.label.c_str(), icon.rect.x + (100 - MeasureText(icon.label.c_str(), 15)) / 2, icon.rect.y + 110, 15, WHITE);
            if (hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) dummy = system(icon.command.c_str());
        }

        DrawTaskbar(GetScreenWidth(), GetScreenHeight());
        DrawSettingsGUI(GetScreenWidth(), GetScreenHeight());

        EndDrawing();
    }

    (void)dummy;
    CloseWindow();
    return 0;
}
