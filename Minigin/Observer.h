#ifndef OBSERVER_H
#define OBSERVER_H

#include <cstdint>
#include <cstddef>

namespace dae
{
	// feedback - hashing instead of enum
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

    class Observer
    {
    public:
        virtual ~Observer() = default;
        virtual void OnNotify(EventId eventId, int value = 0) = 0;
    };
}
#endif