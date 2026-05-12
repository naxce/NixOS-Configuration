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

void AddIcon(std::string name, std::string cmd, float x, float y, Color col);
void SetupIcons();
void DrawTaskbar(int w, int h);
void DrawSettingsGUI(int w, int h);
bool DrawCustomButton(Rectangle rect, std::string text);

std::vector<DesktopIcon> icons;
bool showSettings = false;

bool DrawCustomButton(Rectangle rect, std::string text) {
    bool hover = CheckCollisionPointRec(GetMousePosition(), rect);
    DrawRectangleRec(rect, hover ? (Color){0, 255, 255, 150} : (Color){40, 40, 45, 255});
    DrawRectangleLinesEx(rect, 1, (Color){0, 255, 255, 255});
    DrawText(text.c_str(), rect.x + 15, rect.y + 12, 16, WHITE);
    return (hover && IsMouseButtonReleased(MOUSE_LEFT_BUTTON));
}

void AddIcon(std::string name, std::string cmd, float x, float y, Color col) {
    icons.push_back({ {x, y, 100, 100}, name, cmd, col });
}

void SetupIcons() {
    icons.clear();
    AddIcon("VESKTOP", "nohup vesktop > /dev/null 2>&1 &", 50, 50, (Color){88, 101, 242, 255});
    AddIcon("STEAM", "nohup gamemoderun steam > /dev/null 2>&1 &", 50, 180, (Color){23, 26, 33, 255});
    AddIcon("BROWSER", "nohup firefox > /dev/null 2>&1 &", 50, 310, (Color){255, 113, 67, 255});
    AddIcon("KITTY", "nohup kitty > /dev/null 2>&1 &", 50, 440, (Color){40, 40, 40, 255});
}

void DrawTaskbar(int w, int h) {
    static int dummy;
    DrawRectangle(0, h - 45, w, 45, Fade((Color){10, 10, 15, 255}, 0.95f));
    DrawLine(0, h - 45, w, h - 45, (Color){0, 255, 255, 255});

    Rectangle showDesktopBtn = { 5, (float)h - 40, 40, 35 };
    bool hoverSD = CheckCollisionPointRec(GetMousePosition(), showDesktopBtn);
    DrawRectangleRec(showDesktopBtn, hoverSD ? (Color){0, 255, 255, 100} : (Color){40, 40, 45, 255});
    DrawText("#", showDesktopBtn.x + 15, showDesktopBtn.y + 8, 20, WHITE);

    if (hoverSD && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        dummy = system("swaymsg [app_id=\".*\"] floating enable; swaymsg [app_id=\".*\"] move scratchpad");
    }

    time_t now = time(0);
    struct tm *ltm = localtime(&now);
    DrawText(TextFormat("%02d:%02d", ltm->tm_hour, ltm->tm_min), w - 80, h - 32, 20, (Color){0, 255, 255, 255});

    Rectangle settingsBtn = { (float)w - 200, (float)h - 40, 110, 35 };
    bool hoverSet = CheckCollisionPointRec(GetMousePosition(), settingsBtn);
    DrawRectangleRec(settingsBtn, hoverSet ? (Color){0, 255, 255, 100} : (Color){40, 40, 45, 255});
    DrawText("SETTINGS", settingsBtn.x + 10, settingsBtn.y + 10, 16, WHITE);

    if (hoverSet && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) showSettings = !showSettings;
    (void)dummy;
}

void DrawSettingsGUI(int w, int h) {
    if (!showSettings) return;
    static int dummy;
    Rectangle panel = { (float)w - 320, (float)h - 410, 300, 350 };
    DrawRectangleRec(panel, (Color){20, 20, 25, 245});
    DrawRectangleLinesEx(panel, 2, (Color){0, 255, 255, 255});
    DrawText("ZENITH PANEL", panel.x + 20, panel.y + 20, 18, (Color){0, 255, 255, 255});

    if (DrawCustomButton({ panel.x + 20, panel.y + 60, 260, 40 }, "MONITORS")) dummy = system("nohup wdisplays > /dev/null 2>&1 &");
    if (DrawCustomButton({ panel.x + 20, panel.y + 110, 260, 40 }, "RESTART")) dummy = system("pkill zenith");
    if (DrawCustomButton({ panel.x + 20, panel.y + 160, 260, 40 }, "CLEAN RAM")) dummy = system("pkexec sync; echo 3 | pkexec tee /proc/sys/vm/drop_caches");

    DrawText("System: NixOS Unstable", panel.x + 20, panel.y + 290, 14, GRAY);
    DrawText("GPU: NVIDIA RTX Mode", panel.x + 20, panel.y + 310, 14, GREEN);
    (void)dummy;
}

int main() {
    setenv("WLR_NO_HARDWARE_CURSORS", "1", 1);
    setenv("LIBVA_DRIVER_NAME", "nvidia", 1);
    setenv("__GLX_VENDOR_LIBRARY_NAME", "nvidia", 1);
    setenv("GBM_BACKEND", "nvidia-drm", 1);

    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
    InitWindow(1920, 1080, "Zenith WM");

    int m = 0;
    SetWindowSize(GetMonitorWidth(m), GetMonitorHeight(m));
    ToggleFullscreen();
    SetTargetFPS(200);
    SetupIcons();

    static int dummy;

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
            DrawRectangleLinesEx(icon.rect, 2, hover ? (Color){0, 255, 255, 255} : (Color){80, 80, 85, 150});
            DrawText(icon.label.c_str(), icon.rect.x + (100 - MeasureText(icon.label.c_str(), 15)) / 2, icon.rect.y + 110, 15, WHITE);
            if (hover && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                dummy = system(icon.command.c_str());
            }
        }

        DrawTaskbar(GetScreenWidth(), GetScreenHeight());
        DrawSettingsGUI(GetScreenWidth(), GetScreenHeight());
        EndDrawing();
    }

    (void)dummy;
    CloseWindow();
    return 0;
}
