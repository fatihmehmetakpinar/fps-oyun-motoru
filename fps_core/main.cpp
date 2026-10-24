#include "Engine.h"

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    Engine engine;

    if (!engine.Init("CS Competitive FPS Core - Stage 6 (3D AudioStream DSP + HE/Flashbang Grenades)", 1280, 960)) {
        return 1;
    }

    engine.Run();
    engine.Shutdown();
    return 0;
}
