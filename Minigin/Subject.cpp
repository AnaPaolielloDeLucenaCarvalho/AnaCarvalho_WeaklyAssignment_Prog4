#include "Subject.h"
#include <algorithm>

namespace dae
{
    void Subject::AddObserver(Observer* observer)
    {
        m_observers.push_back(observer);
    }

    void Subject::RemoveObserver(Observer* observer)
    {
        m_observers.erase(std::remove(m_observers.begin(), m_observers.end(), observer), m_observers.end());
    }

    void Subject::Notify(EventId eventId, int value)
    {
        for (auto observer : m_observers)
        {
            observer->OnNotify(eventId, value);
        }
    }
}