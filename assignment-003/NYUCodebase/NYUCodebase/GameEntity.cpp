#include "GameEntity.h"
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
void Entity::SetLeftBoxBound(float x) {
	vertices[VERTEX_INDICES::T1_TOP_LEFT_X] =
		vertices[VERTEX_INDICES::T1_BOTTOM_LEFT_X] =
		vertices[VERTEX_INDICES::T2_BOTTOM_LEFT_X] =
		x;
}
void Entity::SetRightBoxBound(float x) {
	vertices[VERTEX_INDICES::T1_TOP_RIGHT_X] =
		vertices[VERTEX_INDICES::T2_TOP_RIGHT_X] =
		vertices[VERTEX_INDICES::T2_BOTTOM_RIGHT_X] =
		x;
}
void Entity::SetTopBoxBound(float y) {
	vertices[VERTEX_INDICES::T1_TOP_LEFT_Y] =
		vertices[VERTEX_INDICES::T1_TOP_RIGHT_Y] =
		vertices[VERTEX_INDICES::T2_TOP_RIGHT_Y] =
		y;
}
void Entity::SetBottomBoxBound(float y) {
	vertices[VERTEX_INDICES::T1_BOTTOM_LEFT_Y] =
		vertices[VERTEX_INDICES::T2_BOTTOM_LEFT_Y] =
		vertices[VERTEX_INDICES::T2_BOTTOM_RIGHT_Y] =
		y;
}
void Entity::MoveX(float dx) {
	SetLeftBoxBound(GetLeftBoxBound() + dx);
	SetRightBoxBound(GetRightBoxBound() + dx);
}
void Entity::MoveY(float dy) {
	SetTopBoxBound(GetTopBoxBound() + dy);
	SetBottomBoxBound(GetBottomBoxBound() + dy);
}
