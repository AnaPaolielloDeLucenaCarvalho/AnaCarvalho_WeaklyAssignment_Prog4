#ifndef ObjectPoolComponent_H
#define ObjectPoolComponent_H

#include "Component.h"
#include <vector>

// DESIGN PATTERN - Component Pattern
// By inheriting from Component, this profiling tool can be snapped onto any empty GameObject case. It keeps the testing logic completely isolated from the core engine.

namespace dae
{
    class ObjectPoolComponent final : public Component
    {
    public:
        explicit ObjectPoolComponent(GameObject* pOwner);
        ~ObjectPoolComponent() override = default;

        void Update(float deltaTime) override;
        void Render() const override;

    private:
        mutable int m_samples = 25;

        // DESIGN DECISION - Hot vs Cold Data
        // Storing the timings in contiguous vectors ensures fast iterations when plotting the ImGui graphs.
        mutable std::vector<float> m_timingsInt;
        mutable std::vector<float> m_timingsGO3D;
        mutable std::vector<float> m_timingsGO3DAlt;

        std::vector<float> m_xData;

        void RunExercise1() const;
        void RunExercise2() const;
        void RunExercise2Alt() const;
    };
}
#endif