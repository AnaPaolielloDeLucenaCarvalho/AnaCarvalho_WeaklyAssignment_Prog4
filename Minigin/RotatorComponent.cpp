#include "RotatorComponent.h"

dae::RotatorComponent::RotatorComponent(GameObject* pOwner, float radius, float speed)
	: Component(pOwner)
	, m_radius(radius)
	, m_speed(speed)
{
}

void dae::RotatorComponent::Update(float deltaTime)
{
    m_angle += m_speed * deltaTime;

    // Prevent overflow
    if (m_angle > 2.0f * 3.14159f) m_angle -= 2.0f * 3.14159f;

    float x = m_radius * std::cos(m_angle);
    float y = m_radius * std::sin(m_angle);

    GetOwner()->SetLocalPosition(x, y);
}