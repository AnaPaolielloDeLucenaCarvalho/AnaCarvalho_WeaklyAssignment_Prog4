#ifndef ROTATORCOMPONENT_H // No #pragma once, use include guards (for tom's sake)
#define ROTATORCOMPONENT_H

#include "Component.h"
#include "GameObject.h"
#include <cmath> // cos and sin

namespace dae
{
	class RotatorComponent final : public Component
	{
	public:
		RotatorComponent(GameObject* pOwner, float radius, float speed);

		void Update(float deltaTime) override;

	private:
		float m_radius{};
		float m_speed{};
		float m_angle{ 0.0f };
	};
}

#endif // ROTATORCOMPONENT_H