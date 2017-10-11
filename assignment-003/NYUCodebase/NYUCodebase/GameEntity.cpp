#ifdef _WINDOWS
	#include <GL/glew.h>
#endif
#include "GameEntity.h"
#include "Dimensions.h"
#include "Draw.h"
#define SPRITE_SCALE 0.5f
#define PLAYER_ACCEL 0.0000125f
#define PLAYER_BRAKE 0.01f
#define STATIONARY_VELOCITY 0.0001f
float lerp(float v0, float v1, float t) {
	return (1.0f - t) * v0 + t * v1;
}
Entity::UVWrap::UVWrap() {}
Entity::UVWrap::UVWrap(float U, float V, float Width, float Height) :
	U(U), V(V), Width(Width), Height(Height) {}
void Entity::UVWrap::GetTextureCoordinates(float* texCoords) const {
	SetBox(texCoords, V, U + Width, V + Height, U);
}
float Entity::GetLeftBoxBound() const {
	return GetCenterX() - orthoHalfWidth;
}
float Entity::GetRightBoxBound() const {
	return GetCenterX() + orthoHalfWidth;
}
float Entity::GetTopBoxBound() const {
	return GetCenterY() + orthoHalfHeight;
}
float Entity::GetBottomBoxBound() const {
	return GetCenterY() - orthoHalfHeight;
}
float Entity::GetWidth() const {
	return orthoHalfWidth * 2.0f;
}
float Entity::GetHeight() const {
	return orthoHalfHeight * 2.0f;
}
bool Entity::IsCollidingWith(const Entity& other) const {
	return !(
		other.GetTopBoxBound() < GetBottomBoxBound() ||
		GetTopBoxBound() < other.GetBottomBoxBound() ||
		other.GetRightBoxBound() < GetLeftBoxBound() ||
		GetRightBoxBound() < other.GetLeftBoxBound()
		);
}
void Entity::Draw() const {
	DrawTrianglesWithTexture(ModelviewMatrix, 2, vertices, texCoords, spriteSheet);
}
void Entity::GetCenter(float& x, float& y) const {
	x = GetCenterX();
	y = GetCenterY();
}
float Entity::GetCenterX() const {
	return ModelviewMatrix.m[3][0];
}
float Entity::GetCenterY() const {
	return ModelviewMatrix.m[3][1];
}
void Entity::SetBoxLeft(float* a, float x) {
	a[VertexIndices::T1_TOP_LEFT_X] =
		a[VertexIndices::T1_BOTTOM_LEFT_X] =
		a[VertexIndices::T2_BOTTOM_LEFT_X] =
		x;
}
void Entity::SetBoxRight(float* a, float x) {
	a[VertexIndices::T1_TOP_RIGHT_X] =
		a[VertexIndices::T2_TOP_RIGHT_X] =
		a[VertexIndices::T2_BOTTOM_RIGHT_X] =
		x;
}
void Entity::SetBoxTop(float* a, float y) {
	a[VertexIndices::T1_TOP_LEFT_Y] =
		a[VertexIndices::T1_TOP_RIGHT_Y] =
		a[VertexIndices::T2_TOP_RIGHT_Y] =
		y;
}
void Entity::SetBoxBottom(float* a, float y) {
	a[VertexIndices::T1_BOTTOM_LEFT_Y] =
		a[VertexIndices::T2_BOTTOM_LEFT_Y] =
		a[VertexIndices::T2_BOTTOM_RIGHT_Y] =
		y;
}
void Entity::SetBox(float* a, float top, float right, float bottom, float left) {
	SetBoxTop(a, top);
	SetBoxLeft(a, left);
	SetBoxRight(a, right);
	SetBoxBottom(a, bottom);
}
Entity::Entity(GLuint spriteSheet, float spriteSheetX, float spriteSheetY, float spriteWidthPx, float spriteHeightPx, float scale, float orthoPositionX, float orthoPositionY) {
	SetSpriteSheet(spriteSheet);
	SetSprite(spriteSheetX, spriteSheetY, spriteWidthPx, spriteHeightPx, scale);
	MoveX(orthoPositionX);
	MoveY(orthoPositionY);
}
void Entity::Move(float dx, float dy) {
	ModelviewMatrix.Translate(dx, dy, 0.0f);
}
void Entity::MoveX(float dx) {
	Move(dx, 0.0f);
}
void Entity::MoveY(float dy) {
	Move(0.0f, dy);
}
void Entity::SetSpriteSheet(GLuint spriteSheet) {
	// Store the texture ID.
	this->spriteSheet = spriteSheet;
}
void Entity::SetSprite(float x, float y, float width, float height, float scale = 1.0f) {
	// Store the new UV mapping.
	UV = UVWrap(
		x / SPRITE_SHEET_WIDTH,
		y / SPRITE_SHEET_HEIGHT,
		width / SPRITE_SHEET_WIDTH,
		height / SPRITE_SHEET_HEIGHT
	);
	// Recompute the texture coordinates array.
	UV.GetTextureCoordinates(texCoords);
	// Recompute the aspect ratio.
	orthoHalfWidth = scale * width / WIDTH * ORTHO_X_BOUND;
	orthoHalfHeight = scale * height / HEIGHT * ORTHO_Y_BOUND;
	SetBox(vertices, orthoHalfHeight, orthoHalfWidth, -orthoHalfHeight, -orthoHalfWidth);
}
PlayerLaserCannon::PlayerLaserCannon(GLuint spriteSheet, float orthoPositionX, float orthoPositionY) :
	Entity(spriteSheet, 3.0f, 90.0f, 111.0f, 74.0f, SPRITE_SCALE, orthoPositionX, orthoPositionY) {}
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
Bullet::Bullet(GLuint spriteSheet, bool fromPlayer, float orthoPositionX, float orthoPositionY) :
	Entity(spriteSheet, 177.0f, 25.0f, 12.0f, 64.0f, SPRITE_SCALE, orthoPositionX, orthoPositionY) {
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
Invader::Invader(GLuint spriteSheet, InvaderType invaderType, float orthoPositionX, float orthoPositionY) :
	Entity(spriteSheet, 114.0f, 120.0f + invaderType * 64.0f, 84.0f, 64.0f, SPRITE_SCALE, orthoPositionX, orthoPositionY),
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
