#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#if _DEBUG && __has_include(<vld.h>)
#include <vld.h>
#endif

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
#include "ServiceLocator.h"
#include "MiniaudioSoundSystem.h"
#include "LoggingSoundSystem.h"

#if USE_STEAMWORKS
#pragma warning (push)
#pragma warning (disable:4996)
#include <steam_api.h>
#pragma warning (pop)
#endif

#include <filesystem>
namespace fs = std::filesystem;

std::shared_ptr<dae::AchievementManager> g_AchievementMgr = nullptr;

// Digger Sounds
enum DiggerSounds
{
	MUSIC = 0,
	BONUS = 1,
	NEXT_LEVEL = 2,
	DEATH = 3
};

static void load()
{
	// SOUND SYSTEM
	auto miniaudioSystem = std::make_unique<dae::MiniaudioSoundSystem>();
	auto loggingSoundSystem = std::make_unique<dae::LoggingSoundSystem>(std::move(miniaudioSystem));
	dae::ServiceLocator::register_sound_system(std::move(loggingSoundSystem));

	auto& soundSystem = dae::ServiceLocator::get_sound_system();

	// folder path for both web and computer
#ifdef __EMSCRIPTEN__
	const std::string soundFolder = "Sounds/";
#else
	const std::string soundFolder = "Data/Sounds/";
#endif

	soundSystem.loadSound(DiggerSounds::MUSIC, soundFolder + "main_music.wav");
	soundSystem.loadSound(DiggerSounds::BONUS, soundFolder + "bonus.wav");
	soundSystem.loadSound(DiggerSounds::NEXT_LEVEL, soundFolder + "next_level.wav");
	soundSystem.loadSound(DiggerSounds::DEATH, soundFolder + "death.wav");

// ---------------------------------------------------

	// SCENE SETUP
	auto& scene = dae::SceneManager::GetInstance().CreateScene();
	auto& input = dae::InputManager::GetInstance();

	auto font = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 36);
	auto fontSmall = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 22);

// ---------------------------------------------------
	
	// RENDERING BACKGROUND
	{
		/*auto go = std::make_unique<dae::GameObject>();
		go->AddComponent<dae::RenderComponent>("background.png");
		scene.Add(std::move(go));

		go = std::make_unique<dae::GameObject>();
		go->AddComponent<dae::RenderComponent>("logo.png");
		go->SetLocalPosition(358, 180);
		scene.Add(std::move(go));*/

	}

