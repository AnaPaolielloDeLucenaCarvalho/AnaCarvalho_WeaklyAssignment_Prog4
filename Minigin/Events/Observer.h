#ifndef OBSERVER_H
#define OBSERVER_H

#include <cstdint>
#include <cstddef>
#include <vector>

// DESIGN PATTERN - Observer Pattern
// I used the Observer pattern here to completely decouple gameplay systems. Instead of the Player  needing an #include reference to the UI or Audio system, it just blindly broadcasts an EventId. Any system that cares can inherit from this Observer class and listen in!

namespace dae
{
    // DESIGN DECISION - Compile-Time String Hashing
    // Passing raw strings (like "PlayerDied") around the engine every frame is incredibly slow and  causes memory bloat. Instead, this constexpr sdbm_hash converts string names into integer IDs at compile time. It is infinitely faster for the CPU to compare two integers than two strings!
    template <int length> struct sdbm_hash
    {
        consteval static unsigned int _calculate(const char* const text, unsigned int& value) 
        {
            const unsigned int character = sdbm_hash<length - 1>::_calculate(text, value);
            value = character + (value << 6) + (value << 16) - value;
            return text[length - 1];
        }
        consteval static unsigned int calculate(const char* const text) 
        {
            unsigned int value = 0;
            const auto character = _calculate(text, value);
            return character + (value << 6) + (value << 16) - value;
        }
    };

    template <> struct sdbm_hash<1> 
    {
        consteval static int _calculate(const char* const text, unsigned int&)
        { 
            return text[0]; 
        }
    };

    template <size_t N> consteval unsigned int make_sdbm_hash(const char(&text)[N]) 
    {
        return sdbm_hash<N - 1>::calculate(text);
    }

    using EventId = unsigned int;

    class Subject; // Forward declaration

    class Observer
    {
    public:
        virtual ~Observer();
        virtual void OnNotify(EventId eventId, int value = 0) = 0;

        // Internal methods used by Subject for bidirectional tracking
        void AddSubjectInternal(Subject* subject);
        void RemoveSubjectInternal(Subject* subject);

        Observer(const Observer& other) = delete;
        Observer(Observer&& other) = delete;
        Observer& operator=(const Observer& other) = delete;
        Observer& operator=(Observer&& other) = delete;

    protected:
        Observer() = default;

    private:
        std::vector<Subject*> m_subjects;
    };
}
#endif