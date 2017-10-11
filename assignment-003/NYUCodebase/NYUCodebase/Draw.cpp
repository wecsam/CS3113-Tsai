#include "Draw.h"
#include "Dimensions.h"
#include "Rectangle.h"
#define CHARACTERS_SHEET_WIDTH 640.0f // pixels
#define CHARACTERS_SHEET_HEIGHT 1120.0f // pixels
ShaderProgram* program;
GLuint fontGrid;

void DrawTrianglesWithTexture(const Matrix& ModelviewMatrix, GLsizei numTriangles, const float* vertices, const float* texCoords, GLuint textureID) {
	program->SetModelviewMatrix(ModelviewMatrix);
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program->positionAttribute);
	glEnableVertexAttribArray(program->texCoordAttribute);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glDrawArrays(GL_TRIANGLES, 0, 3 * numTriangles);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

bool DrawText(const std::string& text, float baselineStartPixelX, float baselineStartPixelY, float scale) {
	// Dynamically allocate arrays of floats in memory for the vertices and texture coordinates.
	size_t s = 12 * sizeof(float) * text.size();
	float* vertices = (float*)malloc(s);
	float* texCoords = (float*)malloc(s);
	if (!(vertices && texCoords)) {
		return false;
	}
	// Add each character to the arrays.
	float top = PIXEL_FROM_TOP_TO_ORTHO((baselineStartPixelY - CHARACTERS_BASELINE * scale));
	for (size_t i = 0; i < text.size(); ++i) {
		// Set the vertices.
		float left = PIXEL_FROM_LEFT_TO_ORTHO((baselineStartPixelX + CHARACTERS_WIDTH * i * scale));
		Rectangle::SetBox(vertices + i * 12, top, left + PIXEL_TO_ORTHO_X((CHARACTERS_WIDTH * scale)), top - PIXEL_TO_ORTHO_Y((CHARACTERS_HEIGHT * scale)), left);
		// Make sure that the character is in range.
		char c = text[i];
		if (c < 0x20 || c > 0xff) {
			c = 0x20;
		}
		// Set the texture coordinates.
		div_t gridPosition = div(c, 16);
		Rectangle::SetBox(
			texCoords + i * 12,
			CHARACTERS_HEIGHT / CHARACTERS_SHEET_HEIGHT * (gridPosition.quot - 2),
			CHARACTERS_WIDTH / CHARACTERS_SHEET_WIDTH * (gridPosition.rem + 1),
			CHARACTERS_HEIGHT / CHARACTERS_SHEET_HEIGHT * (gridPosition.quot - 1),
			CHARACTERS_WIDTH / CHARACTERS_SHEET_WIDTH * gridPosition.rem
		);
	}
	// Draw the arrays.
	DrawTrianglesWithTexture(IDENTITY_MATRIX, 2 * text.size(), vertices, texCoords, fontGrid);
	// Free the allocated arrays.
	free(vertices);
	free(texCoords);
	return true;
}
