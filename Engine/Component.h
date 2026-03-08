#ifndef COMPONENT_H // No #pragma once, use include guards (for tom's sake)
#define COMPONENT_H

namespace dae
{
    class GameObject;

    class Component
    {
    public:
        virtual ~Component() = default; // Avoid leaks

        Component(const Component& other) = delete;
        Component(Component&& other) = delete;
        Component& operator=(const Component& other) = delete;
        Component& operator=(Component&& other) = delete;

		virtual void Update(float deltaTime) = 0; // Unreal method, the derived classes have to implement deltaTime
        virtual void Render() const {}

        GameObject* GetOwner() const { return m_pOwner; } // Component has access to owner

		// We don't want to delete it immediately, so we mark it for destruction
        void MarkForDestroy() { m_isMarkedForDestroy = true; }
        bool IsMarkedForDestroy() const { return m_isMarkedForDestroy; }

        // Think about this: should it ever be possible for a component to exist without a gameobject? no
        Component(GameObject* pOwner) : m_pOwner(pOwner) {}

    private:
        GameObject* m_pOwner;
        bool m_isMarkedForDestroy{ false };
    };
}

#endif // COMPONENT_H