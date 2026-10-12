#include "SimulationUI.h"
#include <filesystem>
#include <vector>

int main(int argc, char* argv[]) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(1280,900,"Attack Graph Simulation");
    if (!IsWindowReady()) return 1;
    SetWindowMinSize(960,720);
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);
    Font font = GetFontDefault();
    bool customFont = false;
    const char* candidates[]{"C:/Windows/Fonts/arial.ttf","/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf","/System/Library/Fonts/Supplemental/Arial.ttf"};
    std::vector<int> codepoints;
    for (int value=32;value<=591;++value) codepoints.push_back(value);
    for (int value=768;value<=879;++value) codepoints.push_back(value);
    for (int value=7680;value<=7935;++value) codepoints.push_back(value);
    for (const char* path : candidates) {
        if (FileExists(path)) {
            Font loaded = LoadFontEx(path,28,codepoints.data(),static_cast<int>(codepoints.size()));
            if (loaded.texture.id != font.texture.id && loaded.texture.id != 0) {
                font=loaded;customFont=true;break;
            }
        }
    }
    SimulationUI application(font);
    if (argc > 1) application.importDataset(argv[1]);
    else {
        const std::filesystem::path locations[]{
            std::filesystem::path(PROJECT_ROOT)/"data/graph.json",
            std::filesystem::u8path(GetApplicationDirectory())/"data/graph.json",
            std::filesystem::u8path(GetApplicationDirectory())/"graph.json",
            std::filesystem::path("data/graph.json")
        };
        for (const auto& path : locations) {
            if (std::filesystem::exists(path)) { application.importDataset(path.u8string());break; }
        }
    }
    while (!WindowShouldClose()) application.frame();
    if (customFont) UnloadFont(font);
    CloseWindow();
    return 0;
}
