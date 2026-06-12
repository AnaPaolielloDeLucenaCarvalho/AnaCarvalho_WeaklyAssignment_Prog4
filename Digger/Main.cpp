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
#include "HighScoreManager.h"
#include "NameEntryComponent.h"
#include "CycleLetterCommand.h"
#include "AdvanceIndexCommand.h"
#include "ConfirmNameCommand.h"
#include "SystemCommands.h"
#include "UICommands.h"

#if USE_STEAMWORKS
#pragma warning (push)
#pragma warning (disable:4996)
#include <steam_api.h>
#pragma warning (pop)
#endif

#include <filesystem>
#include "EnemyComponent.h"
#include "EnemySpawnerComponent.h"
#include "LevelTransitionManager.h"
#include "GameOverManager.h"
#include <iostream>
#include <sstream>
#include <iomanip>
namespace fs = std::filesystem;

std::shared_ptr<dae::AchievementManager> g_AchievementMgr = nullptr;

static std::unique_ptr<dae::HighScoreManager> g_HighScoreMgr = nullptr;


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
	soundSystem.LoadSound(DiggerSounds::PICK_UP, soundFolder + "pickUp.mp3");
	soundSystem.LoadSound(DiggerSounds::BONUS_PICKUP, soundFolder + "bonusPickUp.mp3");
	soundSystem.LoadSound(DiggerSounds::COMBO_8_EMES, soundFolder + "8ComboEmes.mp3");
	soundSystem.LoadSound(DiggerSounds::KILL_ENEMY, soundFolder + "killEnemy.mp3");
	soundSystem.LoadSound(DiggerSounds::SHOOT, soundFolder + "shoot.mp3");

