#pragma once
#include <SDL.h>
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
	void MoveX(float);
	void MoveY(float);
	virtual void CalculateMotion(Uint32) = 0;
	static void SetBoxLeft(float*, float);
	static void SetBoxRight(float*, float);
	static void SetBoxTop(float*, float);
	static void SetBoxBottom(float*, float);
	static void SetBox(float*, float top, float right, float bottom, float left);
	struct UVWrap {
		float U, V, Width, Height;
		UVWrap();
		UVWrap(float U, float V, float Width, float Height);
		void GetTextureCoordinates(float*) const;
	};
protected:
	Entity(GLuint, float, float, float, float, float);
	// To determine which sprite appears on screen, the texture must be mapped onto this rectangle.
	void SetSpriteSheet(GLuint);
	// To change the sprite, use SetSprite and give it the coordinates of the image.
	void SetSprite(float, float, float, float, float);
	UVWrap UV;
private:
	// Each entity is made of two triangles that form a rectangle together.
	// Each triangle has three vertices.
	// Each vertex has two floats.
	// That makes 12 floats total in the vertex array.
	enum VertexIndices {
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
	float vertices[VertexIndices::NUM_VERTICES] = { 0.0f };
	float texCoords[VertexIndices::NUM_VERTICES];
	GLuint spriteSheet;
};
class PlayerLaserCannon : public Entity {
public:
	PlayerLaserCannon(GLuint);
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
	Bullet(GLuint, bool, float = 0.0f, float = 0.0f);
	bool IsOffScreen() const;
	void CalculateMotion(Uint32);
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
	Invader(GLuint, InvaderType);
	void CalculateMotion(Uint32);
};
