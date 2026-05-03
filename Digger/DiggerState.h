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
        float m_BonusTimer{ 5.0f };
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
        void OnEnter(DiggerComponent* /*digger*/) override {}
        DiggerState* Update(DiggerComponent* /*digger*/, float /*deltaTime*/) override { return nullptr; }
    };
}
#endif