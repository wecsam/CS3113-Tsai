#pragma once
#include "Tile.h"
class Switch : public Tile {
public:
	Switch(float row, float column, int doorX, int doorY, bool facingLeft);
	int GetDoorX() const;
	int GetDoorY() const;
	// Returns true if the switch is in the "on" position
	bool IsOn() const;
	// Flips the switch and opens or closes the door
	void Flip();
	Tile* Door = nullptr;
private:
	int doorX, doorY;
};
