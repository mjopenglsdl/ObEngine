#include <Graphics/DrawUtils.hpp>
#include <Input/InputManager.hpp>
#include <Modes/Game.hpp>
#include <Scene/Scene.hpp>
#include <Script/GlobalState.hpp>
#include <System/Config.hpp>
#include <System/Cursor.hpp>
#include <System/Loaders.hpp>
#include <System/Path.hpp>
#include <System/Window.hpp>
#include <Time/FramerateCounter.hpp>
#include <Time/FramerateManager.hpp>
#include <Triggers/TriggerDatabase.hpp>

namespace obe::Modes
{
    void startGame()
    {
        //Creating Window
        System::InitWindow(System::WindowContext::GameWindow);

        //Game Triggers
        Triggers::TriggerGroupPtr gameTriggers(
            Triggers::TriggerDatabase::GetInstance()->createTriggerGroup("Global", "Game"),
            Triggers::TriggerGroupPtrRemover);

        gameTriggers
			->addTrigger("Start")
			->trigger("Start")
			->addTrigger("End")
			->addTrigger("Update")
			->addTrigger("Render");

        //Font <REVISION> Remove this
        sf::Font font;
        font.loadFromFile("Data/Fonts/arial.ttf");

        //Config
        vili::ComplexNode& gameConfig = System::Config.at("GameConfig");

        //Cursor
        System::Cursor cursor;

        //Scene Creation / Loading
        Scene::Scene scene;

        Script::ScriptEngine.setErrorHandler([](int statuscode, const char* message)
        {
            Debug::Log->error("<LuaError>({0}) : {1}", statuscode, message);
        });

        //Keybinding
        Input::InputManager inputManager;

        inputManager.configure(System::Config.at("KeyBinding"));
        inputManager.addContext("game");

        sf::Event event;

        //Framerate / DeltaTime
        Time::FPSCounter fps;
        fps.loadFont(font);
        Time::FramerateManager framerateManager(gameConfig);

        System::Path("boot.lua").loadResource(&Script::ScriptEngine, System::Loaders::luaLoader);
        Script::ScriptEngine.dostring("Game.Start()");

        //Game Starts
        while (System::MainWindow.isOpen())
        {
            framerateManager.update();

            gameTriggers->pushParameter("Update", "dt", framerateManager.getGameSpeed());
            gameTriggers->trigger("Update");
                
            if (framerateManager.doRender())
                gameTriggers->trigger("Render");

                    

            //Events
            scene.update();
            Triggers::TriggerDatabase::GetInstance()->update();
            inputManager.update();
            cursor.update();

            while (System::MainWindow.pollEvent(event))
            {
                switch (event.type)
                {
                case sf::Event::Closed:
                    System::MainWindow.close();
                    break;

                case sf::Event::KeyPressed:
                    if (event.key.code == sf::Keyboard::Escape)
                        System::MainWindow.close();
                    break;
                }
            }

            if (framerateManager.doRender())
            {
                System::MainWindow.clear(Graphics::Utils::ClearColor);
                scene.draw();

                System::MainWindow.display();
            }
        }
        gameTriggers->trigger("End");
        Triggers::TriggerDatabase::GetInstance()->update();
            
        scene.update();
        System::MainWindow.close();

        std::cin.get();
    }
}