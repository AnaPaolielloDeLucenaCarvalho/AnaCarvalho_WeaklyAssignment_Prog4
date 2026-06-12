#ifndef GOLDBAGSTATE_H
#define GOLDBAGSTATE_H
#include <memory>

// DESIGN PATTERN - State Pattern (Finite State Machine)
// Used the State pattern here because the Gold Bag has distinct phases (Idle, Wobbling, Falling, Broken). Instead of a massive list of if/else booleans, the logic is separated so the bag can only execute one behavior at a time.

namespace dae
{
    class GoldBagComponent;

    class GoldBagState
    {
    public:
        virtual ~GoldBagState() = default;

        virtual void OnEnter(GoldBagComponent* /*bag*/) {}
        virtual void OnExit(GoldBagComponent* /*bag*/) {}
        virtual std::unique_ptr<dae::GoldBagState> Update(GoldBagComponent* bag, float deltaTime) = 0;

        GoldBagState(const GoldBagState& other) = delete;
        GoldBagState(GoldBagState&& other) = delete;
        GoldBagState& operator=(const GoldBagState& other) = delete;
        GoldBagState& operator=(GoldBagState&& other) = delete;

    protected:
        GoldBagState() = default;
    };

    class GoldBagIdleState final : public GoldBagState
    {
    public:
        std::unique_ptr<dae::GoldBagState> Update(GoldBagComponent* bag, float deltaTime) override;
    };

    class GoldBagWobbleState final : public GoldBagState
    {
    public:
        void OnEnter(GoldBagComponent* bag) override;
        std::unique_ptr<dae::GoldBagState> Update(GoldBagComponent* bag, float deltaTime) override;
    private:
        float m_wobbleTimer{ 2.0f };
        float m_toggleTimer{ 0.0f };
        bool m_isLeft{ true };
    };

    class GoldBagFallingState final : public GoldBagState
    {
    public:
        void OnEnter(GoldBagComponent* bag) override;
        std::unique_ptr<dae::GoldBagState> Update(GoldBagComponent* bag, float deltaTime) override;
    private:
        float m_startY{ 0.f };
    };

    class GoldBagBrokenState final : public GoldBagState
    {
    public:
        void OnEnter(GoldBagComponent* bag) override;
        std::unique_ptr<dae::GoldBagState> Update(GoldBagComponent* bag, float deltaTime) override;
    private:
        float m_despawnTimer{ 5.0f };
        float m_animTimer{ 0.0f };
        int m_goldFrame{ 1 };
    };
}
#endif