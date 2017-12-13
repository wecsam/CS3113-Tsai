#pragma once
struct Input
{
	Input();
	bool QuitRequested = false;
	bool DownPressed = false;
	bool EnterPressed = false;
	bool EscapePressed = false;
	bool SpacePressed = false;
	bool UpPressed = false;
	enum Direction { NONE, DOWN, LEFT, UP, RIGHT, NUM_DIRECTIONS };
	Direction PlayerDirection;
};
