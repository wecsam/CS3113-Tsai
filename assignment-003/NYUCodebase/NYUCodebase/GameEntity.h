#pragma once
#include "ShaderProgram.h"
#define SPRITE_SHEET_WIDTH 200.0f // pixels
#define SPRITE_SHEET_HEIGHT 314.0f // pixels
class Entity {
public:
	float GetLeftBoxBound() const;
	float GetRightBoxBound() const;
	float GetTopBoxBound() const;
	float GetBottomBoxBound() const;
	bool IsCollidingWith(const Entity&) const;
	void Draw(ShaderProgram&) const;
	void GetCenter(float&, float&) const;
protected:
	Entity(GLuint, float, float, float, float, float);
	void MoveX(float);
	void MoveY(float);
	// To determine which sprite appears on screen, the texture must be mapped onto this rectangle.
	void SetSpriteSheet(GLuint);
	// To change the sprite, use SetSprite and give it the coordinates of the image.
	void SetSprite(float, float, float, float, float);
	struct UVWrap {
		float U, V, Width, Height;
		UVWrap();
		UVWrap(float, float, float, float);
		void GetTextureCoordinates(float*) const;
	};
	UVWrap UV;
private:
	static void SetBoxLeft(float*, float);
	static void SetBoxRight(float*, float);
	static void SetBoxTop(float*, float);
	static void SetBoxBottom(float*, float);
	// Each entity is made of two triangles that form a rectangle together.
	// Each triangle has three vertices.
	// Each vertex has two floats.
	// That makes 12 floats total in the vertex array.
	enum VERTEX_INDICES {
		T1_TOP_LEFT_X,
		T1_TOP_LEFT_Y,
		T1_BOTTOM_LEFT_X,
		T1_BOTTOM_LEFT_Y,
		T1_TOP_RIGHT_X,
		T1_TOP_RIGHT_Y,
		T2_TOP_RIGHT_X,
		T2_TOP_RIGHT_Y,
		T2_BOTTOM_LEFT_X,
		T2_BOTTOM_LEFT_Y,
		T2_BOTTOM_RIGHT_X,
		T2_BOTTOM_RIGHT_Y,
		NUM_VERTICES
	};
	float vertices[VERTEX_INDICES::NUM_VERTICES] = { 0.0f };
	float texCoords[VERTEX_INDICES::NUM_VERTICES];
	GLuint spriteSheet;
};
class PlayerLaserCannon : public Entity {
public:
	PlayerLaserCannon(GLuint);
	void ShowNoThrust();
	void ShowThrustLeft();
	void ShowThrustRight();
};
