#ifndef DIGGERSTATE_H
#define DIGGERSTATE_H
#include <memory>

// DESIGN PATTERN - State Pattern (Finite State Machine)
// Used the State pattern to decouple the player's behaviors. Instead of the main Update loop checking things every single frame, Digger just delegates its logic to the active state class. It makes transitioning between gameplay phases incredibly safe and predictable.

namespace dae
{
    class DiggerComponent;

    class DiggerState
    {
    public:
        virtual ~DiggerState() = default;
        virtual void OnEnter(DiggerComponent* /*digger*/) {}
        virtual void OnExit(DiggerComponent* /*digger*/) {}
        virtual std::unique_ptr<dae::DiggerState> Update(DiggerComponent* digger, float deltaTime) = 0;

        DiggerState(const DiggerState& other) = delete;
        DiggerState(DiggerState&& other) = delete;
        DiggerState& operator=(const DiggerState& other) = delete;
        DiggerState& operator=(DiggerState&& other) = delete;

    protected:
        DiggerState() = default;
    };

    class DiggerNormalState final : public DiggerState
    {
    public:
        void OnEnter(DiggerComponent* digger) override;
        std::unique_ptr<dae::DiggerState> Update(DiggerComponent* digger, float deltaTime) override;
    };

    class DiggerBonusState final : public DiggerState
    {
    public:
        void OnEnter(DiggerComponent* digger) override;
        void OnExit(DiggerComponent* digger) override;
        std::unique_ptr<dae::DiggerState> Update(DiggerComponent* digger, float deltaTime) override;
    private:
        float m_bonusTimer{ 15.0f }; // 15s of bonus mode
    };

    class DiggerDeadState final : public DiggerState
    {
    public:
        void OnEnter(DiggerComponent* digger) override;
        std::unique_ptr<dae::DiggerState> Update(DiggerComponent* digger, float deltaTime) override;
    private:
        float m_respawnTimer{ 2.0f };
        float m_animTimer{ 0.0f };
        int m_currentFrame{ 1 };
    };

    class DiggerGameOverState final : public DiggerState
    {
    public:
        void OnEnter(DiggerComponent* digger) override;
        std::unique_ptr<dae::DiggerState> Update(DiggerComponent* digger, float deltaTime) override;
    private:
        float m_timer{ 0.0f };
    };

    class DiggerLevelCompleteState final : public DiggerState
    {
    public:
        void OnEnter(DiggerComponent* digger) override;
        std::unique_ptr<dae::DiggerState> Update(DiggerComponent* digger, float deltaTime) override;
    private:
        float m_transitionTimer{ 4.0f };
    };
}
#endif