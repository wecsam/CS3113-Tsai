#include "Coin.h"
Coin::Coin(unsigned int row, unsigned int column)
	: Rectangle(row, column) {
	SetBox(texture, 0.0f, 1.0f, 1.0f, 0.0f);
}
