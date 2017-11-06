#pragma once
#include <SDL.h>
#include "Rectangle.h"
struct Player : Rectangle {
	Player(unsigned int row, unsigned int column);
	const float* GetVertices() const;
	const float* GetTexture() const;
	float GetLeftBoxBound() const;
	float GetRightBoxBound() const;
	float GetTopBoxBound() const;
	float GetBottomBoxBound() const;
	void StayAbove(float y);
	void StayToRightOf(float x);
	void StayBelow(float y);
	void StayToLeftOf(float x);
	void Jump();
	void Stand();
	void Walk();
	void ProcessInput(Uint32 MillisecondsElapsed);
	class _ContainsCenterOf {
	public:
		bool operator()(const Rectangle&) const;
		friend Player;
	private:
		_ContainsCenterOf(Player*);
		Player* parent;
	} ContainsCenterOf;
#include "PlayerStateEnum.txt"
	static const float STATE_VERTICES[][NUM_VERTICES];
	static const float STATE_TEXTURE[][NUM_VERTICES];
private:
	// The Player class will not use the default vertex array.
	static const float VERTICES[];
	States state;
	float velocityX = 0.0f;
	float velocityY = 0.0f;
	float accelerationX = 0.0f;
	int allowedJumps = 0;
	uint8_t lastStateOfUpKey = 0;
};
