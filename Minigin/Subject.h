#ifndef SUBJECT_H
#define SUBJECT_H

#include <vector>
#include "Observer.h"

namespace dae
{
    class Subject
    {
    public:
        virtual ~Subject() = default;

        void AddObserver(Observer* observer);
        void RemoveObserver(Observer* observer);
        void Notify(EventId eventId, int value);

    private:
        std::vector<Observer*> m_observers;
    };
}
#endif