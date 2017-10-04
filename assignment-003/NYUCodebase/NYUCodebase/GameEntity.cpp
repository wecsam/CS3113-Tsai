#ifdef _WINDOWS
	#include <GL/glew.h>
#endif
#include "GameEntity.h"
#include "Dimensions.h"
#define SPRITE_SCALE 0.5f
Entity::UVWrap::UVWrap() {}
Entity::UVWrap::UVWrap(float U, float V, float Width, float Height) :
	U(U), V(V), Width(Width), Height(Height) {}
void Entity::UVWrap::GetTextureCoordinates(float* texCoords) const {
	SetBoxLeft(texCoords, U);
	SetBoxRight(texCoords, U + Width);
	SetBoxTop(texCoords, V);
	SetBoxBottom(texCoords, V + Height);
}
float Entity::GetLeftBoxBound() const {
	return vertices[VERTEX_INDICES::T1_TOP_LEFT_X];
}
float Entity::GetRightBoxBound() const {
	return vertices[VERTEX_INDICES::T1_TOP_RIGHT_X];
}
float Entity::GetTopBoxBound() const {
	return vertices[VERTEX_INDICES::T1_TOP_LEFT_Y];
}
float Entity::GetBottomBoxBound() const {
	return vertices[VERTEX_INDICES::T1_BOTTOM_LEFT_Y];
}
bool Entity::IsCollidingWith(const Entity& other) const {
	return !(
		other.GetTopBoxBound() < GetBottomBoxBound() ||
		GetTopBoxBound() < other.GetBottomBoxBound() ||
		other.GetRightBoxBound() < GetLeftBoxBound() ||
		GetRightBoxBound() < other.GetLeftBoxBound()
		);
}
void Entity::Draw(ShaderProgram& program) const {
	// Here are the actual draw calls.
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program.positionAttribute);
	glEnableVertexAttribArray(program.texCoordAttribute);
	glBindTexture(GL_TEXTURE_2D, spriteSheet);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);
}
void Entity::GetCenter(float& x, float& y) const {
	x = (GetLeftBoxBound() + GetRightBoxBound()) / 2.0f;
	y = (GetTopBoxBound() + GetBottomBoxBound()) / 2.0f;
}
void Entity::SetBoxLeft(float* a, float x) {
	a[VERTEX_INDICES::T1_TOP_LEFT_X] =
		a[VERTEX_INDICES::T1_BOTTOM_LEFT_X] =
		a[VERTEX_INDICES::T2_BOTTOM_LEFT_X] =
		x;
}
void Entity::SetBoxRight(float* a, float x) {
	a[VERTEX_INDICES::T1_TOP_RIGHT_X] =
		a[VERTEX_INDICES::T2_TOP_RIGHT_X] =
		a[VERTEX_INDICES::T2_BOTTOM_RIGHT_X] =
		x;
}
void Entity::SetBoxTop(float* a, float y) {
	a[VERTEX_INDICES::T1_TOP_LEFT_Y] =
		a[VERTEX_INDICES::T1_TOP_RIGHT_Y] =
		a[VERTEX_INDICES::T2_TOP_RIGHT_Y] =
		y;
}
void Entity::SetBoxBottom(float* a, float y) {
	a[VERTEX_INDICES::T1_BOTTOM_LEFT_Y] =
		a[VERTEX_INDICES::T2_BOTTOM_LEFT_Y] =
		a[VERTEX_INDICES::T2_BOTTOM_RIGHT_Y] =
		y;
}
Entity::Entity(GLuint spriteSheet, float x, float y, float width, float height, float scale = 1.0f) {
	SetSpriteSheet(spriteSheet);
	SetSprite(x, y, width, height, scale);
}
void Entity::MoveX(float dx) {
	SetBoxLeft(vertices, GetLeftBoxBound() + dx);
	SetBoxRight(vertices, GetRightBoxBound() + dx);
}
void Entity::MoveY(float dy) {
	SetBoxTop(vertices, GetTopBoxBound() + dy);
	SetBoxBottom(vertices, GetBottomBoxBound() + dy);
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
	// Recompute the bounding box.
	float orthoXBound = scale * width / WIDTH * ORTHO_X_BOUND,
		orthoYBound = scale * height / HEIGHT * ORTHO_Y_BOUND,
		centerX, centerY;
	GetCenter(centerX, centerY);
	SetBoxLeft(vertices, centerX - orthoXBound);
	SetBoxRight(vertices, centerX + orthoXBound);
	SetBoxTop(vertices, centerY + orthoYBound);
	SetBoxBottom(vertices, centerY - orthoYBound);
}
PlayerLaserCannon::PlayerLaserCannon(GLuint spriteSheet) :
	Entity(spriteSheet, 3.0f, 90.0f, 111.0f, 74.0f, SPRITE_SCALE) {}
void PlayerLaserCannon::ShowNoThrust() {
	SetSprite(3.0f, 90.0f, 111.0f, 74.0f, SPRITE_SCALE);
}
void PlayerLaserCannon::ShowThrustLeft() {
	SetSprite(3.0f, 238.0f, 111.0f, 74.0f, SPRITE_SCALE);
}
void PlayerLaserCannon::ShowThrustRight() {
	SetSprite(3.0f, 164.0f, 111.0f, 74.0f, SPRITE_SCALE);
}
Bullet::Bullet(GLuint spriteSheet) :
	Entity(spriteSheet, 177.0f, 25.0f, 12.0f, 64.0f, SPRITE_SCALE) {}
Invader::Invader(GLuint spriteSheet, INVADER_TYPE invaderType) :
	Entity(spriteSheet, 114.0f, 120.0f + invaderType * 64.0f, 84.0f, 64.0f, SPRITE_SCALE) {}
