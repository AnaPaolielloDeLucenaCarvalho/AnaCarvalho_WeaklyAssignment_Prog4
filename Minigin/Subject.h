#ifndef SUBJECT_H
#define SUBJECT_H

#include <vector>
#include <algorithm>
#include "Observer.h"

namespace dae
{
    class Subject
    {
    public:
        virtual ~Subject() = default;
        void AddObserver(Observer* observer) { m_observers.push_back(observer); }
        void RemoveObserver(Observer* observer)
        {
            m_observers.erase(std::remove(m_observers.begin(), m_observers.end(), observer), m_observers.end());
        }

    protected:
        void Notify(Event event, int value)
        {
            for (auto observer : m_observers) 
            { 
                observer->OnNotify(event, value); 
            }
        }

    private:
        std::vector<Observer*> m_observers;
    };
}
#endif