#ifndef ObjectPoolComponent_H
#define ObjectPoolComponent_H

#include "Component.h"
#include <vector>

// DESIGN PATTERN - Component Pattern
// By inheriting from Component, this profiling tool can be snapped onto any empty GameObject case. It keeps the testing logic completely isolated from the core engine.

namespace dae
{
    class ObjectPoolComponent : public Component
    {
    public:
        ObjectPoolComponent(GameObject* pOwner);
        ~ObjectPoolComponent() override = default;

        void Update(float deltaTime) override;
        void Render() const override;

    private:
        mutable int m_Samples = 25;

        // DESIGN DECISION - Hot vs Cold Data
        // Storing the timings in contiguous vectors ensures fast iterations when plotting the ImGui graphs.
        mutable std::vector<float> m_TimingsInt;
        mutable std::vector<float> m_TimingsGO3D;
        mutable std::vector<float> m_TimingsGO3DAlt;

        std::vector<float> m_XData;

        void RunExercise1() const;
        void RunExercise2() const;
        void RunExercise2Alt() const;
    };
}
#endif