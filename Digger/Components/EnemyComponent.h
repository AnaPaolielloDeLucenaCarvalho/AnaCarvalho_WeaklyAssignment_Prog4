#ifndef ENEMY_COMPONENT_H
#define ENEMY_COMPONENT_H

#include "Component.h"
#include "GameObject.h"
#include <glm/vec2.hpp>
#include <memory>

// DESIGN PATTERN - State Pattern
// The Enemy AI uses the State pattern to seamlessly switch from a Nobbin (follows the path, can't dig)  to a Hobbin (eats straight through the dirt walls). Instead of checking timers and throwing messy  if/else statements in the Update loop, the component just delegates to the active State class.

namespace dae
{
    class DiggerComponent;
    class EnemyState;

    class EnemyComponent final : public Component
    {
    public:
        EnemyComponent(GameObject* owner, DiggerComponent* p1, DiggerComponent* p2);
        ~EnemyComponent() override;

        EnemyComponent(const EnemyComponent&) = delete;
        EnemyComponent& operator=(const EnemyComponent&) = delete;
        EnemyComponent(EnemyComponent&&) = delete;
        EnemyComponent& operator=(EnemyComponent&&) = delete;

        void Render() const override {}
        void Update(float deltaTime) override;
        void ChangeState(std::unique_ptr<EnemyState> newState);

        void MoveAI(float deltaTime);

        void SetCanDig(bool canDig) 
        { 
            m_canDig = canDig; 
        }
        bool CanDig() const 
        { 
            return m_canDig; 
        }

        DiggerComponent* GetTarget() const 
        { 
            return m_pTarget; 
        }

    private:
        std::unique_ptr<EnemyState> m_pCurrentState{ nullptr };
        DiggerComponent* m_p1{ nullptr };
        DiggerComponent* m_p2{ nullptr };
        DiggerComponent* m_pTarget{ nullptr };

        glm::vec2 m_currentDirection{ 1, 0 };
        bool m_canDig{ false };
        float m_animTimer{ 0.0f };
        int m_frame{ 1 };
    };

    class EnemyState
    {
    public:
        virtual ~EnemyState() = default;

        EnemyState() = default;
        EnemyState(const EnemyState& other) = delete;
        EnemyState(EnemyState&& other) = delete;
        EnemyState& operator=(const EnemyState& other) = delete;
        EnemyState& operator=(EnemyState&& other) = delete;
        virtual void OnEnter(EnemyComponent* /*enemy*/) {}
        virtual void OnExit(EnemyComponent* /*enemy*/) {}
        virtual std::unique_ptr<EnemyState> Update(EnemyComponent* enemy, float deltaTime) = 0;
    };

    class NobbinState final : public EnemyState
    {
    public:
        void OnEnter(EnemyComponent* enemy) override;
        std::unique_ptr<EnemyState> Update(EnemyComponent* enemy, float deltaTime) override;
    private:
        float m_hobbinTimer{ 10.0f }; // turn into a hobbin after 10s
    };

    class HobbinState final : public EnemyState
    {
    public:
        void OnEnter(EnemyComponent* enemy) override;
        std::unique_ptr<EnemyState> Update(EnemyComponent* enemy, float deltaTime) override;
    };
}
#endif