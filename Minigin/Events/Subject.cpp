#include "Subject.h"
#include <algorithm>

namespace dae
{
    void Subject::AddObserver(Observer* observer)
    {
        // Simply append the new listener to our internal vector list
        m_observers.push_back(observer);
    }

    void Subject::RemoveObserver(Observer* observer)
    {
        // DESIGN DECISION - Erase-Remove Idiom
        // Using standard algorithms is safer than writing a manual for-loop.  std::remove shifts all valid pointers to the front, and erase chops off the garbage at the end.
        m_observers.erase(std::remove(m_observers.begin(), m_observers.end(), observer), m_observers.end());
    }

    void Subject::Notify(EventId eventId, int value)
    {
        // Iterate through every registered listener and forcefully call their OnNotify method. This is where the Pointer Chasing (Cache Miss) happens, but because events don't fire thousands of times per frame, the performance hit is totally negligible.
        for (auto observer : m_observers)
        {
            observer->OnNotify(eventId, value);
        }
    }
}