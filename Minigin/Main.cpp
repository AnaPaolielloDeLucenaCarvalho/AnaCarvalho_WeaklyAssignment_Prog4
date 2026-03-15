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
#include "DiggerComponent.h"
#include "UIObservers.h"
#include "Subject.h"
#include "Observer.h"
#include "AchievementManager.h"

#if USE_STEAMWORKS
#pragma warning (push)
#pragma warning (disable:4996)
#include <steam_api.h>
#pragma warning (pop)
#endif

#include <filesystem>
namespace fs = std::filesystem;

std::shared_ptr<dae::AchievementManager> g_AchievementMgr = nullptr;

static void load()
{
	// SCENE SETUP
	auto& scene = dae::SceneManager::GetInstance().CreateScene();
	auto& input = dae::InputManager::GetInstance();

	auto font = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 36);
	auto fontSmall = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 22);

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
		fpsObject->SetLocalPosition(10, 20);
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

	// W4 INPUT - W5 EVENTS
	{
		float playerSpeed = 125.f;

		// PLAYER 1 (Digger)
		auto digger1 = std::make_unique<dae::GameObject>();
		auto render1 = digger1->AddComponent<dae::RenderComponent>("PNG/Digger/CRDIG1.png");
		render1->SetScale(2.f);
		digger1->SetLocalPosition(60, 200);

		auto diggerComp1 = digger1->AddComponent<dae::DiggerComponent>();
		auto diggerPtr1 = digger1.get();
		scene.Add(std::move(digger1));

		// UI - Player 1 Score
		auto scoreUI1 = std::make_unique<dae::GameObject>();
		scoreUI1->SetLocalPosition(10, 120);
		scoreUI1->AddComponent<dae::TextComponent>("P1 Score: 0", fontSmall, SDL_Color{ 255, 255, 255, 255 });
		auto scoreObs1 = scoreUI1->AddComponent<dae::ScoreDisplayComponent>();
		scene.Add(std::move(scoreUI1));

		// UI - Player 1 Lives
		auto livesUI1 = std::make_unique<dae::GameObject>();
		livesUI1->SetLocalPosition(10, 150);
		livesUI1->AddComponent<dae::TextComponent>("P1 Lives: 3", fontSmall, SDL_Color{ 255, 0, 0, 255 });
		auto livesObs1 = livesUI1->AddComponent<dae::LivesDisplayComponent>(3);
		scene.Add(std::move(livesUI1));

		// LINKING - Player 1
		diggerComp1->AddObserver(scoreObs1);
		diggerComp1->AddObserver(livesObs1);

		// CONTROLS - Player 1
		input.BindCommand(SDL_SCANCODE_W, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(diggerPtr1, glm::vec2{ 0, -1 }, playerSpeed));
		input.BindCommand(SDL_SCANCODE_S, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(diggerPtr1, glm::vec2{ 0, 1 }, playerSpeed));
		input.BindCommand(SDL_SCANCODE_A, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(diggerPtr1, glm::vec2{ -1, 0 }, playerSpeed));
		input.BindCommand(SDL_SCANCODE_D, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(diggerPtr1, glm::vec2{ 1, 0 }, playerSpeed));

		// PLAYER 2 (Enemy)
		auto digger2 = std::make_unique<dae::GameObject>();
		auto render2 = digger2->AddComponent<dae::RenderComponent>("PNG/Enemy/CRHOB1.png");
		render2->SetScale(2.f);
		digger2->SetLocalPosition(850, 200);

		auto diggerComp2 = digger2->AddComponent<dae::DiggerComponent>();
		auto diggerPtr2 = digger2.get();
		scene.Add(std::move(digger2));

		// UI - Player 2 Score
		auto scoreUI2 = std::make_unique<dae::GameObject>();
		scoreUI2->SetLocalPosition(800, 120);
		scoreUI2->AddComponent<dae::TextComponent>("P2 Score: 0", fontSmall, SDL_Color{ 255, 255, 255, 255 });
		auto scoreObs2 = scoreUI2->AddComponent<dae::ScoreDisplayComponent>();
		scene.Add(std::move(scoreUI2));

		// UI - Player 2 Lives
		auto livesUI2 = std::make_unique<dae::GameObject>();
		livesUI2->SetLocalPosition(800, 150);
		livesUI2->AddComponent<dae::TextComponent>("P2 Lives: 3", fontSmall, SDL_Color{ 255, 0, 0, 255 });
		auto livesObs2 = livesUI2->AddComponent<dae::LivesDisplayComponent>(3);
		scene.Add(std::move(livesUI2));

		// LINKING - Player 2
		diggerComp2->AddObserver(scoreObs2);
		diggerComp2->AddObserver(livesObs2);

		// CONTROLS - Player 2
		input.BindCommand(0, dae::Gamepad::ControllerButton::DPadUp, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(diggerPtr2, glm::vec2{ 0, -1 }, playerSpeed * 2.f));
		input.BindCommand(0, dae::Gamepad::ControllerButton::DPadDown, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(diggerPtr2, glm::vec2{ 0, 1 }, playerSpeed * 2.f));
		input.BindCommand(0, dae::Gamepad::ControllerButton::DPadLeft, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(diggerPtr2, glm::vec2{ -1, 0 }, playerSpeed * 2.f));
		input.BindCommand(0, dae::Gamepad::ControllerButton::DPadRight, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(diggerPtr2, glm::vec2{ 1, 0 }, playerSpeed * 2.f));

		// POINTS - diamonds
		std::vector<dae::GameObject*> diamondPtrs;
		for (int i = 0; i < 5; ++i)
		{
			auto diamond = std::make_unique<dae::GameObject>();
			auto render3 = diamond->AddComponent<dae::RenderComponent>("PNG/Money/CEMERALD.png");
			render3->SetScale(2.f);
			diamond->SetLocalPosition(200.f + (i * 50.f), 300.f);
			diamondPtrs.push_back(diamond.get());
			scene.Add(std::move(diamond));
		}

		diggerComp1->SetOtherPlayer(diggerPtr2);
		diggerComp1->SetDiamonds(diamondPtrs);

		diggerComp2->SetOtherPlayer(diggerPtr1);
		diggerComp2->SetDiamonds(diamondPtrs);

		// ACHIEVEMENTS
		g_AchievementMgr = std::make_shared<dae::AchievementManager>();
		diggerComp1->AddObserver(g_AchievementMgr.get());
		diggerComp2->AddObserver(g_AchievementMgr.get());
	}

	// W5 - GAME INSTRUCTIONS
	{
		auto instructions1 = std::make_unique<dae::GameObject>();
		instructions1->SetLocalPosition(10, 520);
		instructions1->AddComponent<dae::TextComponent>("P1: WASD | P2: D-Pad", fontSmall, SDL_Color{ 255, 255, 0, 255 });
		scene.Add(std::move(instructions1));

		auto instructions2 = std::make_unique<dae::GameObject>();
		instructions2->SetLocalPosition(10, 550);
		instructions2->AddComponent<dae::TextComponent>("POINTS: Eat Diamonds | LIVES: Touching each other", fontSmall, SDL_Color{ 255, 255, 0, 255 });
		scene.Add(std::move(instructions2));
	}
}

int main(int, char* [])
{
#if USE_STEAMWORKS
	if (!SteamAPI_Init())
	{
		std::cerr << "SteamAPI_Init() failed! Achievements will not work." << std::endl;
	}
	else
	{
		std::cout << "SteamAPI_Init() Success!" << std::endl;
	}
#endif

#if __EMSCRIPTEN__
	fs::path data_location = "";
#else
	fs::path data_location = "./Data/";
	if (!fs::exists(data_location))
		data_location = "../Data/";
#endif
	{
		dae::Minigin engine(data_location);
		engine.Run(load);
	}

#if USE_STEAMWORKS
	SteamAPI_Shutdown();
	std::cout << "SteamAPI Shutdown." << std::endl;
#endif

	return 0;
}
