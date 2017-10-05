#pragma once
#include <vector>
#include "GameEntity.h"
class Lives {
public:
	Lives(GLuint spriteSheet, float plcWidth, size_t numberOfLives, float positionX, float positionY);
	void AddLife();
	void RemoveLife();
	void Draw(ShaderProgram&) const;
private:
	std::vector<PlayerLaserCannon> icons;
	const float x;
	const float y;
	const float plcWidth;
	const GLuint spriteSheet;
};
