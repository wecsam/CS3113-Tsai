#pragma once
#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include "Matrix.h"
class Rectangle
{
public:
	void Move(float, float);
	void MoveX(float);
	void MoveY(float);
	void GetCenter(float&, float&) const;
	float GetCenterX() const;
	float GetCenterY() const;
	float GetLeftBoxBound() const;
	float GetRightBoxBound() const;
	float GetTopBoxBound() const;
	float GetBottomBoxBound() const;
	float GetWidth() const;
	float GetHeight() const;
	static void SetBoxLeft(float*, float);
	static void SetBoxRight(float*, float);
	static void SetBoxTop(float*, float);
	static void SetBoxBottom(float*, float);
	static void SetBox(float*, float top, float right, float bottom, float left);
protected:
	Rectangle();
	Rectangle(float orthoHalfWidth, float orthoHalfHeight);
	Rectangle(float orthoHalfWidth, float orthoHalfHeight, float orthoPositionX, float orthoPositionY);
	void Draw(GLuint textureID) const;
	void SetHalfWidthAndHalfHeight(float orthoHalfWidth, float orthoHalfHeight);
	void SetUV(float U, float V, float width, float height);
	Matrix ModelviewMatrix;
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
};
