#include <ctime>
#include <cstdlib>
#ifdef _WINDOWS
	#include <GL/glew.h>
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "stb_image.h"
#include "ShaderProgram.h"
#define WIDTH 640
#define HEIGHT 360
#define ORTHO_X_BOUND 3.55f
#define ORTHO_Y_BOUND 2.0f
#define PADDLE_WIDTH 0.1f
#define PADDLE_ACCEL 0.0000125f
#define PADDLE_BRAKE 0.000025f
#define BALL_DRAG 0.04f
#define PADDLE_ANGLE_STRIKE 0.005f

SDL_Window* displayWindow;
const float const verticesBallStartingPosition[] = {
	-0.05, 0.05,  // Left top
	-0.05, -0.05, // Left bottom
	0.05, 0.05,   // Right top
	0.05, -0.05,  // Right bottom
	0.05, 0.05,   // Right top
	-0.05, -0.05  // Left bottom
};

float RandomFloat(float max = 1.0f, float min = 0.0f) {
	return min + (max - min) * rand() / (float)RAND_MAX;
}

void SetRandomBackgroundColor() {
	glClearColor(RandomFloat(0.4f), RandomFloat(0.4f), RandomFloat(0.4f), 1.0f);
}

void MoveVertices(float* vertices, register size_t start, size_t end, float delta) {
	// If start is even, move the X coordinates.
	// If start is odd, move the Y coordinates.
	for (; start < end; start += 2) {
		vertices[start] += delta;
	}
}

void MovePaddle(float* vertices, float& velocity, Uint32 millisecondsElapsed, bool up, bool down) {
	/*
	Call this function once per paddle per frame.
	Arguments:
		vertices: an array of twelve floats (three vertices in each of two triangles)
		velocity: the current velocity of the paddle, in units per millisecond
		millisecondsElapsed: the number of milliseconds that have elapsed since the last frame
		up: if true, the paddle moves up
		down: if true, the paddle moves down
	*/
	if (up) {
		velocity += PADDLE_ACCEL * millisecondsElapsed;
	}
	else if (down) {
		velocity -= PADDLE_ACCEL * millisecondsElapsed;
	}
	else {
		// No keys are pressed. Stop the paddle.
		if (fabs(velocity) < 0.005f) {
			velocity = 0.0f;
		}
		else if (velocity > 0) {
			velocity -= PADDLE_BRAKE * millisecondsElapsed;
		}
		else {
			velocity += PADDLE_BRAKE * millisecondsElapsed;
		}
	}
	MoveVertices(vertices, 1, 12, millisecondsElapsed * velocity);
	// Make sure that the paddle does not go off screen.
	// If the paddle hits the edge, reverse and reduce its velocity.
	if (vertices[1] >= ORTHO_Y_BOUND && velocity > 0.0f || vertices[3] <= -ORTHO_Y_BOUND && velocity < 0.0f) {
		velocity *= -0.2f;
	}
}

void ResetBallPosition(float* vertices) {
	memcpy(vertices, verticesBallStartingPosition, 12 * sizeof(float));
}

bool BallPaddleCollision(const float* paddleVertices, const float* ballVertices, bool paddleIsLeft) {
	/*
	Test whether the paddle is hitting the ball.
	Arguments:
		paddleVertices: an array of twelve floats (three vertices in each of two triangles)
		ballVertices: an array of twelve floats (three vertices in each of two triangles)
	*/
	return
		// The top of the ball is above the bottom of the paddle, and the bottom of the ball is below the top of the paddle.
		(ballVertices[1] >= paddleVertices[3] && ballVertices[3] <= paddleVertices[1]) &&
		(
			paddleIsLeft ?
			// The left of the paddle is to the left of the right of the paddle.
			ballVertices[0] <= paddleVertices[4] :
			// The right of the paddle is to the right of the left of the paddle.
			ballVertices[4] >= paddleVertices[0]
		);
}

