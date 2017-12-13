#pragma once
#include "Rectangle.h"
#define PLAYER_HALF_WIDTH 0.125f
#define PLAYER_HALF_HEIGHT 0.125f
#define PLAYER_FEET_OFFSET_Y (0.75f * PLAYER_HALF_HEIGHT)
class Character : public Rectangle {
public:
	enum Direction { DOWN, LEFT, UP, RIGHT, NUM_DIRECTIONS };
	Character(float x, float y);
	void Face(Direction);
	void Walk();
	void Stand();
	Direction GetFacingDirection() const;
private:
	void UpdateTextureCoordinates();
	Direction facing;
	int walkFrame;
};
