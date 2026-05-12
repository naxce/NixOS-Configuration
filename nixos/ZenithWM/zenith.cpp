#include <iostream>
#include <vector>
#include <string>
#include <raylib.h>
#include <unistd.h>
#include <stdlib.h>

struct DesktopIcon {
    Rectangle rect;
    std::string label;
    std::string command;
    Color color;
};

struct Notification {
    std::string appName;
    std::string message;
    float lifetime;
};

std::vector<Notification> notifications;
std::vector<DesktopIcon> icons;

void AddIcon(std::string name, std::string cmd, float x, float y, Color col) {
    icons.push_back({ {x, y, 100, 100}, name, cmd, col });
}

void SetupIcons() {
    icons.clear();
    AddIcon("VESKTOP", "vesktop &", 50, 50, (Color){88, 101, 242, 255});
    AddIcon("STEAM", "gamemoderun steam &", 50, 180, (Color){23, 26, 33, 255});
    AddIcon("BROWSER", "firefox &", 50, 310, (Color){255, 113, 67, 255});
    AddIcon("KITTY", "kitty &", 50, 440, (Color){40, 40, 40, 255});
    AddIcon("CONFIG", "wdisplays &", 50, 570, (Color){0, 150, 136, 255});
}

void DrawUI(int w, int h) {
    int dummy;
    
    for (auto& icon : icons) {
        bool hover = CheckCollisionPointRec(GetMousePosition(), icon.rect);
        
        DrawRectangleRec(icon.rect, hover ? Fade(icon.color, 0.8f) : Fade(icon.color, 0.4f));
        DrawRectangleLinesEx(icon.rect, 2, hover ? (Color){0, 255, 255, 255} : (Color){200, 200, 200, 100});
        
        DrawText(icon.label.c_str(), icon.rect.x + (100 - MeasureText(icon.label.c_str(), 15)) / 2, icon.rect.y + 110, 15, (Color){255, 255, 255, 255});

        if (hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            dummy = system(icon.command.c_str());
        }
    }

    DrawRectangle(0, h - 40, w, 40, Fade((Color){0, 0, 0, 255}, 0.9f));
    DrawText("ZENITH CORE | MAX PERFORMANCE MODE", 20, h - 30, 18, (Color){0, 255, 255, 255});
    DrawText(TextFormat("FPS: %i", GetFPS()), w - 100, h - 30, 18, (Color){0, 255, 0, 255});
    (void)dummy;
}

int main() {
    setenv("WLR_NO_HARDWARE_CURSORS", "1", 1);
    setenv("LIBVA_DRIVER_NAME", "nvidia", 1);
    setenv("__GLX_VENDOR_LIBRARY_NAME", "nvidia", 1);
    setenv("GBM_BACKEND", "nvidia-drm", 1);

    InitWindow(2560, 1440, "Zenith WM"); 
    
    int monitor = 0; 
    SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
    ToggleFullscreen();
    
    SetTargetFPS(200);
    SetupIcons();

    int dummy;

    while (!WindowShouldClose()) {
        if (IsKeyDown(KEY_LEFT_SUPER)) {
            if (IsKeyPressed(KEY_X)) dummy = system("pkill zenith");
        }

        BeginDrawing();
            ClearBackground((Color){15, 15, 20, 255});
            DrawUI(GetScreenWidth(), GetScreenHeight());
        EndDrawing();
    }

    (void)dummy;
    CloseWindow();
    return 0;
}