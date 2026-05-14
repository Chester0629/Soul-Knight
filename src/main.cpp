#include "App.hpp"

#include "Core/Context.hpp"

#ifdef _WIN32
extern "C" int __stdcall AllowSetForegroundWindow(unsigned long);
#endif

int main(int, char**) {
    auto context = Core::Context::GetInstance();

#ifdef _WIN32
    AllowSetForegroundWindow(0xFFFFFFFFUL); // ASFW_ANY: allow any process to steal focus
#endif

    App app;

    while (!context->GetExit()) {
        switch (app.GetCurrentState()) {
            case App::State::START:
                app.Start();
                break;

            case App::State::MAIN_MENU:
                app.UpdateMainMenu();
                break;

            case App::State::CHARACTER_SELECT:
                app.UpdateCharacterSelect();
                break;

            case App::State::UPGRADE:
                app.UpdateUpgrade();
                break;

            case App::State::UPDATE:
                app.Update();
                break;

            case App::State::GAME_OVER:
            case App::State::VICTORY:
                app.UpdateOverlay();
                break;

            case App::State::END:
                app.End();
                context->SetExit(true);
                break;
        }
        context->Update();
    }
    return 0;
}
