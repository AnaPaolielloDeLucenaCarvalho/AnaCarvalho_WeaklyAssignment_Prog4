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
#include "MuteCommand.h"
#include "GoldBagComponent.h"
#include "LevelManager.h"
#include "DiggerState.h"
#include "MenuManager.h"
#include "SkipLevelCommand.h"
#include "ShootCommand.h"
#include "DiggerSounds.h"

#if USE_STEAMWORKS
#pragma warning (push)
#pragma warning (disable:4996)
#include <steam_api.h>
#pragma warning (pop)
#endif

#include <filesystem>
#include "EnemyComponent.h"
#include "EnemySpawnerComponent.h"
#include "LevelTransitionManager.h"   // extracted from Main.cpp in Step 1
#include <iostream>
namespace fs = std::filesystem;

std::shared_ptr<dae::AchievementManager> g_AchievementMgr = nullptr;


static void load()
{
// ----------------- SOUND -----------------

	auto miniaudioSystem = std::make_unique<dae::MiniaudioSoundSystem>();
	auto loggingSoundSystem = std::make_unique<dae::LoggingSoundSystem>(std::move(miniaudioSystem));
	dae::ServiceLocator::RegisterSoundSystem(std::move(loggingSoundSystem));

	auto& soundSystem = dae::ServiceLocator::GetSoundSystem();

#ifdef __EMSCRIPTEN__
	const std::string soundFolder = "Sounds/";
#else
	const std::string soundFolder = "Data/Sounds/";
#endif

	soundSystem.LoadSound(DiggerSounds::MUSIC, soundFolder + "main_music.wav");
	soundSystem.LoadSound(DiggerSounds::BONUS, soundFolder + "bonus.wav");
	soundSystem.LoadSound(DiggerSounds::NEXT_LEVEL, soundFolder + "next_level.wav");
	soundSystem.LoadSound(DiggerSounds::DEATH, soundFolder + "death.wav");

// ----------------- SCENE SETUP -----------------

	// 2 scenes
	auto& menuScene = dae::SceneManager::GetInstance().CreateScene();
	auto& gameScene = dae::SceneManager::GetInstance().CreateScene();

	auto& input = dae::InputManager::GetInstance();

	auto fontLarge = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 36);
	auto fontSmall = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 22);

// ----------------- MENU SETUP -----------------

	// TODO - make this pretty and correct to digger (all the game modes here)
	auto titleObj = std::make_unique<dae::GameObject>();
	auto titleRender = titleObj->AddComponent<dae::RenderComponent>("PNG/Other/CTITLE.png");
	titleRender->SetScale(2.75f);
	titleObj->SetLocalPosition(70.f, 30.f);
	menuScene.Add(std::move(titleObj));

	auto startText = std::make_unique<dae::GameObject>();
	startText->AddComponent<dae::TextComponent>("PRESS ANY KEY TO START", fontLarge, SDL_Color{ 255, 255, 0, 255 });
	startText->SetLocalPosition(320, 400);
	startText->AddComponent<dae::MenuManager>(&gameScene);
	menuScene.Add(std::move(startText));

	// Start on the menu scene
	dae::SceneManager::GetInstance().SetActiveScene(&menuScene);


