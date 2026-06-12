#ifndef COMPONENT_H
#define COMPONENT_H


// DESIGN PATTERN - Component Pattern (Base Class)
// This is the abstract base class that all behaviors inherit from. By forcing everything to have an Update() and Render() method, the GameObject can just loop through its components blindly.

namespace dae
{
    class GameObject;

    class Component
    {
    public:
        virtual ~Component();

        Component(const Component& other) = delete;
        Component(Component&& other) = delete;
        Component& operator=(const Component& other) = delete;
        Component& operator=(Component&& other) = delete;

        virtual void Update(float deltaTime) = 0;
        virtual void Render() const {}

        GameObject* GetOwner() const { return m_pOwner; }

        void MarkForDestroy() { m_isMarkedForDestroy = true; }
        bool IsMarkedForDestroy() const { return m_isMarkedForDestroy; }

        Component(GameObject* pOwner);

    private:
        GameObject* m_pOwner;
        bool m_isMarkedForDestroy{ false };
    };
}
#endif