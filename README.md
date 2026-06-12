# Digger Game - Minigin Engine

This project aims to recreate the 1983 arcade game **Digger** from scratch using a custom C++ game engine. The game features full support for both Keyboard and Gamepads across all game modes, menus, and UI screens. It was developed as my final exam project for the **Programming 4** course at DAE (Howest).

## Links
* **GitHub Pages Build (Web):** [Play Digger on the WEB Here](https://anapaoliellodelucenacarvalho.github.io/AnaCarvalho_WeaklyAssignment_Prog4/)

---

## Game Features
* **Multiple Modes:**
  * **Single Player:** Standard single-player arcade mode
  * **Co-Op:** Two players control Diggers and work together to clear the level
  * **Versus:** Player 1 controls Digger, Player 2 controls the enemy (Nobbin/Hobbin) trying to catch them
* **Persistent High Scores:** Arcade-style leaderboard that saves scores locally.
* **3 Custom Levels:** Loaded from data files.
* **Audio & Music:** Has the original music and sound effects (from the internet)

---

## Controls
The game is fully navigable with either a Keyboard or an Xbox-compatible Controller. Player 1 is permanently mapped to Gamepad 0, and Player 2 is permanently mapped to Gamepad 1.

### Menus & UI (Main Menu, Game Over, Name Entry)
| Action | Keyboard | All Gamepads |
| :--- | :--- | :--- |
| **Navigate Up** | `W` / `Up Arrow` | D-Pad Up |
| **Navigate Down** | `S` / `Down Arrow` | D-Pad Down |
| **Next Letter** (Name) | `D` / `Right Arrow` | D-Pad Right |
| **Previus Letter** (Name) | `A` / `Left Arrow` | D-Pad Left |
| **Select / Confirm** | `Space` / `Enter` | Left Shoulder / Right Shoulder |

### Gameplay (Player 1)
| Action | Keyboard | Gamepad 0 |
| :--- | :--- | :--- |
| **Move Up** | `W` | D-Pad Up |
| **Move Down** | `S` | D-Pad Down |
| **Move Left** | `A` | D-Pad Left |
| **Move Right** | `D` | D-Pad Right |
| **Shoot** | `Space` | Left Trigger / Right Trigger |

### Gameplay (Player 2)
| Action | Keyboard | Gamepad 1 |
| :--- | :--- | :--- |
| **Move Up** | `I` | D-Pad Up |
| **Move Down** | `K` | D-Pad Down |
| **Move Left** | `J` | D-Pad Left |
| **Move Right** | `L` | D-Pad Right  |
| **Shoot** | `Right Shift` | Left Trigger / Right Trigger |

### System Commands (Global Hotkeys)
| Action | Keyboard | Gamepad 0 / 1 |
| :--- | :--- | :--- |
| **Skip Level** | `F1` | Button B |
| **Mute Audio** | `F2` | Button Y |
| **Instructions** | `F3` | Button X |
| **Main Menu** | `F10` | Button A |

---

## Engine Architecture
This game runs on **Minigin**. The architecture heavily applies principles from Robert Nystrom's *Game Programming Patterns* and Gregory Davidson's *Beautiful C++*.

### Key Design Choices
* **Entity-Component-System (ECS):** The game revolves around `GameObject`s holding custom `Component`s via `std::unique_ptr` for strict memory ownership. Raw pointers are strictly avoided for ownership (RAII).
* **Observer Pattern (Events):** Gameplay systems are entirely decoupled using a Bidirectional Observer Pattern. The game logic broadcasts an `EventId` (using compile-time string hashing via `make_sdbm_hash` for maximum performance). If a UI element is destroyed mid-game, its destructor safely unregisters itself to prevent dangling pointers.
* **State Pattern:** Complex object behaviors rely on State Machines to prevent spaghetti code. The Enemy AI swaps cleanly between `NobbinState` and `HobbinState`, and the `GoldBagComponent` manages falling/breaking physics via dedicated states.
* **Command Pattern:** Input is abstracted via the Command pattern (`PlayerCommands`, `SystemCommands`), allowing seamless, plug-and-play mapping for both Keyboard and Gamepads.
* **Multithreading:** Audio processing (`MiniaudioSoundSystem`) is offloaded to a background `std::jthread` to prevent audio decoding from blocking the main game loop, utilizing `std::unique_lock` to ensure thread-safe queueing.
* **PIMPL Idiom:** Third-party libraries (like `miniaudio.h`) are completely hidden behind the Pointer to Implementation (PIMPL) idiom to reduce compile times and ensure strict dependency isolation.

---

## Build Instructions
1. Clone the repository.
2. Ensure you have the latest stable version of Visual Studio and CMake installed.
3. Open the project folder in Visual Studio (or generate the solution via CMake).
4. Set the build target to **x64 Release**.
5. Build and run the `Digger` executable target.

---
*Developed by Ana Paoliello de Lucena Carvalho for DAE Programming 4*
