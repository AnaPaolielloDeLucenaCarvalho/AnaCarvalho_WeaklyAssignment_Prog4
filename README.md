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
| :--- | :--- | :--- | :--- |
| **Navigate Up** | `W` / `Up Arrow` | D-Pad Up |
| **Navigate Down** | `S` / `Down Arrow` | D-Pad Down |
| **Next Letter** (Name) | `D` / `Right Arrow` | D-Pad Right |
| **Previus Letter** (Name) | `A` / `Left Arrow` | D-Pad Left |
| **Select / Confirm** | `Space` / `Enter` | Button A |

### Gameplay (Player 1)
| Action | Keyboard | Gamepad 0 |
| :--- | :--- | :--- |
| **Move Up** | `W` | D-Pad Up / Joystick |
| **Move Down** | `S` | D-Pad Down / Joystick |
| **Move Left** | `A` | D-Pad Left / Joystick |
| **Move Right** | `D` | D-Pad Right / Joystick |
| **Shoot** | `Space` | Left Trigger / Right Trigger |

### Gameplay (Player 2)
| Action | Keyboard | Gamepad 1 |
| :--- | :--- | :--- |
| **Move Up** | `I` | D-Pad Up / Joystick |
| **Move Down** | `K` | D-Pad Down / Joystick |
| **Move Left** | `J` | D-Pad Left / Joystick |
| **Move Right** | `L` | D-Pad Right / Joystick  |
| **Shoot** | `Right Shift` | Left Trigger / Right Trigger |

### System Commands (Global Hotkeys)
| Action | Keyboard | Gamepad 0 / 1 | Description |
| :--- | :--- | :--- | :--- |
| **Skip Level** | `F1` | Button B |
| **Mute Audio** | `F2` | Button Y |
| **Instructions** | `F3` | Button X |
| **Main Menu** | `F10` | Button A |

---

## Engine Architecture
This game runs on **Minigin**. The architecture heavily applies principles from Robert Nystrom's *Game Programming Patterns* and Gregory Davidson's *Beautiful C++*.

---

## Build Instructions
1. Clone the repository.
2. Ensure you have the latest stable version of Visual Studio and CMake installed.
3. Open the project folder in Visual Studio (or generate the solution via CMake).
4. Set the build target to **x64 Release**.
5. Build and run the `Digger` executable target.

---
*Developed by Ana Paoliello de Lucena Carvalho for DAE Programming 4*
