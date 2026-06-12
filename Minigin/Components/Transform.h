#ifndef TRANSFORM_H
#define TRANSFORM_H
#include <glm/glm.hpp>


// DESIGN PATTERN - Data Component
// I separated position, rotation, and scale into their own little struct. Every GameObject gets one of these automatically.

namespace dae
{
	class Transform final
	{
	public:
		const glm::vec3& GetPosition() const;
		void SetPosition(float x, float y, float z = 0);
		void SetPosition(const glm::vec3& position);
	private:
		glm::vec3 m_position;
	};
}
#endif
