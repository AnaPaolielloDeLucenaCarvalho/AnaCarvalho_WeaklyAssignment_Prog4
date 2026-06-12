#include "Component.h"

namespace dae
{
    Component::~Component() = default;

    Component::Component(GameObject* pOwner)
        : m_pOwner(pOwner)
    {
    }
}