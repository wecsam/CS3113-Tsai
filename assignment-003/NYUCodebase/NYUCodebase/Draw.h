#pragma once
#include "ShaderProgram.h"
#define CHARACTERS_WIDTH 40.0f // pixels
#define CHARACTERS_HEIGHT 80.0f // pixels
#define CHARACTERS_BASELINE 58.89f // pixels from top of line
const Matrix IDENTITY_MATRIX;
extern ShaderProgram* program;

void DrawTrianglesWithTexture(const Matrix& ModelviewMatrix, GLsizei numTriangles, const float* vertices, const float* texCoords, GLuint textureID);
bool DrawText(GLuint charactersT, const std::string& text, float baselineStartPixelX, float baselineStartPixelY, float scale = 1.0f);
