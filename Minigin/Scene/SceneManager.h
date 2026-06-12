#pragma once
#include <vector>
#include <string>
#include <memory>
#include "Scene.h"
#include "Singleton.h"

namespace dae
{
	class Scene;
	class SceneManager final : public Singleton<SceneManager>
	{
	public:
		Scene& CreateScene();

		void Update(float deltaTime);
		void Render();

		void SetActiveScene(Scene* pScene);
		Scene* GetActiveScene() const { return m_pActiveScene; }
		Scene* GetScene(size_t index) const { return (index < m_scenes.size()) ? m_scenes[index].get() : nullptr; }

	private:
		friend class Singleton<SceneManager>;
		SceneManager() = default;
		std::vector<std::unique_ptr<Scene>> m_scenes{};

		Scene* m_pActiveScene{ nullptr };
	};
}