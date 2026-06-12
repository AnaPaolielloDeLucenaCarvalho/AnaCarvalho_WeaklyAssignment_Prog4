#ifndef ObjectPoolComponent_H
#define ObjectPoolComponent_H

#include "Component.h"
#include <vector>

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