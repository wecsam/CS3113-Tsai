#pragma once
#include "Rectangle.h"
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