// ---------------------------------------------------
		
	// LEVEL GENERATION
	glm::vec2 p1Spawn{ 60, 200 }; // fallbacks
	glm::vec2 p2Spawn{ 850, 200 };
	std::vector<glm::vec2> diamondSpawns;

	// 'X' = Dirt
	// ' ' = Empty
	// 'P' = Player 1
	// 'E' = Player 2
	// 'D' = Diamond
	// 'C' = Coin Bag
	std::vector<std::string> mapLayout =
	{
		"XX XXXXXXXXXXXXXXX      XX",
		"XX XXXXCXXXXXXXXXX XXCXXXX",
		"XX XXXDDXXXXXDXXXX XXXXXXX",
		"XX CXXDDXXXXXDXXXX XXXXXXX",
		"XXEXXXDDCXXXXDCXXX XXDDDXX",
		"XX XXXDDXXXXXDXXXX XXDDDXX",
		"XX XXXDDXXXXXDXXXX XXDDDXX",
		"XX  XXXXXXXXCXCXXX XXXXXXX",
		"XXX XXXXXXXXXXXXXX XXXXXXX",
		"XXX     XXXXXXXXXX XXXXXXX",
		"XXXXXXX XXXXXXXXXX XXXXXXX",
		"XXDXXXX XXXXXXXXXX XXXXDXX",
		"XXDDXXX      P     XXXDDXX",
		"XXXXXXXXXXXXXXXXXXXXXXXXXX",
	};

	float tileWidth = 40.0f;
	float tileHeight = 8.0f;
	float startX = 0.f;
	float startY = 52.0f;

	// DRAW A GLOBAL DIRT BACKGROUND
	for (int row = 0; row < 14; ++row)
	{
		for (int col = 0; col < 26; ++col)
		{
			float blockX = startX + (col * tileWidth);
			float blockY = startY + (row * tileWidth);

			for (int strip = 0; strip < 5; ++strip)
			{
				auto tile = std::make_unique<dae::GameObject>();
				tile->AddComponent<dae::RenderComponent>("PNG/Map/VBACK1.png");
				tile->SetLocalPosition(blockX, blockY + (strip * tileHeight));
				scene.Add(std::move(tile));
			}
		}
	}

	// DRAW PATH AND ENTITIES
	for (size_t row = 0; row < mapLayout.size(); ++row)
	{
		for (size_t col = 0; col < mapLayout[row].size(); ++col)
		{
			char tileChar = mapLayout[row][col];

			float blockX = startX + (col * tileWidth);
			float blockY = startY + (row * tileWidth);

			// PATH
			if (tileChar == ' ' || tileChar == 'P' || tileChar == 'E')
			{
				// Later change to use the correct textures
				auto pathSquare = std::make_unique<dae::GameObject>();
				pathSquare->AddComponent<dae::UIPanelComponent>(tileWidth, tileWidth, SDL_Color{ 0, 0, 0, 255 });
				pathSquare->SetLocalPosition(blockX, blockY);
				scene.Add(std::move(pathSquare));
			}

			// ENTITIES
			if (tileChar == 'P')
			{
				p1Spawn = { blockX, blockY };
			}
			else if (tileChar == 'E')
			{
				p2Spawn = { blockX, blockY };
			}
			else if (tileChar == 'D')
			{
				diamondSpawns.push_back({ blockX, blockY });
			}
			else if (tileChar == 'C')
			{
			}
		}
	}

// ---------------------------------------------------
	
	// UI BACKGROUND
	auto hudBg = std::make_unique<dae::GameObject>();
	hudBg->AddComponent<dae::UIPanelComponent>(1024.f, 52.f, SDL_Color{ 0, 0, 0, 255 });
	hudBg->SetLocalPosition(0, 0);
	scene.Add(std::move(hudBg));

// ---------------------------------------------------
	
	// FPS COUNTER
	auto fpsObject = std::make_unique<dae::GameObject>();
	fpsObject->AddComponent<dae::TextComponent>("0 FPS", fontSmall, SDL_Color{ 150, 150, 150, 255 });
	fpsObject->AddComponent<dae::FPSComponent>();
	fpsObject->SetLocalPosition(475, 15);
	scene.Add(std::move(fpsObject));

// ---------------------------------------------------
	
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

