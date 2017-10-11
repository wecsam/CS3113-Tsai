#pragma once
#include <SDL.h>
#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include "Rectangle.h"
#define SPRITE_SHEET_WIDTH 200.0f // pixels
#define SPRITE_SHEET_HEIGHT 314.0f // pixels
extern GLuint spriteSheet;
class Entity : public Rectangle {
public:
	void Draw() const;
	virtual void CalculateMotion(Uint32) = 0;
protected:
	Entity(float spriteSheetX, float spriteSheetY, float spriteWidthPx, float spriteHeightPx, float scale = 1.0f, float orthoPositionX = 0.0f, float orthoPositionY = 0.0f);
	// To change the sprite, use SetSprite and give it the coordinates of the image.
	void SetSprite(float, float, float, float, float);
};
class PlayerLaserCannon : public Entity {
public:
	PlayerLaserCannon(float orthoPositionX = 0.0f, float orthoPositionY = 0.0f);
	void CalculateMotion(Uint32);
	void ShowNoThrust();
	void ShowThrustLeft();
	void ShowThrustRight();
	enum Movements {
		STATIONARY,
		LEFT,
		RIGHT
	};
	Movements CurrentMovement = Movements::STATIONARY;
private:
	Movements CurrentThrust = Movements::STATIONARY;
	float velocity = 0.0f;
};
class Bullet : public Entity {
public:
	Bullet(bool fromPlayer, float orthoPositionX = 0.0f, float orthoPositionY = 0.0f);
	bool IsOffScreen() const;
	void CalculateMotion(Uint32);
	void MoveOffScreen();
private:
	float velocity;
};
class Invader : public Entity {
public:
	enum InvaderType {
		BACK,
		MIDDLE,
		FRONT
	};
	Invader(InvaderType, float orthoPositionX = 0.0f, float orthoPositionY = 0.0f);
	void CalculateMotion(Uint32);
	unsigned int GetPointValue() const;
private:
	static const unsigned int points[];
	InvaderType type;
};
