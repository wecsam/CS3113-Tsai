#include "Rectangle.h"
#include "Dimensions.h"
Rectangle::Rectangle(
	float x, float y,
	float halfWidth, float halfHeight
) : Model() {
	Model.Translate(x, y, 0.0f);
	SetBox(vertices, halfHeight, halfWidth, -halfHeight, -halfWidth);
}
Rectangle::Rectangle(
	float x, float y,
	float halfWidth, float halfHeight,
	float u, float v,
	float textureWidth, float textureHeight
) : Rectangle(x, y, halfWidth, halfHeight) {
	SetBox(texture, v, u + textureWidth, v + textureHeight, u);
}
const float* Rectangle::GetVertices() const {
	return vertices;
}
const float* Rectangle::GetTextureCoordinates() const {
	return texture;
}
float Rectangle::GetCenterX() const {
	return Model.m[3][0];
}
float Rectangle::GetCenterY() const {
	return Model.m[3][1];
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