// ----------------- SCENE SETUP -----------------

	auto& menuScene  = dae::SceneManager::GetInstance().CreateScene();
	auto& scoreScene = dae::SceneManager::GetInstance().CreateScene();
	auto& gameScene  = dae::SceneManager::GetInstance().CreateScene();
	auto& gameOverScene = dae::SceneManager::GetInstance().CreateScene();
	auto& instructionsScene = dae::SceneManager::GetInstance().CreateScene();

	// Create the HighScoreManager — no global, no static allocation before main().
	g_HighScoreMgr = std::make_unique<dae::HighScoreManager>();
	dae::HighScoreManager* pMgr = g_HighScoreMgr.get();

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

	auto topScores = pMgr->GetTopScores(5);

	auto leaderboardTitle = std::make_unique<dae::GameObject>();
	leaderboardTitle->AddComponent<dae::TextComponent>("TOP 5 PLAYERS", fontLarge, SDL_Color{ 0, 255, 255, 255 }); // Cyan
	leaderboardTitle->SetLocalPosition(175.f, 210.f);
	menuScene.Add(std::move(leaderboardTitle));

	auto headerRank = std::make_unique<dae::GameObject>();
	headerRank->AddComponent<dae::TextComponent>("RANK", fontSmall, SDL_Color{ 255, 255, 255, 255 });
	headerRank->SetLocalPosition(150.f, 260.f);
	menuScene.Add(std::move(headerRank));

	auto headerScore = std::make_unique<dae::GameObject>();
	headerScore->AddComponent<dae::TextComponent>("SCORE", fontSmall, SDL_Color{ 255, 255, 255, 255 });
	headerScore->SetLocalPosition(250.f, 260.f);
	menuScene.Add(std::move(headerScore));

	auto headerName = std::make_unique<dae::GameObject>();
	headerName->AddComponent<dae::TextComponent>("NAME", fontSmall, SDL_Color{ 255, 255, 255, 255 });
	headerName->SetLocalPosition(400.f, 260.f);
	menuScene.Add(std::move(headerName));

	std::vector<dae::TextComponent*> menuScoreTexts;
	std::vector<dae::TextComponent*> menuNameTexts;

	for (size_t i = 0; i < topScores.size(); ++i)
	{
		float currentY = 290.f + static_cast<float>(i) * 30.0f;

		auto rankObj = std::make_unique<dae::GameObject>();
		rankObj->AddComponent<dae::TextComponent>(std::to_string(i + 1), fontSmall, SDL_Color{ 255, 255, 255, 255 });
		rankObj->SetLocalPosition(150.f, currentY);
		menuScene.Add(std::move(rankObj));

		std::ostringstream scoreOss;
		scoreOss << std::setfill('0') << std::setw(5) << topScores[i].score;
		auto scoreObj = std::make_unique<dae::GameObject>();
		auto pScoreText = scoreObj->AddComponent<dae::TextComponent>(scoreOss.str(), fontSmall, SDL_Color{ 255, 255, 255, 255 });
		menuScoreTexts.push_back(pScoreText);
		scoreObj->SetLocalPosition(250.f, currentY);
		menuScene.Add(std::move(scoreObj));

		auto nameObj = std::make_unique<dae::GameObject>();
		auto pNameText = nameObj->AddComponent<dae::TextComponent>(topScores[i].initials, fontSmall, SDL_Color{ 255, 255, 255, 255 });
		menuNameTexts.push_back(pNameText);
		nameObj->SetLocalPosition(400.f, currentY);
		menuScene.Add(std::move(nameObj));
	}

	auto opt1Obj = std::make_unique<dae::GameObject>();
	auto opt1Text = opt1Obj->AddComponent<dae::TextComponent>("1 PLAYER CLASSIC", fontSmall, SDL_Color{ 255, 255, 255, 255 });
	opt1Obj->SetLocalPosition(600.f, 280.f);
	menuScene.Add(std::move(opt1Obj));

	auto opt2Obj = std::make_unique<dae::GameObject>();
	auto opt2Text = opt2Obj->AddComponent<dae::TextComponent>("2 PLAYER CO-OP", fontSmall, SDL_Color{ 255, 255, 255, 255 });
	opt2Obj->SetLocalPosition(600.f, 320.f);
	menuScene.Add(std::move(opt2Obj));

	auto opt3Obj = std::make_unique<dae::GameObject>();
	auto opt3Text = opt3Obj->AddComponent<dae::TextComponent>("2 PLAYER VERSUS", fontSmall, SDL_Color{ 255, 255, 255, 255 });
	opt3Obj->SetLocalPosition(600.f, 360.f);
	menuScene.Add(std::move(opt3Obj));

	std::vector<dae::TextComponent*> menuOptions = { opt1Text, opt2Text, opt3Text };

	auto menuManagerObj = std::make_unique<dae::GameObject>();
	auto pMenuMgr = menuManagerObj->AddComponent<dae::MenuManager>(pMgr, &scoreScene, &gameScene, menuOptions, menuScoreTexts, menuNameTexts);
	menuScene.Add(std::move(menuManagerObj));

	input.BindCommand(SDL_SCANCODE_W, dae::KeyState::Pressed, std::make_unique<dae::MenuNavigateCommand>(-1, pMenuMgr, nullptr, &menuScene));
	input.BindCommand(SDL_SCANCODE_UP, dae::KeyState::Pressed, std::make_unique<dae::MenuNavigateCommand>(-1, pMenuMgr, nullptr, &menuScene));
	input.BindCommand(0, dae::Gamepad::ControllerButton::DPadUp, dae::KeyState::Pressed, std::make_unique<dae::MenuNavigateCommand>(-1, pMenuMgr, nullptr, &menuScene));
	input.BindCommand(1, dae::Gamepad::ControllerButton::DPadUp, dae::KeyState::Pressed, std::make_unique<dae::MenuNavigateCommand>(-1, pMenuMgr, nullptr, &menuScene));

	input.BindCommand(SDL_SCANCODE_S, dae::KeyState::Pressed, std::make_unique<dae::MenuNavigateCommand>(1, pMenuMgr, nullptr, &menuScene));
	input.BindCommand(SDL_SCANCODE_DOWN, dae::KeyState::Pressed, std::make_unique<dae::MenuNavigateCommand>(1, pMenuMgr, nullptr, &menuScene));
	input.BindCommand(0, dae::Gamepad::ControllerButton::DPadDown, dae::KeyState::Pressed, std::make_unique<dae::MenuNavigateCommand>(1, pMenuMgr, nullptr, &menuScene));
	input.BindCommand(1, dae::Gamepad::ControllerButton::DPadDown, dae::KeyState::Pressed, std::make_unique<dae::MenuNavigateCommand>(1, pMenuMgr, nullptr, &menuScene));

	input.BindCommand(SDL_SCANCODE_SPACE, dae::KeyState::Pressed, std::make_unique<dae::MenuSelectCommand>(pMenuMgr, nullptr, &menuScene));
	input.BindCommand(SDL_SCANCODE_RETURN, dae::KeyState::Pressed, std::make_unique<dae::MenuSelectCommand>(pMenuMgr, nullptr, &menuScene));
	input.BindCommand(0, dae::Gamepad::ControllerButton::ButtonA, dae::KeyState::Pressed, std::make_unique<dae::MenuSelectCommand>(pMenuMgr, nullptr, &menuScene));
	input.BindCommand(1, dae::Gamepad::ControllerButton::ButtonA, dae::KeyState::Pressed, std::make_unique<dae::MenuSelectCommand>(pMenuMgr, nullptr, &menuScene));
	input.BindCommand(0, dae::Gamepad::ControllerButton::LeftShoulder, dae::KeyState::Pressed, std::make_unique<dae::MenuSelectCommand>(pMenuMgr, nullptr, &menuScene));
	input.BindCommand(1, dae::Gamepad::ControllerButton::LeftShoulder, dae::KeyState::Pressed, std::make_unique<dae::MenuSelectCommand>(pMenuMgr, nullptr, &menuScene));
	input.BindCommand(0, dae::Gamepad::ControllerButton::RightShoulder, dae::KeyState::Pressed, std::make_unique<dae::MenuSelectCommand>(pMenuMgr, nullptr, &menuScene));
	input.BindCommand(1, dae::Gamepad::ControllerButton::RightShoulder, dae::KeyState::Pressed, std::make_unique<dae::MenuSelectCommand>(pMenuMgr, nullptr, &menuScene));

	// Start on the menu scene
	dae::SceneManager::GetInstance().SetActiveScene(&menuScene);


