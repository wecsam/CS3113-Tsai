#include "Player.h"
#include <SDL.h>
Player::Player(unsigned int row, unsigned int column)
	: Rectangle(row, column) {
	Stand();
}
const float* Player::GetVertices() const {
	return STATE_VERTICES[state];
}
const float* Player::GetTexture() const {
	return STATE_TEXTURE[state];
}
void Player::Jump() {
	state = JUMP;
}
void Player::Stand() {
	state = STAND;
}
void Player::Walk() {
	state = static_cast<States>(WALK01 + SDL_GetTicks() % (NUM_STATES - WALK01));
}
#include "PlayerStateData.txt"
