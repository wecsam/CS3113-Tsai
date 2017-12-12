#include "Character.h"
#include <SDL.h>
#define NUM_WALKING_FRAMES 4
#define PLAYER_HALF_WIDTH 0.125f
#define PLAYER_HALF_HEIGHT 0.125f
Character::Character(float x, float y) : Rectangle(x, y, PLAYER_HALF_WIDTH, PLAYER_HALF_HEIGHT) {
	Face(DOWN);
	Stand();
}
void Character::Face(Direction d) {
	facing = d;
}
void Character::Walk() {
	walkFrame = SDL_GetTicks() / 100 % NUM_WALKING_FRAMES;
	UpdateTextureCoordinates();
}
void Character::Stand() {
	walkFrame = 0;
	UpdateTextureCoordinates();
}
Character::Direction Character::GetFacingDirection() const {
	return facing;
}
void Character::UpdateTextureCoordinates() {
	SetBox(
		texture,
		walkFrame / (float)NUM_WALKING_FRAMES,
		(facing + 1) / (float)NUM_DIRECTIONS,
		(walkFrame + 1) / (float)NUM_WALKING_FRAMES,
		facing / (float)NUM_DIRECTIONS
	);
}
