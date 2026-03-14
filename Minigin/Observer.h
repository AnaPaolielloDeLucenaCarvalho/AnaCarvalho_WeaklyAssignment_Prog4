#ifndef OBSERVER_H
#define OBSERVER_H

namespace dae
{
    enum class Event
    {
        PlayerDied,
        DiamondPickedUp,
        EnemyKilled,
        GoalReached
    };

    class Observer
    {
    public:
        virtual ~Observer() = default;
        virtual void OnNotify(Event event, int value = 0) = 0;
    };
}
#endif