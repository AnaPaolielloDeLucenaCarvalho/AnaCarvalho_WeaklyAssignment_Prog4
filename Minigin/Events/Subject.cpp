#include "Subject.h"
#include <algorithm>

namespace dae
{
    Subject::~Subject()
    {
        for (auto observer : m_observers)
        {
            observer->RemoveSubjectInternal(this);
        }
    }

    void Subject::AddObserver(Observer* observer)
    {
        m_observers.push_back(observer);
        observer->AddSubjectInternal(this);
    }

    void Subject::RemoveObserver(Observer* observer)
    {
        m_observers.erase(std::remove(m_observers.begin(), m_observers.end(), observer), m_observers.end());
        observer->RemoveSubjectInternal(this);
    }

    void Subject::RemoveObserverInternal(Observer* observer)
    {
        m_observers.erase(std::remove(m_observers.begin(), m_observers.end(), observer), m_observers.end());
    }

    void Subject::Notify(EventId eventId, int value)
    {
        // Iterate through every registered listener and forcefully call their OnNotify method. This is where the Pointer Chasing (Cache Miss) happens, but because events don't fire thousands of times per frame, the performance hit is totally negligible.
        for (const auto& observer : m_observers)
        {
            observer->OnNotify(eventId, value);
        }
    }
}