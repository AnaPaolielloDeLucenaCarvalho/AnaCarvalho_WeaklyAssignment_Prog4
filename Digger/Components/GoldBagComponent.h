#ifndef GOLD_BAG_COMPONENT_H
#define GOLD_BAG_COMPONENT_H

#include "Component.h"
#include "GameObject.h"
#include <memory>

// DESIGN PATTERN - Component Pattern
// This component acts as the main physical shell for the Gold Bag entity. It stores the player references and physical collision logic, while delegating all its behavioural logic (wobbling, falling, breaking) down into its internal State Machine.

namespace dae
{
    class GoldBagState;

    class GoldBagComponent final : public dae::Component
    {
    public:
        explicit GoldBagComponent(dae::GameObject* owner);
        ~GoldBagComponent() override;

        GoldBagComponent(const GoldBagComponent&) = delete;
        GoldBagComponent& operator=(GoldBagComponent&&) = delete;

        void Render() const override {}
        void Update(float deltaTime) override;

        bool IsDirtDirectlyUnderneath() const;
        bool HitBottom() const;
        void SmashEntitiesBelow();

        void SetPlayers(GameObject* p1, GameObject* p2) 
        { 
            m_pPlayer1 = p1; 
            m_pPlayer2 = p2; 
        }

        bool IsBroken() const 
        { 
            return m_isBroken; 
        }
        void SetBroken(bool broken) 
        { 
            m_isBroken = broken; 
        }

    private:
        void ChangeState(std::unique_ptr<dae::GoldBagState> newState);

        std::unique_ptr<GoldBagState> m_pCurrentState{ nullptr };

        GameObject* m_pPlayer1{ nullptr };
        GameObject* m_pPlayer2{ nullptr };

        bool m_isBroken{ false };
    };
}
#endif