#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#if _DEBUG && __has_include(<vld.h>)
#include <vld.h>
#endif

// Comment to commit again so - that i can test out Emscripten 2 time

#include "Minigin.h"
#include "SceneManager.h"
#include "Scene.h"
#include "GameObject.h"
#include "TextComponent.h"
#include "FPSComponent.h"
#include "ResourceManager.h"
#include "RenderComponent.h"
#include "RotatorComponent.h"
#include "TrashCacheComponent.h"
#include "InputManager.h"
#include "MoveCommand.h"

#include <filesystem>
namespace fs = std::filesystem;

static void load()
{
	// SCENE SETUP
	auto& scene = dae::SceneManager::GetInstance().CreateScene();
	auto& input = dae::InputManager::GetInstance();

	auto font = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 36);

	// RENDERING
	{
		auto go = std::make_unique<dae::GameObject>();
		go->AddComponent<dae::RenderComponent>("background.png");
		scene.Add(std::move(go));

		go = std::make_unique<dae::GameObject>();
		go->AddComponent<dae::RenderComponent>("logo.png");
		go->SetLocalPosition(358, 180);
		scene.Add(std::move(go));

		auto textObj = std::make_unique<dae::GameObject>();
		textObj->AddComponent<dae::TextComponent>("Programming 4 Assignment", font, SDL_Color{ 255, 255, 0, 255 });
		textObj->SetLocalPosition(292, 20);
		scene.Add(std::move(textObj));
	}

	// FPS COUNTER
	{
		auto fpsObject = std::make_unique<dae::GameObject>();
		fpsObject->AddComponent<dae::TextComponent>("0 FPS", font, SDL_Color{ 255, 255, 0, 255 });
		fpsObject->AddComponent<dae::FPSComponent>();
		fpsObject->SetLocalPosition(10, 10);
		scene.Add(std::move(fpsObject));
	}

	// W2 ROTATION
	{
		/*
		// Root Object (Center of screen, stationary)
		auto pivotObj = std::make_unique<dae::GameObject>();
		pivotObj->SetLocalPosition(358, 180);
		auto pivotPtr = pivotObj.get();
		scene.Add(std::move(pivotObj));

		// 1 Child (Rotates around Root)
		auto char1Obj = std::make_unique<dae::GameObject>();
		char1Obj->AddComponent<dae::RenderComponent>("PNG/Money/VEMERALD.png");
		char1Obj->AddComponent<dae::RotatorComponent>(50.0f, 2.f); // Radius 50, Speed 2
		char1Obj->SetParent(pivotPtr, false);
		auto char1Ptr = char1Obj.get();
		scene.Add(std::move(char1Obj));

		// 2 Child (Rotates around 1 Child)
		auto char2Obj = std::make_unique<dae::GameObject>();
		char2Obj->AddComponent<dae::RenderComponent>("PNG/Money/VEMERALD.png");
		char2Obj->AddComponent<dae::RotatorComponent>(150.0f, -3.0f);
		char2Obj->SetParent(char1Ptr, false);
		scene.Add(std::move(char2Obj));
		*/
	}

	// W3 TRASH CACHE
	{
		/*
		auto go2 = std::make_unique<dae::GameObject>();
		go2->AddComponent<dae::TrashCacheComponent>();
		scene.Add(std::move(go2));
		*/
	}

	// W4 INPUT
	{
		// CHARACTER 1 - Digger (WASD)
		auto digger = std::make_unique<dae::GameObject>();
		digger->AddComponent<dae::RenderComponent>("PNG/Digger/CRDIG1.png");
		digger->SetLocalPosition(100, 100);
		auto diggerPtr = digger.get();
		scene.Add(std::move(digger));

		float playerSpeed = 2.0f;

		input.BindCommand(SDL_SCANCODE_W, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(diggerPtr, glm::vec2{ 0, -1 }, playerSpeed));
		input.BindCommand(SDL_SCANCODE_S, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(diggerPtr, glm::vec2{ 0, 1 }, playerSpeed));
		input.BindCommand(SDL_SCANCODE_A, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(diggerPtr, glm::vec2{ -1, 0 }, playerSpeed));
		input.BindCommand(SDL_SCANCODE_D, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(diggerPtr, glm::vec2{ 1, 0 }, playerSpeed));

		// CHARACTER 2 - Enemy (DPad)
		auto enemy = std::make_unique<dae::GameObject>();
		enemy->AddComponent<dae::RenderComponent>("PNG/Enemy/VNOB1.png");
		enemy->SetLocalPosition(200, 200);
		auto enemyPtr = enemy.get();
		scene.Add(std::move(enemy));

		float enemySpeed = playerSpeed * 2.0f; // Double speed

		input.BindCommand(dae::Gamepad::ControllerButton::DPadUp, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(enemyPtr, glm::vec2{ 0, -1 }, enemySpeed));
		input.BindCommand(dae::Gamepad::ControllerButton::DPadDown, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(enemyPtr, glm::vec2{ 0, 1 }, enemySpeed));
		input.BindCommand(dae::Gamepad::ControllerButton::DPadLeft, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(enemyPtr, glm::vec2{ -1, 0 }, enemySpeed));
		input.BindCommand(dae::Gamepad::ControllerButton::DPadRight, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(enemyPtr, glm::vec2{ 1, 0 }, enemySpeed));
	}
}

int main(int, char*[]) {
#if __EMSCRIPTEN__
	fs::path data_location = "";
#else
	fs::path data_location = "./Data/";
	if(!fs::exists(data_location))
		data_location = "../Data/";
#endif
	dae::Minigin engine(data_location);
	engine.Run(load);
    return 0;
}
