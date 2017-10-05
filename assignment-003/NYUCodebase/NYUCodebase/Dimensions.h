#pragma once
#define WIDTH 600
#define HEIGHT 800
#define ORTHO_X_BOUND 3.0f
#define ORTHO_Y_BOUND 4.0f
#define PIXEL_FROM_TOP_TO_ORTHO(y) (ORTHO_Y_BOUND * (1.0f - 2.0f * y / HEIGHT))
#define PIXEL_FROM_BOTTOM_TO_ORTHO(y) (ORTHO_Y_BOUND * (2.0f * y / HEIGHT - 1.0f))
#define PIXEL_FROM_LEFT_TO_ORTHO(x) (ORTHO_X_BOUND * (2.0f * x / WIDTH - 1.0f))
#define PIXEL_FROM_RIGHT_TO_ORTHO(x) (ORTHO_X_BOUND * (1.0f - 2.0f * x / WIDTH))
