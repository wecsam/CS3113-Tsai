#include "Draw.h"
#include "GameEntity.h"
#include "Dimensions.h"
#define CHARACTERS_SHEET_WIDTH 640.0f // pixels
#define CHARACTERS_SHEET_HEIGHT 1120.0f // pixels

bool DrawText(ShaderProgram& program, GLuint charactersT, const std::string& text, float baselineStartPixelX, float baselineStartPixelY, float scale) {
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
		Entity::SetBox(vertices + i * 12, top, left + PIXEL_TO_ORTHO_X((CHARACTERS_WIDTH * scale)), top - PIXEL_TO_ORTHO_Y((CHARACTERS_HEIGHT * scale)), left);
		// Set the texture coordinates.
		div_t gridPosition = div(text[i], 16);
		Entity::UVWrap uv(
			CHARACTERS_WIDTH / CHARACTERS_SHEET_WIDTH * gridPosition.rem,
			CHARACTERS_HEIGHT / CHARACTERS_SHEET_HEIGHT * (gridPosition.quot - 2),
			CHARACTERS_WIDTH / CHARACTERS_SHEET_WIDTH,
			CHARACTERS_HEIGHT / CHARACTERS_SHEET_HEIGHT
		);
		uv.GetTextureCoordinates(texCoords + i * 12);
	}
	// Draw the arrays.
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program.positionAttribute);
	glEnableVertexAttribArray(program.texCoordAttribute);
	glBindTexture(GL_TEXTURE_2D, charactersT);
	glDrawArrays(GL_TRIANGLES, 0, 6 * text.size());
	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);
	// Free the allocated arrays.
	free(vertices);
	free(texCoords);
	return true;
}
