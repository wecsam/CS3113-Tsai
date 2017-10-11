#include "Rectangle.h"
#include "Draw.h"
void Rectangle::Move(float dx, float dy) {
	ModelviewMatrix.Translate(dx, dy, 0.0f);
}
void Rectangle::MoveX(float dx) {
	Move(dx, 0.0f);
}
void Rectangle::MoveY(float dy) {
	Move(0.0f, dy);
}
void Rectangle::GetCenter(float& x, float& y) const {
	x = GetCenterX();
	y = GetCenterY();
}
float Rectangle::GetCenterX() const {
	return ModelviewMatrix.m[3][0];
}
float Rectangle::GetCenterY() const {
	return ModelviewMatrix.m[3][1];
}
float Rectangle::GetLeftBoxBound() const {
	return GetCenterX() + vertices[T1_TOP_LEFT_X];
}
float Rectangle::GetRightBoxBound() const {
	return GetCenterX() + vertices[T1_TOP_RIGHT_X];
}
float Rectangle::GetTopBoxBound() const {
	return GetCenterY() + vertices[T1_TOP_LEFT_Y];
}
float Rectangle::GetBottomBoxBound() const {
	return GetCenterY() + vertices[T1_BOTTOM_LEFT_Y];
}
float Rectangle::GetWidth() const {
	return vertices[T1_TOP_RIGHT_X] - vertices[T1_TOP_LEFT_X];
}
float Rectangle::GetHeight() const {
	return vertices[T1_TOP_LEFT_Y] - vertices[T1_BOTTOM_LEFT_Y];
}
void Rectangle::SetBoxLeft(float* a, float x) {
	a[VertexIndices::T1_TOP_LEFT_X] =
		a[VertexIndices::T1_BOTTOM_LEFT_X] =
		a[VertexIndices::T2_BOTTOM_LEFT_X] =
		x;
}
void Rectangle::SetBoxRight(float* a, float x) {
	a[VertexIndices::T1_TOP_RIGHT_X] =
		a[VertexIndices::T2_TOP_RIGHT_X] =
		a[VertexIndices::T2_BOTTOM_RIGHT_X] =
		x;
}
void Rectangle::SetBoxTop(float* a, float y) {
	a[VertexIndices::T1_TOP_LEFT_Y] =
		a[VertexIndices::T1_TOP_RIGHT_Y] =
		a[VertexIndices::T2_TOP_RIGHT_Y] =
		y;
}
void Rectangle::SetBoxBottom(float* a, float y) {
	a[VertexIndices::T1_BOTTOM_LEFT_Y] =
		a[VertexIndices::T2_BOTTOM_LEFT_Y] =
		a[VertexIndices::T2_BOTTOM_RIGHT_Y] =
		y;
}
void Rectangle::SetBox(float* a, float top, float right, float bottom, float left) {
	SetBoxTop(a, top);
	SetBoxLeft(a, left);
	SetBoxRight(a, right);
	SetBoxBottom(a, bottom);
}
Rectangle::Rectangle() {}
Rectangle::Rectangle(float orthoHalfWidth, float orthoHalfHeight) {
	SetHalfWidthAndHalfHeight(orthoHalfWidth, orthoHalfHeight);
}
Rectangle::Rectangle(float orthoHalfWidth, float orthoHalfHeight, float orthoPositionX, float orthoPositionY) :
	Rectangle(orthoHalfWidth, orthoHalfHeight) {
	Move(orthoPositionX, orthoPositionY);
}
void Rectangle::Draw(GLuint textureID) const {
	DrawTrianglesWithTexture(ModelviewMatrix, 2, vertices, texCoords, textureID);
}
void Rectangle::SetHalfWidthAndHalfHeight(float orthoHalfWidth, float orthoHalfHeight) {
	SetBox(vertices, orthoHalfHeight, orthoHalfWidth, -orthoHalfHeight, -orthoHalfWidth);
}
void Rectangle::SetUV(float U, float V, float width, float height) {
	SetBox(texCoords, V, U + width, V + height, U);
}