// ----------------- SCORE SCENE (NAME ENTRY) SETUP -----------------

	{
		auto scoreBg = std::make_unique<dae::GameObject>();
		scoreBg->AddComponent<dae::UIPanelComponent>(1040.f, 612.f, SDL_Color{ 0, 0, 0, 255 });
		scoreBg->SetZIndex(-1);
		scoreScene.Add(std::move(scoreBg));

		auto promptObj = std::make_unique<dae::GameObject>();
		promptObj->AddComponent<dae::TextComponent>("ENTER YOUR INITIALS", fontLarge, SDL_Color{ 255, 255, 0, 255 });
		promptObj->SetLocalPosition(330, 200);
		scoreScene.Add(std::move(promptObj));

		auto hintsObj = std::make_unique<dae::GameObject>();
		hintsObj->AddComponent<dae::TextComponent>("UP/DOWN: change letter   RIGHT: next   ENTER: confirm", fontSmall, SDL_Color{ 180, 180, 180, 255 });
		hintsObj->SetLocalPosition(200, 440);
		scoreScene.Add(std::move(hintsObj));

		// The text object that NameEntryComponent drives.
		auto initialsObj = std::make_unique<dae::GameObject>();
		initialsObj->SetLocalPosition(430, 300);
		auto pInitialsText = initialsObj->AddComponent<dae::TextComponent>("A A A", fontLarge, SDL_Color{ 255, 255, 255, 255 });
		auto pEntryComp    = initialsObj->AddComponent<dae::NameEntryComponent>(pInitialsText, pMgr, &gameScene);
		scoreScene.Add(std::move(initialsObj));

		// Bind name-entry keys — these live on top of any game bindings.
		// W / Up to cycle letter UP (-1)
		input.BindCommand(SDL_SCANCODE_W, dae::KeyState::Pressed, std::make_unique<dae::CycleLetterCommand>(pEntryComp, -1, &scoreScene));
		input.BindCommand(SDL_SCANCODE_UP, dae::KeyState::Pressed, std::make_unique<dae::CycleLetterCommand>(pEntryComp, -1, &scoreScene));
		input.BindCommand(0, dae::Gamepad::ControllerButton::DPadUp, dae::KeyState::Pressed, std::make_unique<dae::CycleLetterCommand>(pEntryComp, -1, &scoreScene));
		input.BindCommand(1, dae::Gamepad::ControllerButton::DPadUp, dae::KeyState::Pressed, std::make_unique<dae::CycleLetterCommand>(pEntryComp, -1, &scoreScene));

		// S / Down to cycle letter DOWN (+1)
		input.BindCommand(SDL_SCANCODE_S, dae::KeyState::Pressed, std::make_unique<dae::CycleLetterCommand>(pEntryComp, +1, &scoreScene));
		input.BindCommand(SDL_SCANCODE_DOWN, dae::KeyState::Pressed, std::make_unique<dae::CycleLetterCommand>(pEntryComp, +1, &scoreScene));
		input.BindCommand(0, dae::Gamepad::ControllerButton::DPadDown, dae::KeyState::Pressed, std::make_unique<dae::CycleLetterCommand>(pEntryComp, +1, &scoreScene));
		input.BindCommand(1, dae::Gamepad::ControllerButton::DPadDown, dae::KeyState::Pressed, std::make_unique<dae::CycleLetterCommand>(pEntryComp, +1, &scoreScene));

		// Next Letter / Index
		input.BindCommand(SDL_SCANCODE_D, dae::KeyState::Pressed, std::make_unique<dae::AdvanceIndexCommand>(pEntryComp, 1, &scoreScene));
		input.BindCommand(SDL_SCANCODE_RIGHT, dae::KeyState::Pressed, std::make_unique<dae::AdvanceIndexCommand>(pEntryComp, 1, &scoreScene));
		input.BindCommand(0, dae::Gamepad::ControllerButton::DPadRight, dae::KeyState::Pressed, std::make_unique<dae::AdvanceIndexCommand>(pEntryComp, 1, &scoreScene));
		input.BindCommand(1, dae::Gamepad::ControllerButton::DPadRight, dae::KeyState::Pressed, std::make_unique<dae::AdvanceIndexCommand>(pEntryComp, 1, &scoreScene));

		// Previous Letter / Index
		input.BindCommand(SDL_SCANCODE_A, dae::KeyState::Pressed, std::make_unique<dae::AdvanceIndexCommand>(pEntryComp, -1, &scoreScene));
		input.BindCommand(SDL_SCANCODE_LEFT, dae::KeyState::Pressed, std::make_unique<dae::AdvanceIndexCommand>(pEntryComp, -1, &scoreScene));
		input.BindCommand(0, dae::Gamepad::ControllerButton::DPadLeft, dae::KeyState::Pressed, std::make_unique<dae::AdvanceIndexCommand>(pEntryComp, -1, &scoreScene));
		input.BindCommand(1, dae::Gamepad::ControllerButton::DPadLeft, dae::KeyState::Pressed, std::make_unique<dae::AdvanceIndexCommand>(pEntryComp, -1, &scoreScene));

		// Confirm
		input.BindCommand(SDL_SCANCODE_SPACE, dae::KeyState::Pressed, std::make_unique<dae::ConfirmNameCommand>(pEntryComp, &scoreScene));
		input.BindCommand(SDL_SCANCODE_RETURN,dae::KeyState::Pressed, std::make_unique<dae::ConfirmNameCommand>(pEntryComp, &scoreScene));
		input.BindCommand(0, dae::Gamepad::ControllerButton::ButtonA, dae::KeyState::Pressed, std::make_unique<dae::ConfirmNameCommand>(pEntryComp, &scoreScene));
		input.BindCommand(1, dae::Gamepad::ControllerButton::ButtonA, dae::KeyState::Pressed, std::make_unique<dae::ConfirmNameCommand>(pEntryComp, &scoreScene));
		input.BindCommand(0, dae::Gamepad::ControllerButton::LeftShoulder, dae::KeyState::Pressed, std::make_unique<dae::ConfirmNameCommand>(pEntryComp, &scoreScene));
		input.BindCommand(1, dae::Gamepad::ControllerButton::LeftShoulder, dae::KeyState::Pressed, std::make_unique<dae::ConfirmNameCommand>(pEntryComp, &scoreScene));
		input.BindCommand(0, dae::Gamepad::ControllerButton::RightShoulder, dae::KeyState::Pressed, std::make_unique<dae::ConfirmNameCommand>(pEntryComp, &scoreScene));
		input.BindCommand(1, dae::Gamepad::ControllerButton::RightShoulder, dae::KeyState::Pressed, std::make_unique<dae::ConfirmNameCommand>(pEntryComp, &scoreScene));
	}


