#ifndef GOLD_BAG_COMPONENT_H
#define GOLD_BAG_COMPONENT_H

#include "Component.h"
#include "GameObject.h"

namespace dae
{
    class GoldBagState;

    class GoldBagComponent : public dae::Component
    {
    public:
        GoldBagComponent(dae::GameObject* owner);
        ~GoldBagComponent() override;

        void Update(float deltaTime) override;

        bool IsDirtDirectlyUnderneath() const;
        bool HitBottom() const;
        void SmashEntitiesBelow();

        void SetPlayers(GameObject* p1, GameObject* p2) { m_pPlayer1 = p1; m_pPlayer2 = p2; }

        bool IsBroken() const { return m_IsBroken; }
        void SetBroken(bool broken) { m_IsBroken = broken; }

    private:
        void ChangeState(GoldBagState* newState);

        GoldBagState* m_pCurrentState{ nullptr };

        GameObject* m_pPlayer1{ nullptr };
        GameObject* m_pPlayer2{ nullptr };

		bool m_IsBroken{ false };
    };
}
#endif