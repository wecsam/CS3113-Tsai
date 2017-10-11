#include "Animation.h"
#include <cstdlib>
#include <SDL_timer.h>
#include "Dimensions.h"
GLuint anExplosion;

Animation::Animation(GLuint frameSheet, unsigned int frameWidth, unsigned int frameHeight, unsigned int framesPerRow, long framesTotal, float fps, float orthoPositionX, float orthoPositionY) :
	Rectangle(PIXEL_TO_ORTHO_X(frameWidth) / 2.0f, PIXEL_TO_ORTHO_Y(frameHeight) / 2.0f, orthoPositionX, orthoPositionY),
	frameSheet(frameSheet),
	frameWidth(frameWidth), frameHeight(frameHeight),
	framesPerRow(framesPerRow), numberOfRows((framesTotal + framesPerRow - 1) / framesPerRow),
	framesTotal(framesTotal), fps(fps) {
	Rewind();
}
void Animation::Rewind() {
	startTick = SDL_GetTicks();
}
bool Animation::Draw() {
	// Calculate which frame the animation should be on
	// based on how much time has passed since Rewind()
	// was last called.
	long frame = (long)((SDL_GetTicks() - startTick) * fps / 1000);
	// Only draw if the animation has not completed.
	if (frame < framesTotal) {
		// Map the frame number to a position on the frame sheet.
		ldiv_t gridPosition = div(frame, framesPerRow);
		SetUV(gridPosition.rem / (float)framesPerRow, gridPosition.quot / (float)numberOfRows, 1.0f / framesPerRow, 1.0f / numberOfRows);
		// Draw the frame.
		Rectangle::Draw(frameSheet);
		return false;
	}
	return true;
}
ExplosionAnimation::ExplosionAnimation(float orthoPositionX, float orthoPositionY) :
	Animation(anExplosion, 97, 124, 16, 86, 60.0f, orthoPositionX, orthoPositionY + PIXEL_TO_ORTHO_Y(50.0f)) {}