bool MoveBall(float* leftPaddleVertices, float* rightPaddleVertices, float* ballVertices, float& xVelocity, float& yVelocity, Uint32 millisecondsElapsed) {
	/*
	Arguments:
		leftPaddleVertices: an array of twelve floats (three vertices in each of two triangles)
		rightPaddleVertices: an array of twelve floats (three vertices in each of two triangles)
		ballVertices: an array of twelve floats (three vertices in each of two triangles)
		xVelocity: the current velocity of the ball in the X direction, in units per millisecond
		yVelocity: the current velocity of the ball in the Y direction, in units per millisecond
		millisecondsElapsed: the number of milliseconds that have elapsed since the last frame
	Returns:
		whether a player scored
	*/
	// Check whether the ball has hit a paddle.
	if (BallPaddleCollision(leftPaddleVertices, ballVertices, true)) {
		if (xVelocity < 0.0f) {
			// Send the ball heading toward the right with a little extra speed.
			xVelocity = 1.4f * fabs(xVelocity);
			// The Y velocity should be set proportionally to the difference from the ball's center Y to the paddle's center Y.
			yVelocity = (ballVertices[1] + ballVertices[3]) * PADDLE_ANGLE_STRIKE - (leftPaddleVertices[1] + leftPaddleVertices[3]) * PADDLE_ANGLE_STRIKE;
		}
	}
	else if (BallPaddleCollision(rightPaddleVertices, ballVertices, false)) {
		if (xVelocity > 0.0f) {
			// Send the ball heading toward the left with a little extra speed.
			xVelocity = -1.4f * fabs(xVelocity);
			// The Y velocity should be set proportionally to the difference from the ball's center Y to the paddle's center Y.
			yVelocity = (ballVertices[1] + ballVertices[3]) * PADDLE_ANGLE_STRIKE - (rightPaddleVertices[1] + rightPaddleVertices[3]) * PADDLE_ANGLE_STRIKE;
		}
	}
	// Check whether the ball has hit the left or right wall.
	else if (ballVertices[0] <= -ORTHO_X_BOUND && xVelocity < 0.0f || ballVertices[4] >= ORTHO_X_BOUND && xVelocity > 0.0f) {
		// One of the players scored.
		// According to the assignment, we don't need to check which player scored.
		// Reset the position of the ball.
		ResetBallPosition(ballVertices);
		// Flip the ball's X velocity and reduce its speed.
		xVelocity *= -0.8f;
		// Set the Y velocity to 0.
		yVelocity = 0.0f;
		return true;
	}
	// Slow the ball down slightly.
	else if (xVelocity > 0) {
		xVelocity -= BALL_DRAG * xVelocity * xVelocity * millisecondsElapsed;
	}
	else {
		xVelocity += BALL_DRAG * xVelocity * xVelocity * millisecondsElapsed;
	}
	// Check whether the ball has hit the top or bottom wall.
	if (ballVertices[1] >= ORTHO_Y_BOUND && yVelocity > 0.0f || ballVertices[3] <= -ORTHO_Y_BOUND && yVelocity < 0.0f) {
		// Bounce the ball with a small loss in momentum.
		yVelocity *= -0.95f;
	}
	// Update the position of the ball.
	MoveVertices(ballVertices, 0, 12, millisecondsElapsed * xVelocity);
	MoveVertices(ballVertices, 1, 12, millisecondsElapsed * yVelocity);
	return false;
}

int main(int argc, char *argv[])
{
	srand(time(NULL));
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Pong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
	#ifdef _WINDOWS
		glewInit();
	#endif

	glViewport(0, 0, WIDTH, HEIGHT);
	SetRandomBackgroundColor();
	ShaderProgram program(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
	Matrix projectionMatrix;
	projectionMatrix.SetOrthoProjection(-ORTHO_X_BOUND, ORTHO_X_BOUND, -ORTHO_Y_BOUND, ORTHO_Y_BOUND, -1.0f, 1.0f);
	Matrix modelviewMatrix;
	glUseProgram(program.programID);
	program.SetProjectionMatrix(projectionMatrix);
	program.SetModelviewMatrix(modelviewMatrix);

	float vertices[] = {
		// Left paddle
		-ORTHO_X_BOUND,  0.3,                // Left top
		-ORTHO_X_BOUND, -0.3,                // Left bottom
		-ORTHO_X_BOUND + PADDLE_WIDTH, 0.3,  // Right top
		-ORTHO_X_BOUND + PADDLE_WIDTH, -0.3, // Right bottom
		-ORTHO_X_BOUND + PADDLE_WIDTH, 0.3,  // Right top
		-ORTHO_X_BOUND, -0.3,                // Left bottom
		// Right paddle
		ORTHO_X_BOUND - PADDLE_WIDTH, 0.3,   // Left top
		ORTHO_X_BOUND - PADDLE_WIDTH, -0.3,  // Left bottom
		ORTHO_X_BOUND, 0.3,                  // Right top
		ORTHO_X_BOUND, -0.3,                 // Right bottom
		ORTHO_X_BOUND, 0.3,                  // Right top
		ORTHO_X_BOUND - PADDLE_WIDTH, -0.3,  // Left bottom
		// Ball
		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
	};
	ResetBallPosition(vertices + 24);
	Uint32 lastFrameTick = 0;
	float leftPaddleVelocity = 0.0, rightPaddleVelocity = 0.0, ballXVelocity = 0.002, ballYVelocity = 0.0;

	SDL_Event event;
	bool done = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
			case SDL_WINDOWEVENT_CLOSE:
				done = true;
				break;
			}
		}
		const Uint8* keys = SDL_GetKeyboardState(NULL);
		// Get time since last frame
		Uint32 thisFrameTick = SDL_GetTicks(), millisecondsElapsed = thisFrameTick - lastFrameTick;
		lastFrameTick = thisFrameTick;
		// Left paddle: W to move up, S to move down
		MovePaddle(vertices, leftPaddleVelocity, millisecondsElapsed, keys[SDL_SCANCODE_W], keys[SDL_SCANCODE_S]);
		// Right paddle: up arrow to move up, down arrow to move down
		MovePaddle(vertices + 12, rightPaddleVelocity, millisecondsElapsed, keys[SDL_SCANCODE_UP], keys[SDL_SCANCODE_DOWN]);
		// Ball collision
		if (MoveBall(vertices, vertices + 12, vertices + 24, ballXVelocity, ballYVelocity, millisecondsElapsed)) {
			// If a player scored, change the background color.
			SetRandomBackgroundColor();
		}
		// Clear screen
		glClear(GL_COLOR_BUFFER_BIT);
		// Draw triangles
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program.positionAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 18);
		glDisableVertexAttribArray(program.positionAttribute);
		// Send to screen
		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}