// ---------------------------------------------------
	
	// W4 INPUT - W5 EVENTS
	float playerSpeed = 125.f;

	// PLAYER 1 (Digger1)
	auto digger1 = std::make_unique<dae::GameObject>();
	digger1->AddComponent<dae::RenderComponent>("PNG/Digger/VRDIG1X.png");
	digger1->SetLocalPosition(p1Spawn.x, p1Spawn.y);

	auto diggerComp1 = digger1->AddComponent<dae::DiggerComponent>();
	auto diggerPtr1 = digger1.get();
	scene.Add(std::move(digger1));

	// UI - Player 1 Score (Sprite)
	auto scoreUI1 = std::make_unique<dae::GameObject>();
	scoreUI1->SetLocalPosition(20, 13);
	auto scoreObs1 = scoreUI1->AddComponent<dae::SpriteScoreDisplayComponent>("PNG/UI/VNUM", ".png", 24.f);
	scene.Add(std::move(scoreUI1));

	// UI - Player 1 Lives (Sprite)
	auto livesUI1 = std::make_unique<dae::GameObject>();
	livesUI1->SetLocalPosition(150, 15); // to the right of the score ui
	auto livesObs1 = livesUI1->AddComponent<dae::LivesSpriteDisplayComponent>("PNG/Digger/VRDIG1X.png", 3, 35.5f);
	scene.Add(std::move(livesUI1));

	// LINKING - Player 1
	diggerComp1->AddObserver(scoreObs1);
	diggerComp1->AddObserver(livesObs1);

	// CONTROLS - Player 1
	input.BindCommand(SDL_SCANCODE_W, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(diggerPtr1, glm::vec2{ 0, -1 }, playerSpeed));
	input.BindCommand(SDL_SCANCODE_S, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(diggerPtr1, glm::vec2{ 0, 1 }, playerSpeed));
	input.BindCommand(SDL_SCANCODE_A, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(diggerPtr1, glm::vec2{ -1, 0 }, playerSpeed));
	input.BindCommand(SDL_SCANCODE_D, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(diggerPtr1, glm::vec2{ 1, 0 }, playerSpeed));

	// PLAYER 2 (Digger2)
	auto digger2 = std::make_unique<dae::GameObject>();
	digger2->AddComponent<dae::RenderComponent>("PNG/Enemy/VRDIG1X.png");
	digger2->SetLocalPosition(p2Spawn.x, p2Spawn.y);

	auto diggerComp2 = digger2->AddComponent<dae::DiggerComponent>();
	auto diggerPtr2 = digger2.get();
	scene.Add(std::move(digger2));

	// UI - Player 2 Score (Sprite)
	auto scoreUI2 = std::make_unique<dae::GameObject>();
	scoreUI2->SetLocalPosition(810, 13);
	auto scoreObs2 = scoreUI2->AddComponent<dae::SpriteScoreDisplayComponent>("PNG/UI/VNUM", ".png", 24.f);
	scene.Add(std::move(scoreUI2));

	// UI - Player 2 Lives (Sprite)
	auto livesUI2 = std::make_unique<dae::GameObject>();
	livesUI2->SetLocalPosition(940, 15); // to the right of the score2 ui
	auto livesObs2 = livesUI2->AddComponent<dae::LivesSpriteDisplayComponent>("PNG/Digger/VRDIG1X.png", 3, 35.5f);
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
	for (const auto& spawnPos : diamondSpawns)
	{
		auto diamond = std::make_unique<dae::GameObject>();
		auto render3 = diamond->AddComponent<dae::RenderComponent>("PNG/Money/CEMERALD.png");
		render3->SetScale(2.f);

		diamond->SetLocalPosition(spawnPos.x, spawnPos.y);

		diamondPtrs.push_back(diamond.get());
		scene.Add(std::move(diamond));
	}

	diggerComp1->SetOtherPlayer(diggerPtr2);
	diggerComp1->SetDiamonds(diamondPtrs);

	diggerComp2->SetOtherPlayer(diggerPtr1);
	diggerComp2->SetDiamonds(diamondPtrs);

	// ACHIEVEMENTS
	g_AchievementMgr = std::make_shared<dae::AchievementManager>();
	scoreObs1->AddObserver(g_AchievementMgr.get());
	scoreObs2->AddObserver(g_AchievementMgr.get());

// ---------------------------------------------------

	// GAME INSTRUCTIONS
	auto instructions1 = std::make_unique<dae::GameObject>();
	instructions1->SetLocalPosition(10, 520);
	instructions1->AddComponent<dae::TextComponent>("P1: WASD | P2: D-Pad", fontSmall, SDL_Color{ 255, 255, 0, 255 });
	scene.Add(std::move(instructions1));

	auto instructions2 = std::make_unique<dae::GameObject>();
	instructions2->SetLocalPosition(10, 550);
	instructions2->AddComponent<dae::TextComponent>("POINTS: Eat Diamonds | LIVES: Touching each other", fontSmall, SDL_Color{ 255, 255, 0, 255 });
	scene.Add(std::move(instructions2));

// ---------------------------------------------------

	// SOUND SYSTEM - play the main music
	soundSystem.play(DiggerSounds::MUSIC, 0.5f);
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

	dae::ServiceLocator::register_sound_system(nullptr);

	return 0;
}
