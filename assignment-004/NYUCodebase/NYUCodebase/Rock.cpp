#include "Rock.h"
#include "Dimensions.h"
Rock::Rock(unsigned int row, unsigned int column)
	: Rectangle(row, column), TouchedSinceLastFrame(this) {
	SetBox(texture, 0.0f, 1.0f, 1.0f, 0.0f);
	oldTopBoxBound = GetTopBoxBound();
}
void Rock::Follow(Player* follow) {
	following = follow;
}
void Rock::Follow(Rock* follow) {
	following = follow;
}
Rectangle* Rock::GetFollowing() const {
	return following;
}
void Rock::Fall() {
	following = nullptr;
	falling = true;
}
bool Rock::IsFalling() const {
	return falling;
}
bool Rock::IsNotFollowingOrMoving() const {
	return !following && !falling;
}
void Rock::UpdateModel(Uint32 MillisecondsElapsed) {
	oldTopBoxBound = GetTopBoxBound();
	if (following) {
		// Position this rock above the entity that it is following.
		SDL_memcpy(&model, &following->model, sizeof(model));
		model.Translate(-0.25f, 0.25f, 0.0f);
	}
	else if (falling) {
		// Make this rock fall.
		fallingVelocity += GRAVITY * MillisecondsElapsed;
		model.Translate(0.0f, fallingVelocity * MillisecondsElapsed, 0.0f);
	}
}
bool Rock::_TouchedSinceLastFrame::operator()(const Rectangle& other) const {
	return parent->falling &&
		parent->GetBottomBoxBound() < other.GetTopBoxBound() &&
		parent->oldTopBoxBound > other.GetBottomBoxBound() &&
		parent->GetLeftBoxBound() < other.GetRightBoxBound() &&
		parent->GetRightBoxBound() > other.GetLeftBoxBound();
}
Rock::_TouchedSinceLastFrame::_TouchedSinceLastFrame(const Rock* parent)
	: parent(parent) {}