// ----------------- GAME OVER SCENE SETUP -----------------

	{
		auto goTitleObj = std::make_unique<dae::GameObject>();
		auto pTitleText = goTitleObj->AddComponent<dae::TextComponent>("GAME OVER", fontLarge, SDL_Color{ 255, 0, 0, 255 });
		goTitleObj->SetLocalPosition(380, 150);
		gameOverScene.Add(std::move(goTitleObj));

		auto scoreObj = std::make_unique<dae::GameObject>();
		auto pScoreText = scoreObj->AddComponent<dae::TextComponent>("FINAL SCORE: 0", fontLarge, SDL_Color{ 255, 255, 255, 255 });
		scoreObj->SetLocalPosition(380, 250);
		gameOverScene.Add(std::move(scoreObj));

		auto goOpt1Obj = std::make_unique<dae::GameObject>();
		auto pOpt1Text = goOpt1Obj->AddComponent<dae::TextComponent>("TRY AGAIN", fontSmall, SDL_Color{ 255, 255, 255, 255 });
		goOpt1Obj->SetLocalPosition(430, 400);
		gameOverScene.Add(std::move(goOpt1Obj));

		auto goOpt2Obj = std::make_unique<dae::GameObject>();
		auto pOpt2Text = goOpt2Obj->AddComponent<dae::TextComponent>("MAIN MENU", fontSmall, SDL_Color{ 255, 255, 255, 255 });
		goOpt2Obj->SetLocalPosition(430, 450);
		gameOverScene.Add(std::move(goOpt2Obj));

		std::vector<dae::TextComponent*> options = { pOpt1Text, pOpt2Text };

		auto managerObj = std::make_unique<dae::GameObject>();
		auto pGoMgr = managerObj->AddComponent<dae::GameOverManager>(&menuScene, &gameScene, pMgr, pTitleText, pScoreText, options);
		gameOverScene.Add(std::move(managerObj));

		input.BindCommand(SDL_SCANCODE_W, dae::KeyState::Pressed, std::make_unique<dae::MenuNavigateCommand>(-1, nullptr, pGoMgr, &gameOverScene));
		input.BindCommand(SDL_SCANCODE_UP, dae::KeyState::Pressed, std::make_unique<dae::MenuNavigateCommand>(-1, nullptr, pGoMgr, &gameOverScene));
		input.BindCommand(0, dae::Gamepad::ControllerButton::DPadUp, dae::KeyState::Pressed, std::make_unique<dae::MenuNavigateCommand>(-1, nullptr, pGoMgr, &gameOverScene));
		input.BindCommand(1, dae::Gamepad::ControllerButton::DPadUp, dae::KeyState::Pressed, std::make_unique<dae::MenuNavigateCommand>(-1, nullptr, pGoMgr, &gameOverScene));

		input.BindCommand(SDL_SCANCODE_S, dae::KeyState::Pressed, std::make_unique<dae::MenuNavigateCommand>(1, nullptr, pGoMgr, &gameOverScene));
		input.BindCommand(SDL_SCANCODE_DOWN, dae::KeyState::Pressed, std::make_unique<dae::MenuNavigateCommand>(1, nullptr, pGoMgr, &gameOverScene));
		input.BindCommand(0, dae::Gamepad::ControllerButton::DPadDown, dae::KeyState::Pressed, std::make_unique<dae::MenuNavigateCommand>(1, nullptr, pGoMgr, &gameOverScene));
		input.BindCommand(1, dae::Gamepad::ControllerButton::DPadDown, dae::KeyState::Pressed, std::make_unique<dae::MenuNavigateCommand>(1, nullptr, pGoMgr, &gameOverScene));

		input.BindCommand(SDL_SCANCODE_SPACE, dae::KeyState::Pressed, std::make_unique<dae::MenuSelectCommand>(nullptr, pGoMgr, &gameOverScene));
		input.BindCommand(SDL_SCANCODE_RETURN, dae::KeyState::Pressed, std::make_unique<dae::MenuSelectCommand>(nullptr, pGoMgr, &gameOverScene));
		input.BindCommand(0, dae::Gamepad::ControllerButton::ButtonA, dae::KeyState::Pressed, std::make_unique<dae::MenuSelectCommand>(nullptr, pGoMgr, &gameOverScene));
		input.BindCommand(1, dae::Gamepad::ControllerButton::ButtonA, dae::KeyState::Pressed, std::make_unique<dae::MenuSelectCommand>(nullptr, pGoMgr, &gameOverScene));
		input.BindCommand(0, dae::Gamepad::ControllerButton::LeftShoulder, dae::KeyState::Pressed, std::make_unique<dae::MenuSelectCommand>(nullptr, pGoMgr, &gameOverScene));
		input.BindCommand(1, dae::Gamepad::ControllerButton::LeftShoulder, dae::KeyState::Pressed, std::make_unique<dae::MenuSelectCommand>(nullptr, pGoMgr, &gameOverScene));
		input.BindCommand(0, dae::Gamepad::ControllerButton::RightShoulder, dae::KeyState::Pressed, std::make_unique<dae::MenuSelectCommand>(nullptr, pGoMgr, &gameOverScene));
		input.BindCommand(1, dae::Gamepad::ControllerButton::RightShoulder, dae::KeyState::Pressed, std::make_unique<dae::MenuSelectCommand>(nullptr, pGoMgr, &gameOverScene));
	}

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

	auto p1LabelObj = std::make_unique<dae::GameObject>();
	p1LabelObj->AddComponent<dae::TextComponent>("PLAYER 1", fontSmall, SDL_Color{ 255, 255, 0, 255 });
	p1LabelObj->SetLocalPosition(20, 15);
	p1LabelObj->SetZIndex(11);
	gameScene.Add(std::move(p1LabelObj));

	auto p2LabelObj = std::make_unique<dae::GameObject>();
	p2LabelObj->AddComponent<dae::TextComponent>("PLAYER 2", fontSmall, SDL_Color{ 255, 255, 0, 255 });
	p2LabelObj->SetLocalPosition(905, 15);
	p2LabelObj->SetZIndex(11);
	auto p2LabelPtr = p2LabelObj.get();
	gameScene.Add(std::move(p2LabelObj));

	auto fpsObject = std::make_unique<dae::GameObject>();
	fpsObject->AddComponent<dae::TextComponent>("0 FPS", fontSmall, SDL_Color{ 150, 150, 150, 255 });
	fpsObject->AddComponent<dae::FPSComponent>();
	fpsObject->SetLocalPosition(400, 15);
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
	diggerComp2->SetIsPlayerOne(false);

	// Inject the session manager so DiggerGameOverState can call SaveScore().
	diggerComp1->SetHighScoreManager(pMgr);
	diggerComp2->SetHighScoreManager(pMgr);

	auto scoreUI1 = std::make_unique<dae::GameObject>();
	scoreUI1->SetLocalPosition(500, 13);
	auto scoreObs1 = scoreUI1->AddComponent<dae::SpriteScoreDisplayComponent>("PNG/UI/VNUM", ".png", 24.f);
	scoreUI1->SetZIndex(10);
	auto scoreUI1Ptr = scoreUI1.get();
	gameScene.Add(std::move(scoreUI1));

	auto livesUI1 = std::make_unique<dae::GameObject>();
	livesUI1->SetLocalPosition(140, 15);
	auto livesObs1 = livesUI1->AddComponent<dae::LivesSpriteDisplayComponent>("PNG/Digger/VRDIG1X.png", 4, 35.5f, false);
	livesUI1->SetZIndex(10);
	gameScene.Add(std::move(livesUI1));

	auto livesUI2 = std::make_unique<dae::GameObject>();
	livesUI2->SetLocalPosition(850, 15);
	auto livesObs2 = livesUI2->AddComponent<dae::LivesSpriteDisplayComponent>("PNG/Digger/VRDIG1X.png", 4, 35.5f, true);
	livesUI2->SetZIndex(10);
	auto livesUI2Ptr = livesUI2.get();
	gameScene.Add(std::move(livesUI2));

	diggerComp1->AddObserver(scoreObs1);
	diggerComp1->AddObserver(livesObs1);
	diggerComp2->AddObserver(livesObs2);

	input.BindCommand(SDL_SCANCODE_W, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(diggerComp1, glm::vec2{ 0, -1 }));
	input.BindCommand(SDL_SCANCODE_S, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(diggerComp1, glm::vec2{ 0, 1 }));
	input.BindCommand(SDL_SCANCODE_A, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(diggerComp1, glm::vec2{ -1, 0 }));
	input.BindCommand(SDL_SCANCODE_D, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(diggerComp1, glm::vec2{ 1, 0 }));
	input.BindCommand(SDL_SCANCODE_SPACE, dae::KeyState::Pressed, std::make_unique<dae::ShootCommand>(diggerComp1));

	input.BindCommand(0, dae::Gamepad::ControllerButton::DPadUp, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(diggerComp1, glm::vec2{ 0, -1 }));
	input.BindCommand(0, dae::Gamepad::ControllerButton::DPadDown, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(diggerComp1, glm::vec2{ 0, 1 }));
	input.BindCommand(0, dae::Gamepad::ControllerButton::DPadLeft, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(diggerComp1, glm::vec2{ -1, 0 }));
	input.BindCommand(0, dae::Gamepad::ControllerButton::DPadRight, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(diggerComp1, glm::vec2{ 1, 0 }));
	input.BindCommand(0, dae::Gamepad::ControllerButton::LeftTrigger, dae::KeyState::Pressed, std::make_unique<dae::ShootCommand>(diggerComp1));
	input.BindCommand(0, dae::Gamepad::ControllerButton::RightTrigger, dae::KeyState::Pressed, std::make_unique<dae::ShootCommand>(diggerComp1));

	input.BindCommand(SDL_SCANCODE_RSHIFT, dae::KeyState::Pressed, std::make_unique<dae::ShootCommand>(diggerComp2));
	input.BindCommand(SDL_SCANCODE_I, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(diggerComp2, glm::vec2{ 0, -1 }));
	input.BindCommand(SDL_SCANCODE_K, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(diggerComp2, glm::vec2{ 0, 1 }));
	input.BindCommand(SDL_SCANCODE_J, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(diggerComp2, glm::vec2{ -1, 0 }));
	input.BindCommand(SDL_SCANCODE_L, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(diggerComp2, glm::vec2{ 1, 0 }));

	input.BindCommand(1, dae::Gamepad::ControllerButton::DPadUp, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(diggerComp2, glm::vec2{ 0, -1 }));
	input.BindCommand(1, dae::Gamepad::ControllerButton::DPadDown, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(diggerComp2, glm::vec2{ 0, 1 }));
	input.BindCommand(1, dae::Gamepad::ControllerButton::DPadLeft, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(diggerComp2, glm::vec2{ -1, 0 }));
	input.BindCommand(1, dae::Gamepad::ControllerButton::DPadRight, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(diggerComp2, glm::vec2{ 1, 0 }));
	input.BindCommand(1, dae::Gamepad::ControllerButton::LeftTrigger, dae::KeyState::Pressed, std::make_unique<dae::ShootCommand>(diggerComp2));
	input.BindCommand(1, dae::Gamepad::ControllerButton::RightTrigger, dae::KeyState::Pressed, std::make_unique<dae::ShootCommand>(diggerComp2));

	input.BindCommand(SDL_SCANCODE_F2, dae::KeyState::Pressed, std::make_unique<dae::MuteCommand>());
	input.BindCommand(0, dae::Gamepad::ControllerButton::ButtonY, dae::KeyState::Pressed, std::make_unique<dae::MuteCommand>());
	input.BindCommand(1, dae::Gamepad::ControllerButton::ButtonY, dae::KeyState::Pressed, std::make_unique<dae::MuteCommand>());

	input.BindCommand(SDL_SCANCODE_F1, dae::KeyState::Pressed, std::make_unique<dae::SkipLevelCommand>(diggerComp1));
	input.BindCommand(0, dae::Gamepad::ControllerButton::ButtonB, dae::KeyState::Pressed, std::make_unique<dae::SkipLevelCommand>(diggerComp1));
	input.BindCommand(1, dae::Gamepad::ControllerButton::ButtonB, dae::KeyState::Pressed, std::make_unique<dae::SkipLevelCommand>(diggerComp1));

	input.BindCommand(SDL_SCANCODE_F10, dae::KeyState::Pressed, std::make_unique<dae::ReturnToMenuCommand>(&menuScene, pMgr));
	input.BindCommand(0, dae::Gamepad::ControllerButton::ButtonA, dae::KeyState::Pressed, std::make_unique<dae::ReturnToMenuCommand>(&menuScene, pMgr));
	input.BindCommand(1, dae::Gamepad::ControllerButton::ButtonA, dae::KeyState::Pressed, std::make_unique<dae::ReturnToMenuCommand>(&menuScene, pMgr));

	input.BindCommand(SDL_SCANCODE_F3, dae::KeyState::Pressed, std::make_unique<dae::ToggleInstructionsCommand>(&instructionsScene));
	input.BindCommand(0, dae::Gamepad::ControllerButton::ButtonX, dae::KeyState::Pressed, std::make_unique<dae::ToggleInstructionsCommand>(&instructionsScene));
	input.BindCommand(1, dae::Gamepad::ControllerButton::ButtonX, dae::KeyState::Pressed, std::make_unique<dae::ToggleInstructionsCommand>(&instructionsScene));

	g_AchievementMgr = std::make_shared<dae::AchievementManager>();
	scoreObs1->AddObserver(g_AchievementMgr.get());

	auto instBg = std::make_unique<dae::GameObject>();
	instBg->AddComponent<dae::UIPanelComponent>(1040.f, 612.f, SDL_Color{ 0, 0, 0, 255 });
	instBg->SetZIndex(-1);
	instructionsScene.Add(std::move(instBg));

	auto instTitle = std::make_unique<dae::GameObject>();
	instTitle->AddComponent<dae::TextComponent>("HOW TO PLAY", fontLarge, SDL_Color{ 255, 255, 0, 255 });
	instTitle->SetLocalPosition(380, 100);
	instructionsScene.Add(std::move(instTitle));

	auto inst1 = std::make_unique<dae::GameObject>();
	inst1->AddComponent<dae::TextComponent>("P1: WASD  |  P2: IJKL", fontSmall, SDL_Color{ 255, 255, 255, 255 });
	inst1->SetLocalPosition(350, 250);
	instructionsScene.Add(std::move(inst1));

	auto inst2 = std::make_unique<dae::GameObject>();
	inst2->AddComponent<dae::TextComponent>("SHOOT: Space (P1) / RShift (P2)", fontSmall, SDL_Color{ 255, 255, 255, 255 });
	inst2->SetLocalPosition(350, 300);
	instructionsScene.Add(std::move(inst2));

	auto inst3 = std::make_unique<dae::GameObject>();
	inst3->AddComponent<dae::TextComponent>("POINTS: Diamonds and Gold", fontSmall, SDL_Color{ 255, 255, 255, 255 });
	inst3->SetLocalPosition(350, 350);
	instructionsScene.Add(std::move(inst3));

	auto inst4 = std::make_unique<dae::GameObject>();
	inst4->AddComponent<dae::TextComponent>("PRESS F3 TO RETURN", fontSmall, SDL_Color{ 255, 0, 0, 255 });
	inst4->SetLocalPosition(400, 500);
	instructionsScene.Add(std::move(inst4));

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
	auto transComp = transMgrObj->AddComponent<dae::LevelTransitionManager>(&gameScene, diggerComp1, diggerComp2, p2LabelPtr, livesUI2Ptr, scoreUI1Ptr, livesObs2);
	gameScene.Add(std::move(transMgrObj));
	diggerComp1->AddObserver(transComp);
	diggerComp2->AddObserver(transComp);
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