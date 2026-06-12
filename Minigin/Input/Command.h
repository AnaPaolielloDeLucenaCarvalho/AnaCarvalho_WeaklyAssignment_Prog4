#ifndef COMMAND_H
#define COMMAND_H


// DESIGN PATTERN - Command Pattern (Base)
// The abstract base class for all actions. By turning an action into an object, we can easily rebind keys or trigger actions from completely different sources (like UI vs keyboard).

namespace dae
{
    class Command
    {
    public:
        virtual ~Command() = default;
        virtual void Execute(float deltaTime) = 0;

		// rule of 5
        Command(const Command& other) = delete;
        Command(Command&& other) = delete;
        Command& operator=(const Command& other) = delete;
        Command& operator=(Command&& other) = delete;

    protected:
        Command() = default;
    };
}

#endif