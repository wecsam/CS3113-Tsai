#include "Lives.h"
Lives::Lives(float plcWidth, size_t numberOfLives, float positionX, float positionY) :
	x(positionX), y(positionY), plcWidth(plcWidth) {
	icons.reserve(numberOfLives);
	for (size_t i = 0; i < numberOfLives; ++i) {
		AddLife();
	}
}
void Lives::AddLife() {
	icons.emplace_back(
		x + icons.size() * plcWidth,
		y
	);
}
void Lives::RemoveLife() {
	icons.pop_back();
}
void Lives::Draw() const {
	for (const PlayerLaserCannon& p : icons) {
		p.Draw();
	}
}
size_t Lives::NumberLeft() const {
	return icons.size();
}
