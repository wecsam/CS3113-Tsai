#pragma once
#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL_stdinc.h>
#include "Rectangle.h"
extern GLuint anExplosion;

class Animation : public Rectangle {
public:
	Animation(GLuint animationSheet, unsigned int frameWidth, unsigned int frameHeight, unsigned int framesPerRow, long framesTotal, float fps, float orthoPositionX, float orthoPositionY);
	void Rewind();
	// Returns true when the animation has completed.
	bool Draw();
private:
	GLuint frameSheet;
	unsigned int frameWidth;
	unsigned int frameHeight;
	unsigned int framesPerRow;
	int numberOfRows;
	long framesTotal;
	float fps;
	Uint32 startTick;
};
class ExplosionAnimation : public Animation {
public:
	ExplosionAnimation(float orthoPositionX, float orthoPositionY);
};
