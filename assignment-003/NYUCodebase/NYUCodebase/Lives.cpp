#include "Lives.h"
Lives::Lives(GLuint spriteSheet, float plcWidth, size_t numberOfLives, float positionX, float positionY) :
	x(positionX), y(positionY), plcWidth(plcWidth), spriteSheet(spriteSheet) {
	icons.reserve(numberOfLives);
	for (size_t i = 0; i < numberOfLives; ++i) {
		AddLife();
	}
}
void Lives::AddLife() {
	icons.emplace_back(
		spriteSheet,
		x + icons.size() * plcWidth,
		y
	);
}
void Lives::RemoveLife() {
	icons.pop_back();
}
void Lives::Draw(ShaderProgram& program) const {
	for (const PlayerLaserCannon& p : icons) {
		p.Draw(program);
	}
}
size_t Lives::NumberLeft() const {
	return icons.size();
}
