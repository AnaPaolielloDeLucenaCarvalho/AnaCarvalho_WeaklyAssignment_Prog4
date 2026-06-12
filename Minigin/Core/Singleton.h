#pragma once

// DESIGN PATTERN - Singleton Pattern
// A simple template to turn any manager class into a Singleton. I use this for things like the InputManager and SceneManager so they can be accessed globally without passing pointers.

namespace dae
{
	template <typename T>
	class Singleton
	{
	public:
		[[nodiscard]] static T& GetInstance()
		{
			static T instance{};
			return instance;
		}

		virtual ~Singleton() = default;
		Singleton(const Singleton& other) = delete;
		Singleton(Singleton&& other) = delete;
		Singleton& operator=(const Singleton& other) = delete;
		Singleton& operator=(Singleton&& other) = delete;

	protected:
		Singleton() = default;
	};
}