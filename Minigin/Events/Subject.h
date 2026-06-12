#ifndef SUBJECT_H
#define SUBJECT_H

#include <vector>
#include "Observer.h"

// DESIGN PATTERN - Observer Pattern (The Broadcaster)
// The Subject acts as a radio tower. Any component (like Digger) can own a Subject and use it  to broadcast events. It keeps a list of listeners but doesn't actually know or care what they are.

namespace dae
{
    class Subject
    {
    public:
        virtual ~Subject() = default;

        Subject() = default;
        Subject(const Subject& other) = delete;
        Subject(Subject&& other) = delete;
        Subject& operator=(const Subject& other) = delete;
        Subject& operator=(Subject&& other) = delete;

        void AddObserver(Observer* observer);
        void RemoveObserver(Observer* observer);
        void Notify(EventId eventId, int value);

    private:
        // DESIGN DECISION - Pointer Chasing Hazard
        // We have to store pointers here because Observers are polymorphic. I know from my Theory notes that iterating through a vector of pointers causes Cache Misses because the CPU has to "drive across town" (RAM) to fetch each object, but it is a necessary trade-off for the extreme flexibility of the Observer pattern.
        std::vector<Observer*> m_observers;
    };
}
#endif