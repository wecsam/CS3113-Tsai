#include "Switch.h"
Switch::Switch(float row, float column, int doorX, int doorY, bool facingLeft)
	: Tile(row - 1.0f, column - 1.0f, facingLeft ? 158 : 160), doorX(doorX), doorY(doorY) {}
int Switch::GetDoorX() const {
	return doorX;
}
int Switch::GetDoorY() const {
	return doorY;
}
bool Switch::IsOn() const {
	return GetType() >= 161;
}
void Switch::Flip() {
	if (IsOn()) {
		// The switch is on, and the door is open.
		SetType(GetType() - 4);
		if (Door) {
			Door->CloseDoor();
		}
	}
	else {
		// The switch is off, and the door is closed.
		SetType(GetType() + 4);
		if (Door) {
			auto test = Door->GetType();
			Door->OpenDoor();
		}
	}
}
