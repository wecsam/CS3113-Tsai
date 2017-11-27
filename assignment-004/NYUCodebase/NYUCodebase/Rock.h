#pragma once
#include <SDL.h>
#include "Rectangle.h"
#include "Player.h"
class Rock : public Rectangle {
public:
	Rock(unsigned int row, unsigned int column);
	void Follow(Player*);
	void Follow(Rock*);
	Rectangle* GetFollowing() const;
	void Fall();
	bool IsFalling() const;
	bool IsNotFollowingOrMoving() const;
	void UpdateModel(Uint32 MillisecondsElapsed);
	class _TouchedSinceLastFrame {
	public:
		bool operator()(const Rectangle&) const;
		friend Rock;
	private:
		_TouchedSinceLastFrame(const Rock*);
		const Rock* parent;
	} TouchedSinceLastFrame;
private:
	Rectangle* following = nullptr;
	bool falling = false;
	float fallingVelocity = 0.0f;
	float oldTopBoxBound;
};

