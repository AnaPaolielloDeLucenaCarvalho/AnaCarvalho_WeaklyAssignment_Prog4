#ifndef MINIGIN_H
#define MINIGIN_H

#include <string>
#include <functional>
#include <filesystem>
#include <memory>

struct SDL_Window;


// DESIGN PATTERN - Facade Pattern (Engine Entry)
// I wrapped all the complex initialization for SDL, OpenGL, and ImGui into a single clean interface here. It hides the messy setup so the main game loop is easy to read.

namespace dae
{
	class Minigin final
	{
		bool m_quit{};
		std::unique_ptr<SDL_Window, void(*)(SDL_Window*)> m_window{nullptr, nullptr};
	public:
		explicit Minigin(const std::filesystem::path& dataPath);
		~Minigin();
		void Run(const std::function<void()>& load);
		void RunOneFrame();

		Minigin(const Minigin& other) = delete;
		Minigin(Minigin&& other) = delete;
		Minigin& operator=(const Minigin& other) = delete;
		Minigin& operator=(Minigin&& other) = delete;
	};
}
#endif