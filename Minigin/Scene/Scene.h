#pragma once
#include <vector>
#include <memory>
#include <string>
#include "GameObject.h"


// DESIGN PATTERN - Scene Graph / Container
// Acts as a container for all the GameObjects in a specific level or menu. It handles updating and rendering them all in order, and cleaning up dead objects at the end of the frame.

namespace dae
{
	class SceneManager;

	class Scene
	{
		friend class SceneManager;

	public:
		void Add(std::unique_ptr<GameObject> object);
		void Remove(GameObject* object);
		void RemoveAll();

		void CleanUpScene();
		void RequestLevelCleanup();

		void Update(float deltaTime);
		void Render() const;

		~Scene();

		Scene(const Scene& other) = delete;
		Scene(Scene&& other) = delete;
		Scene& operator=(const Scene& other) = delete;
		Scene& operator=(Scene&& other) = delete;

	private:
		explicit Scene() = default;

		std::vector<std::unique_ptr<GameObject>> m_objects{};
		std::vector<std::unique_ptr<GameObject>> m_pendingObjects{};

		bool m_levelNeedsCleanup{ false };
	};
}