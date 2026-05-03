#include "DiggerComponent.h"
#include "DiggerState.h"
#include "ServiceLocator.h"
#include <iostream>

namespace dae
{
    DiggerComponent::DiggerComponent(GameObject* owner) : Component(owner)
    {
        m_pCurrentState = new DiggerNormalState();
        m_pCurrentState->OnEnter(this);
    }

    DiggerComponent::~DiggerComponent()
    {
        if (m_pCurrentState)
        {
            m_pCurrentState->OnExit(this);
            delete m_pCurrentState;
        }
    }

    void DiggerComponent::Update(float deltaTime)
    {
        if (m_pCurrentState)
        {
            DiggerState* newState = m_pCurrentState->Update(this, deltaTime);
            if (newState != nullptr)
            {
                ChangeState(newState);
            }
        }

        m_DesiredDirection = { 0, 0 };
    }

    void DiggerComponent::ChangeState(DiggerState* newState)
    {
        if (m_pCurrentState)
        {
            m_pCurrentState->OnExit(this);
            delete m_pCurrentState;
        }

        m_pCurrentState = newState;

        if (m_pCurrentState)
        {
            m_pCurrentState->OnEnter(this);
        }
    }

    void DiggerComponent::Die()
    {
        if (m_Lives > 0)
        {
            m_Lives--;
            m_Subject.Notify(make_sdbm_hash("PlayerDied"), m_Lives);
            ServiceLocator::get_sound_system().play(3, 1.0f);
        }

        if (m_Lives <= 0)
        {
            std::cout << "GAME OVER! Going back to Main Menu...\n";
        }
    }
}