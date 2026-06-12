#include "SceneManager.h"
#include "Scene.h"

namespace dae
{
	void SceneManager::Update(float deltaTime)
	{
		if (m_pActiveScene) 
		{
			m_pActiveScene->Update(deltaTime);
		}
	}

	void SceneManager::Render()
	{
		if (m_pActiveScene) 
		{
			m_pActiveScene->Render();
		}
	}

	Scene& SceneManager::CreateScene()
	{
		m_scenes.emplace_back(new Scene());

		if (!m_pActiveScene) 
		{
			m_pActiveScene = m_scenes.back().get();
		}

		return *m_scenes.back();
	}

	void SceneManager::SetActiveScene(Scene* pScene)
	{
		m_pActiveScene = pScene;
	}
}