// ----------------- GAME SCENE SETUP -----------------

	auto hudBg = std::make_unique<dae::GameObject>();
	hudBg->AddComponent<dae::UIPanelComponent>(1024.f, 52.f, SDL_Color{ 0, 0, 0, 255 });
	hudBg->SetLocalPosition(0, 0);
	hudBg->SetZIndex(10);
	gameScene.Add(std::move(hudBg));

	// Black background for the level play area
	auto levelBg = std::make_unique<dae::GameObject>();
	levelBg->AddComponent<dae::UIPanelComponent>(1040.f, 612.f, SDL_Color{ 0, 0, 0, 255 });
	levelBg->SetLocalPosition(0, 0);
	levelBg->SetZIndex(-1);
	gameScene.Add(std::move(levelBg));

	auto fpsObject = std::make_unique<dae::GameObject>();
	fpsObject->AddComponent<dae::TextComponent>("0 FPS", fontSmall, SDL_Color{ 150, 150, 150, 255 });
	fpsObject->AddComponent<dae::FPSComponent>();
	fpsObject->SetLocalPosition(475, 15);
	fpsObject->SetZIndex(10);
	gameScene.Add(std::move(fpsObject));

	auto digger1 = std::make_unique<dae::GameObject>();
	digger1->AddComponent<dae::RenderComponent>("PNG/Digger/VRDIG1X.png");
	auto diggerComp1 = digger1->AddComponent<dae::DiggerComponent>();
	auto diggerPtr1 = digger1.get();
	digger1->SetZIndex(5);
	gameScene.Add(std::move(digger1));

	auto digger2 = std::make_unique<dae::GameObject>();
	digger2->AddComponent<dae::RenderComponent>("PNG/Enemy/VRDIG1X.png");
	auto diggerComp2 = digger2->AddComponent<dae::DiggerComponent>();
	auto diggerPtr2 = digger2.get();
	digger2->SetZIndex(5);
	gameScene.Add(std::move(digger2));

	diggerComp1->SetOtherPlayer(diggerPtr2);
	diggerComp2->SetOtherPlayer(diggerPtr1);

	auto scoreUI1 = std::make_unique<dae::GameObject>();
	scoreUI1->SetLocalPosition(20, 13);
	auto scoreObs1 = scoreUI1->AddComponent<dae::SpriteScoreDisplayComponent>("PNG/UI/VNUM", ".png", 24.f);
	scoreUI1->SetZIndex(10);
	gameScene.Add(std::move(scoreUI1));

	auto livesUI1 = std::make_unique<dae::GameObject>();
	livesUI1->SetLocalPosition(150, 15);
	auto livesObs1 = livesUI1->AddComponent<dae::LivesSpriteDisplayComponent>("PNG/Digger/VRDIG1X.png", 4, 35.5f);
	livesUI1->SetZIndex(10);
	gameScene.Add(std::move(livesUI1));

	auto scoreUI2 = std::make_unique<dae::GameObject>();
	scoreUI2->SetLocalPosition(810, 13);
	auto scoreObs2 = scoreUI2->AddComponent<dae::SpriteScoreDisplayComponent>("PNG/UI/VNUM", ".png", 24.f);
	scoreUI2->SetZIndex(10);
	gameScene.Add(std::move(scoreUI2));

	auto livesUI2 = std::make_unique<dae::GameObject>();
	livesUI2->SetLocalPosition(940, 15);
	auto livesObs2 = livesUI2->AddComponent<dae::LivesSpriteDisplayComponent>("PNG/Digger/VRDIG1X.png", 4, 35.5f);
	livesUI2->SetZIndex(10);
	gameScene.Add(std::move(livesUI2));

	diggerComp1->AddObserver(scoreObs1);
	diggerComp1->AddObserver(livesObs1);
	diggerComp2->AddObserver(scoreObs2);
	diggerComp2->AddObserver(livesObs2);

	input.BindCommand(SDL_SCANCODE_W, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(diggerComp1, glm::vec2{ 0, -1 }));
	input.BindCommand(SDL_SCANCODE_S, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(diggerComp1, glm::vec2{ 0, 1 }));
	input.BindCommand(SDL_SCANCODE_A, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(diggerComp1, glm::vec2{ -1, 0 }));
	input.BindCommand(SDL_SCANCODE_D, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(diggerComp1, glm::vec2{ 1, 0 }));
	input.BindCommand(SDL_SCANCODE_SPACE, dae::KeyState::Pressed, std::make_unique<dae::ShootCommand>(diggerComp1));

	input.BindCommand(0, dae::Gamepad::ControllerButton::DPadUp, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(diggerComp2, glm::vec2{ 0, -1 }));
	input.BindCommand(0, dae::Gamepad::ControllerButton::DPadDown, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(diggerComp2, glm::vec2{ 0, 1 }));
	input.BindCommand(0, dae::Gamepad::ControllerButton::DPadLeft, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(diggerComp2, glm::vec2{ -1, 0 }));
	input.BindCommand(0, dae::Gamepad::ControllerButton::DPadRight, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(diggerComp2, glm::vec2{ 1, 0 }));

	auto muteCommand = std::make_unique<dae::MuteCommand>();
	input.BindCommand(SDL_SCANCODE_F2, dae::KeyState::Pressed, std::move(muteCommand));

	auto skipLevelCommand1 = std::make_unique<dae::SkipLevelCommand>(diggerComp1);
	input.BindCommand(SDL_SCANCODE_F1, dae::KeyState::Pressed, std::move(skipLevelCommand1));

	g_AchievementMgr = std::make_shared<dae::AchievementManager>();
	scoreObs1->AddObserver(g_AchievementMgr.get());
	scoreObs2->AddObserver(g_AchievementMgr.get());

	auto instructions1 = std::make_unique<dae::GameObject>();
	instructions1->SetLocalPosition(10, 520);
	instructions1->AddComponent<dae::TextComponent>("P1: WASD | P2: D-Pad", fontSmall, SDL_Color{ 255, 255, 0, 255 });
	instructions1->SetZIndex(10);
	gameScene.Add(std::move(instructions1));

	auto instructions2 = std::make_unique<dae::GameObject>();
	instructions2->SetLocalPosition(10, 550);
	instructions2->AddComponent<dae::TextComponent>("POINTS: Diamonds and Gold | LIVES: Getting squashed by a gold bag", fontSmall, SDL_Color{ 255, 255, 0, 255 });
	instructions2->SetZIndex(10);
	gameScene.Add(std::move(instructions2));

	auto instructions3 = std::make_unique<dae::GameObject>();
	instructions3->SetLocalPosition(10, 580);
	instructions3->AddComponent<dae::TextComponent>("F2 to mute/unmute sound", fontSmall, SDL_Color{ 255, 255, 0, 255 });
	instructions3->SetZIndex(10);
	gameScene.Add(std::move(instructions3));

	auto instructions4 = std::make_unique<dae::GameObject>();
	instructions4->SetLocalPosition(10, 490);
	instructions4->AddComponent<dae::TextComponent>("SPACE to shoot", fontSmall, SDL_Color{ 255, 255, 0, 255 });
	instructions4->SetZIndex(10);
	gameScene.Add(std::move(instructions4));

	soundSystem.PlayMusic(DiggerSounds::MUSIC, 0.5f, true);

