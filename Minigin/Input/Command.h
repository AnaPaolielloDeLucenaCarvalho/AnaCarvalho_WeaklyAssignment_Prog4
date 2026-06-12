#ifndef COMMAND_H
#define COMMAND_H

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