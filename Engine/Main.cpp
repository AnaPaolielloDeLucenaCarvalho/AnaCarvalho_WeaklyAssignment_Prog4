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

#include <filesystem>
namespace fs = std::filesystem;

static void load()
{
	auto& scene = dae::SceneManager::GetInstance().CreateScene();

	auto go = std::make_unique<dae::GameObject>();
	go->AddComponent<dae::RenderComponent>("background.png");
	scene.Add(std::move(go));

	go = std::make_unique<dae::GameObject>();
	go->AddComponent<dae::RenderComponent>("logo.png");
	go->SetLocalPosition(358, 180);
	scene.Add(std::move(go));

	auto font = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 36);

	auto textObj = std::make_unique<dae::GameObject>();
	textObj->AddComponent<dae::TextComponent>("Programming 4 Assignment", font, SDL_Color{ 255, 255, 0, 255 });
	textObj->SetLocalPosition(292, 20);
	scene.Add(std::move(textObj));

	// FPS Counter
	auto fpsObject = std::make_unique<dae::GameObject>();
	fpsObject->AddComponent<dae::TextComponent>("0 FPS", font, SDL_Color{ 255, 255, 0, 255 });
	fpsObject->AddComponent<dae::FPSComponent>();
	fpsObject->SetLocalPosition(10, 10);
	scene.Add(std::move(fpsObject));

	// --- W02 ROTATION TEST ---
	//// Root Object (Center of screen, stationary)
	//auto pivotObj = std::make_unique<dae::GameObject>();
	//pivotObj->SetLocalPosition(358, 180);
	//auto pivotPtr = pivotObj.get();
	//scene.Add(std::move(pivotObj));

	//// 1 Child (Rotates around Root)
	//auto char1Obj = std::make_unique<dae::GameObject>();
	//char1Obj->AddComponent<dae::RenderComponent>("cat.png");
	//char1Obj->AddComponent<dae::RotatorComponent>(50.0f, 2.f); // Radius 50, Speed 2
	//char1Obj->SetParent(pivotPtr, false);
	//auto char1Ptr = char1Obj.get();
	//scene.Add(std::move(char1Obj));

	//// 2 Child (Rotates around 1 Child)
	//auto char2Obj = std::make_unique<dae::GameObject>();
	//char2Obj->AddComponent<dae::RenderComponent>("cat.png");
	//char2Obj->AddComponent<dae::RotatorComponent>(150.0f, -3.0f);
	//char2Obj->SetParent(char1Ptr, false);
	//scene.Add(std::move(char2Obj));

	auto go2 = std::make_unique<dae::GameObject>();
	go2->AddComponent<dae::TrashCacheComponent>();
	scene.Add(std::move(go2));
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
