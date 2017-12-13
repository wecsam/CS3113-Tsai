#include "Input.h"
#include <SDL.h>
Input::Input() {
	// Scan for input events.
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
		case SDL_WINDOWEVENT_CLOSE:
			QuitRequested = true;
			break;
		case SDL_KEYDOWN:
			switch (event.key.keysym.scancode) {
			case SDL_SCANCODE_DOWN:
				DownPressed = true;
				break;
			case SDL_SCANCODE_RETURN:
			case SDL_SCANCODE_RETURN2:
			case SDL_SCANCODE_KP_ENTER:
				EnterPressed = true;
				break;
			case SDL_SCANCODE_ESCAPE:
				EscapePressed = true;
				break;
			case SDL_SCANCODE_SPACE:
				SpacePressed = true;
				break;
			case SDL_SCANCODE_UP:
				UpPressed = true;
				break;
			}
			break;
		}
	}
	// Poll for pressed keys.
	const Uint8* keys = SDL_GetKeyboardState(NULL);
	if (keys[SDL_SCANCODE_DOWN]) {
		PlayerDirection = DOWN;
	}
	else if (keys[SDL_SCANCODE_LEFT]) {
		PlayerDirection = LEFT;
	}
	else if (keys[SDL_SCANCODE_UP]) {
		PlayerDirection = UP;
	}
	else if (keys[SDL_SCANCODE_RIGHT]) {
		PlayerDirection = RIGHT;
	}
	else {
		PlayerDirection = NONE;
	}
}
