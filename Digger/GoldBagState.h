#ifndef GOLDBAGSTATE_H
#define GOLDBAGSTATE_H

namespace dae
{
    class GoldBagComponent;

    class GoldBagState
    {
    public:
        virtual ~GoldBagState() = default;

        virtual void OnEnter(GoldBagComponent* /*bag*/) {}
        virtual void OnExit(GoldBagComponent* /*bag*/) {}
        virtual GoldBagState* Update(GoldBagComponent* bag, float deltaTime) = 0;
    };

    class GoldBagIdleState : public GoldBagState
    {
    public:
        GoldBagState* Update(GoldBagComponent* bag, float deltaTime) override;
    };

    class GoldBagWobbleState : public GoldBagState
    {
    public:
        void OnEnter(GoldBagComponent* bag) override;
        GoldBagState* Update(GoldBagComponent* bag, float deltaTime) override;
    private:
        float m_WobbleTimer{ 2.0f };
        float m_ToggleTimer{ 0.0f };
        bool m_IsLeft{ true };
    };

    class GoldBagFallingState : public GoldBagState
    {
    public:
        void OnEnter(GoldBagComponent* bag) override;
        GoldBagState* Update(GoldBagComponent* bag, float deltaTime) override;
    private:
        float m_StartY{ 0.f };
    };

    class GoldBagBrokenState : public GoldBagState
    {
    public:
        void OnEnter(GoldBagComponent* bag) override;
        GoldBagState* Update(GoldBagComponent* bag, float deltaTime) override;
    private:
        float m_DespawnTimer{ 5.0f };
        float m_AnimTimer{ 0.0f };
        int m_GoldFrame{ 1 };
    };
}
#endif