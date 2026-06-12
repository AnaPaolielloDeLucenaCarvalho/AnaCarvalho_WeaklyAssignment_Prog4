#include "Scene.h"
#include "GameObject.h"
#include <algorithm>

using namespace dae;

Scene::~Scene() = default;

void Scene::Add(std::unique_ptr<GameObject> object)
{
	m_pendingObjects.emplace_back(std::move(object));
}

void Scene::Remove(GameObject* object)
{
	if (object)
	{
		object->MarkForDestroy();
	}
}

void Scene::RemoveAll()
{
	m_objects.clear();
}

void Scene::RequestLevelCleanup()
{
	m_levelNeedsCleanup = true;
}

void Scene::Update(float deltaTime)
{
	for (auto& object : m_objects)
	{
		if (object && !object->IsMarkedForDestroy())
		{
			object->Update(deltaTime);
		}
	}

	// waiting room
	for (auto& pending : m_pendingObjects)
	{
		m_objects.emplace_back(std::move(pending));
	}
	m_pendingObjects.clear();

	// nothing was working and it was either trhowing an exception when switching scenes or when the player ate a diamond - so this is the solution - clean up the scene in the end of update
	if (m_levelNeedsCleanup)
	{
		CleanUpScene();
		m_levelNeedsCleanup = false;
	}
}

void Scene::CleanUpScene()
{
	m_objects.erase(std::remove_if(m_objects.begin(), m_objects.end(),
		[](const std::unique_ptr<GameObject>& obj)
		{
			return obj->IsMarkedForDestroy();
		}), m_objects.end());
}

void Scene::Render() const
{
	std::vector<GameObject*> sortedObjects;
	for (const auto& object : m_objects)
	{
		if (object && !object->IsMarkedForDestroy())
		{
			sortedObjects.push_back(object.get());
		}
	}

	std::stable_sort(sortedObjects.begin(), sortedObjects.end(), [](GameObject* a, GameObject* b)
		{
			return a->GetZIndex() < b->GetZIndex();
		});

	for (auto* object : sortedObjects)
	{
		object->Render();
	}
}