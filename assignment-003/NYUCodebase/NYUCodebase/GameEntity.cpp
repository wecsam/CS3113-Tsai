#ifdef _WINDOWS
	#include <GL/glew.h>
#endif
#include "GameEntity.h"
#include "Dimensions.h"
#define SPRITE_SCALE 0.5f
#define PLAYER_ACCEL 0.0000125f
#define PLAYER_BRAKE 0.01f
#define STATIONARY_VELOCITY 0.0001f
GLuint spriteSheet;
float lerp(float v0, float v1, float t) {
	return (1.0f - t) * v0 + t * v1;
}
void Entity::Draw() const {
	Rectangle::Draw(spriteSheet);
}
Entity::Entity(float spriteSheetX, float spriteSheetY, float spriteWidthPx, float spriteHeightPx, float scale, float orthoPositionX, float orthoPositionY) {
	SetSprite(spriteSheetX, spriteSheetY, spriteWidthPx, spriteHeightPx, scale);
	Move(orthoPositionX, orthoPositionY);
}
void Entity::SetSprite(float x, float y, float width, float height, float scale = 1.0f) {
	// Recompute the vertex array.
	SetHalfWidthAndHalfHeight(scale * PIXEL_TO_ORTHO_X_HALF(width), scale * PIXEL_TO_ORTHO_Y_HALF(height));
	// Recompute the texture coordinates array.
	SetUV(
		x / SPRITE_SHEET_WIDTH,
		y / SPRITE_SHEET_HEIGHT,
		width / SPRITE_SHEET_WIDTH,
		height / SPRITE_SHEET_HEIGHT
	);
}
PlayerLaserCannon::PlayerLaserCannon(float orthoPositionX, float orthoPositionY) :
	Entity(3.0f, 90.0f, 111.0f, 74.0f, SPRITE_SCALE, orthoPositionX, orthoPositionY) {}
void PlayerLaserCannon::CalculateMotion(Uint32 millisecondsElapsed) {
	// Change the velocity based on the current movement type.
	switch (CurrentMovement) {
	case STATIONARY:
		// Slow the cannon down.
		velocity = lerp(velocity, 0.0f, millisecondsElapsed * PLAYER_BRAKE);
		// Show a reverse thrust.
		if (velocity > STATIONARY_VELOCITY) {
			ShowThrustLeft();
		}
		else if (velocity < -STATIONARY_VELOCITY) {
			ShowThrustRight();
		}
		else {
			ShowNoThrust();
		}
		break;
	case LEFT:
		// Move the cannon left.
		velocity -= PLAYER_ACCEL * millisecondsElapsed;
		ShowThrustLeft();
		break;
	case RIGHT:
		// Move the cannon right.
		velocity += PLAYER_ACCEL * millisecondsElapsed;
		ShowThrustRight();
		break;
	}
	// Change the position based on the velocity.
	MoveX(velocity * millisecondsElapsed);
	// Make sure that the cannon does not go off-screen.
	if (GetLeftBoxBound() < -ORTHO_X_BOUND && velocity < 0 || GetRightBoxBound() > ORTHO_X_BOUND && velocity > 0) {
		velocity *= -0.2f;
	}
}
void PlayerLaserCannon::ShowNoThrust() {
	if (CurrentThrust != STATIONARY) {
		SetSprite(3.0f, 90.0f, 111.0f, 74.0f, SPRITE_SCALE);
		CurrentThrust = STATIONARY;
	}
}
void PlayerLaserCannon::ShowThrustLeft() {
	if (CurrentThrust != LEFT) {
		SetSprite(3.0f, 238.0f, 111.0f, 74.0f, SPRITE_SCALE);
		CurrentThrust = LEFT;
	}
}
void PlayerLaserCannon::ShowThrustRight() {
	if (CurrentThrust != RIGHT) {
		SetSprite(3.0f, 164.0f, 111.0f, 74.0f, SPRITE_SCALE);
		CurrentThrust = RIGHT;
	}
}
Bullet::Bullet(bool fromPlayer, float orthoPositionX, float orthoPositionY) :
	Entity(177.0f, 25.0f, 12.0f, 64.0f, SPRITE_SCALE, orthoPositionX, orthoPositionY) {
	velocity = fromPlayer ? 0.012f : -0.01f;
}
bool Bullet::IsOffScreen() const {
	// Bullets can't travel sideways in this game, so we only need to check the Y position.
	return GetBottomBoxBound() > ORTHO_Y_BOUND || GetTopBoxBound() < -ORTHO_Y_BOUND;
}
void Bullet::CalculateMotion(Uint32 millisecondsElapsed) {
	// A bullet has no on-board propulsion, and the game is set in space.
	// Just move at a constant velocity.
	MoveY(velocity * millisecondsElapsed);
}
void Bullet::MoveOffScreen() {
	MoveY(2 * ORTHO_Y_BOUND);
}
Invader::Invader(InvaderType invaderType, float orthoPositionX, float orthoPositionY) :
	Entity(114.0f, 120.0f + invaderType * 64.0f, 84.0f, 64.0f, SPRITE_SCALE, orthoPositionX, orthoPositionY),
	type(invaderType) {}
void Invader::CalculateMotion(Uint32 millisecondsElapsed) {
	// TODO
}
const unsigned int Invader::points[] = {
	30,
	20,
	10
};
unsigned int Invader::GetPointValue() const {
	return points[type];
}
