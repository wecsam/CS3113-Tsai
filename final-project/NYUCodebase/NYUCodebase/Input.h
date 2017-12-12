#pragma once
struct Input
{
	Input();
	bool QuitRequested = false;
	bool EscapePressed = false;
	enum Direction { NONE, DOWN, LEFT, UP, RIGHT, NUM_DIRECTIONS };
	Direction PlayerDirection;
};
