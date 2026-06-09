#ifndef DIGGERSTATE_H
#define DIGGERSTATE_H

namespace dae
{
    class DiggerComponent;

    class DiggerState
    {
    public:
        virtual ~DiggerState() = default;
        virtual void OnEnter(DiggerComponent* /*digger*/) {}
        virtual void OnExit(DiggerComponent* /*digger*/) {}
        virtual DiggerState* Update(DiggerComponent* digger, float deltaTime) = 0;

        DiggerState(const DiggerState& other) = delete;
        DiggerState(DiggerState&& other) = delete;
        DiggerState& operator=(const DiggerState& other) = delete;
        DiggerState& operator=(DiggerState&& other) = delete;

    protected:
        DiggerState() = default;
    };

    class DiggerNormalState : public DiggerState
    {
    public:
        void OnEnter(DiggerComponent* digger) override;
        DiggerState* Update(DiggerComponent* digger, float deltaTime) override;
    };

    class DiggerBonusState : public DiggerState
    {
    public:
        void OnEnter(DiggerComponent* digger) override;
        void OnExit(DiggerComponent* digger) override;
        DiggerState* Update(DiggerComponent* digger, float deltaTime) override;
    private:
        float m_BonusTimer{ 15.0f }; // 15 seconds of bonus mode
    };

    class DiggerDeadState : public DiggerState
    {
    public:
        void OnEnter(DiggerComponent* digger) override;
        DiggerState* Update(DiggerComponent* digger, float deltaTime) override;
    private:
        float m_RespawnTimer{ 2.0f };
        float m_AnimTimer{ 0.0f };
        int m_CurrentFrame{ 1 };
    };

    class DiggerGameOverState : public DiggerState
    {
    public:
        void OnEnter(DiggerComponent* digger) override;
        DiggerState* Update(DiggerComponent* digger, float deltaTime) override;
    };

    class DiggerLevelCompleteState : public DiggerState
    {
    public:
        void OnEnter(DiggerComponent* digger) override;
        DiggerState* Update(DiggerComponent* digger, float deltaTime) override;
    private:
        float m_TransitionTimer{ 4.0f };
    };
}
#endif