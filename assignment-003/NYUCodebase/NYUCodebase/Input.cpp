#include "Input.h"
Input::Input() {
	// Scan for input events.
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
		case SDL_WINDOWEVENT_CLOSE:
			QuitRequested = true;
		case SDL_KEYDOWN:
			switch (event.key.keysym.scancode) {
			case SDL_SCANCODE_SPACE:
				++BulletsToFire;
				break;
			case SDL_SCANCODE_ESCAPE:
				EscapePressed = true;
				break;
			}
			break;
		}
	}
	// Poll for pressed keys.
	const Uint8* keys = SDL_GetKeyboardState(NULL);
	if (keys[SDL_SCANCODE_LEFT]) {
		PlayerMovement = PlayerLaserCannon::Movements::LEFT;
	}
	else if (keys[SDL_SCANCODE_RIGHT]) {
		PlayerMovement = PlayerLaserCannon::Movements::RIGHT;
	}
	else {
		PlayerMovement = PlayerLaserCannon::Movements::STATIONARY;
	}
}

void Input::Process(GLuint spriteSheet, PlayerLaserCannon & player, std::forward_list<Bullet>& bullets) {
	player.CurrentMovement = PlayerMovement;
	while (BulletsToFire-- > 0) {
		bullets.emplace_front(spriteSheet, true, (player.GetLeftBoxBound() + player.GetRightBoxBound()) / 2.0f, -2.8f);
	}
}
