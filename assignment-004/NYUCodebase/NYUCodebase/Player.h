#pragma once
#include "Rectangle.h"
struct Player : Rectangle {
	Player(unsigned int row, unsigned int column);
	const float* GetVertices() const;
	const float* GetTexture() const;
	void Jump();
	void Stand();
	void Walk();
#include "PlayerStateEnum.txt"
	static const float STATE_VERTICES[][NUM_VERTICES];
	static const float STATE_TEXTURE[][NUM_VERTICES];
private:
	// The Player class will not use the default vertex array.
	static const float VERTICES[];
	States state;
};
