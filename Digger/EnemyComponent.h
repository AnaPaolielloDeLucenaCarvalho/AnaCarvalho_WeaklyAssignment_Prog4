#ifndef ENEMY_COMPONENT_H
#define ENEMY_COMPONENT_H

#include "Component.h"
#include "GameObject.h"
#include <glm/vec2.hpp>
#include <memory>

namespace dae
{
    class DiggerComponent;
    class EnemyState;

    class EnemyComponent : public Component
    {
    public:
        EnemyComponent(GameObject* owner, DiggerComponent* p1, DiggerComponent* p2);
        ~EnemyComponent() override;

        void Update(float deltaTime) override;
        void ChangeState(EnemyState* newState);

        void MoveAI(float deltaTime);

        void SetCanDig(bool canDig) { m_CanDig = canDig; }
        bool CanDig() const { return m_CanDig; }

        DiggerComponent* GetTarget() const { return m_pTarget; }

    private:
        std::unique_ptr<EnemyState> m_pCurrentState{ nullptr };
        DiggerComponent* m_p1{ nullptr };
        DiggerComponent* m_p2{ nullptr };
        DiggerComponent* m_pTarget{ nullptr };

        glm::vec2 m_CurrentDirection{ 1, 0 };
        bool m_CanDig{ false };
        float m_AnimTimer{ 0.0f };
        int m_Frame{ 1 };
    };

    class EnemyState
    {
    public:
        virtual ~EnemyState() = default;
        virtual void OnEnter(EnemyComponent* /*enemy*/) {}
        virtual void OnExit(EnemyComponent* /*enemy*/) {}
        virtual EnemyState* Update(EnemyComponent* enemy, float deltaTime) = 0;
    };

    class NobbinState : public EnemyState
    {
    public:
        void OnEnter(EnemyComponent* enemy) override;
        EnemyState* Update(EnemyComponent* enemy, float deltaTime) override;
    private:
        float m_HobbinTimer{ 10.0f }; // turn into a hobbin after 10s
    };

    class HobbinState : public EnemyState
    {
    public:
        void OnEnter(EnemyComponent* enemy) override;
        EnemyState* Update(EnemyComponent* enemy, float deltaTime) override;
    };
}
#endif