#ifdef __EMSCRIPTEN__
	std::string levelPath = "Levels.txt";
#else
	std::string levelPath = "./Data/Levels.txt";
	if (!fs::exists(levelPath))
	{
		levelPath = "../Data/Levels.txt";
	}
#endif

	dae::LevelManager::GetInstance().LoadAllLevelsFromFile(levelPath);

	auto transMgrObj = std::make_unique<dae::GameObject>();
	auto transComp = transMgrObj->AddComponent<dae::LevelTransitionManager>(&gameScene, diggerComp1, diggerComp2);

	diggerComp1->AddObserver(transComp);
	diggerComp2->AddObserver(transComp);
	gameScene.Add(std::move(transMgrObj));

	transComp->LoadLevel(0);
}

int main(int, char* [])
{
#if USE_STEAMWORKS
	if (!SteamAPI_Init())
	{
		std::cout << "SteamAPI_Init() failed! Achievements will not work." << std::endl;
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
#if _DEBUG && __has_include(<vld.h>)
		VLDDisable();
#endif

		dae::Minigin engine(data_location);

#if _DEBUG && __has_include(<vld.h>)
		VLDEnable();
#endif

		engine.Run(load);
	}

#if USE_STEAMWORKS
	SteamAPI_Shutdown();
	std::cout << "SteamAPI Shutdown." << std::endl;
#endif
	return 0;
}