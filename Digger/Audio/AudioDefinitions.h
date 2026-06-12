#ifndef AUDIO_DEFINITIONS_H
#define AUDIO_DEFINITIONS_H

// ARCHITECTURAL DECISION - Centralized Enum for Audio
// Separated this into its own file so different components (Player or UI) can trigger sounds without needing to pass hardcoded strings
// Why - Because it prevents typos and makes it faster for the CPU to look up sounds

// Digger Sounds
enum class AudioDefinitions : unsigned short
{
	MUSIC = 0,
	BONUS = 1,
	NEXT_LEVEL = 2,
	DEATH = 3,
    PICK_UP = 4,
    BONUS_PICKUP = 5,
    COMBO_8_EMES = 6,
    KILL_ENEMY = 7,
    SHOOT = 8
};

#endif
