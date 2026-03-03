#ifndef GAMEOBJECT_H // No #pragma once, use include guards (for tom's sake)
#define GAMEOBJECT_H

#include <string>
#include <memory>
#include <vector>
#include "Transform.h"
#include "Component.h"

namespace dae
{
	class GameObject final
	{
        Transform m_worldTransform{};
        Transform m_localTransform{};

		std::vector<std::unique_ptr<Component>> m_Components;
		bool m_isMarkedForDestroy{ false };

        // W02
        GameObject* m_pParent{ nullptr };
        std::vector<GameObject*> m_pChildren{};
        bool m_positionIsDirty{ false };

        void UpdateWorldTransform();

        void RemoveChild(GameObject* child);
        void AddChild(GameObject* child);
	public:
		GameObject() = default;
		~GameObject();

		GameObject(const GameObject& other) = delete;
		GameObject(GameObject&& other) = delete;
		GameObject& operator=(const GameObject& other) = delete;
		GameObject& operator=(GameObject&& other) = delete;

        void Update(float deltaTime);
		void Render() const;

        void SetLocalPosition(float x, float y);
        const Transform& GetTransform();
        void SetPositionDirty();

        void SetParent(GameObject* parent, bool keepWorldPosition);
        GameObject* GetParent() const { return m_pParent; }
        size_t GetChildCount() const { return m_pChildren.size(); }
        GameObject* GetChildAt(unsigned int index) const { return m_pChildren[index]; }
        bool IsChild(GameObject* child) const;

        void MarkForDestroy() { m_isMarkedForDestroy = true; }
        bool IsMarkedForDestroy() const { return m_isMarkedForDestroy; }

        // --- COMPONENT SYSTEM ---

        // Add a component to a game object
        template <typename T, typename... Args>
        T* AddComponent(Args&&... args)
        {
            static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
            auto component = std::make_unique<T>(this, std::forward<Args>(args)...);
            T* rawPtr = component.get();
            m_Components.push_back(std::move(component));
            return rawPtr;
        }

        // Get a component from a game object
        template <typename T>
        T* GetComponent() const
        {
            for (const auto& component : m_Components)
            {
                if (T* castedComponent = dynamic_cast<T*>(component.get()))
                {
                    return castedComponent;
                }
            }
            return nullptr;
        }

        // Check whether a component has been added
        template <typename T>
        bool HasComponent() const { return GetComponent<T>() != nullptr; }

        // Remove a component from a game object in a safe manner
        template <typename T>
        void RemoveComponent()
        {
            for (const auto& component : m_Components)
            {
                if (dynamic_cast<T*>(component.get()) != nullptr)
                {
                    component->MarkForDestroy();
                }
            }
        }
	};
}

#endif // GAMEOBJECT_H