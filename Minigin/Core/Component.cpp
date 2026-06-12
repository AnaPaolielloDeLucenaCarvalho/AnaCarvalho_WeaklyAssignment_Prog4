#include "Component.h"

namespace dae
{
    Component::~Component() = default;

    Component::Component(GameObject* pOwner)
        : m_pOwner(pOwner)
    {
    }

    GameObject* Component::GetOwner() const 
    { 
        return m_pOwner; 
    }

    void Component::MarkForDestroy() 
    { 
        m_isMarkedForDestroy = true; 
    }

    bool Component::IsMarkedForDestroy() const 
    { 
        return m_isMarkedForDestroy; 
    }
}