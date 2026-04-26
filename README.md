# Digger Game - Minigin Engine

This project is trying to recreate the 1983 arcade game **Digger**, from scratch, using a custom C++ game engine. It's being developed as my final exam project for the **Programming 4** course at Digital Arts & Entertainment (Howest).

## Links
* **GitHub Pages Build (Web):** [Play Digger on the WEB Here](https://anapaoliellodelucenacarvalho.github.io/AnaCarvalho_WeaklyAssignment_Prog4/)

---

## Roadmap / Progress Tracker

### Engine & Exam Requirements
- [ ] Parse and load 3 custom level layouts from data files
- [ ] Implement F1 level skip functionality
- [ ] Fix memory leaks
- [ ] Persistent High Scores (Local saving)

### Gameplay Mechanics
- [ ] Core Digger mechanics (digging through dirt)
- [ ] Enemy AI (Nobbins and Hobbins)
- [ ] Coin bags (Gravity, falling, and breaking)
- [ ] Cherry / Bonus item spawning
- [ ] Fully implement Two-Player Modes (Co-Op & Versus)
- [ ] Map rendering fix (Black part missing correct texture)

### Game Flow & UI
- [ ] Main Menu screen
- [ ] Win / Next Level transition logic
- [ ] Game Over screen
- [ ] High Score leaderboard UI

### Audio System
- [ ] Complete Audio implementation:
  - [x] Main background music
  - [x] Death sound
  - [ ] Point collection sound
  - [ ] Level win sound
  - [ ] Bonus sound/music
- [x] Audio system foundation built
- [ ] Audio system be able to stop/pause/play from begging
- [ ] F2 mute/unmute audio toggle

---

## Game Features
* **Multiple Modes:**
  * **Single Player:** Classic gameplay
  * **Co-Op:** Two players control Diggers to clear the mines together.
  * **Versus:** Player 1 controls Digger, while Player 2 controls a Nobbin.
* **Persistent High Scores:** Arcade-style leaderboard that saves scores locally.
* **3 Custom Levels:** Loaded from data files.
* **Audio & Music:** Features the original music: "Popcorn" and Chopin's "Funeral March".

---

## Controls
The game supports both **Keyboard** and **Gamepads** for gameplay and menu navigation.

### Player 1 (Default)
* **Move:** Arrow Keys (or D-Pad/Joystick on Controller 1)
* **Fire Fireball:** `F1` (or Controller 1 Button A/Cross)

### Player 2 (Co-op / Versus)
* **Move:** `W`, `A`, `S`, `D` (or D-Pad/Joystick on Controller 2)
* **Fire Fireball:** `Tab` (or Controller 2 Button A/Cross)

### Global / Engine Controls
* **Skip Level:** `F1`
* **Mute/Unmute Audio:** `F2`
* **Pause Game:** `Space`
* **Return to Title:** `F10`

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
