#pragma once
#include "Defines.h"
#include "math/Vector.h"
#include "Shader.h"
#include <map>
#include <ft2build.h>
#include FT_FREETYPE_H  

using TextRendererPtr = shared_ptr<class TextRenderer>;
class TextRenderer {
public:
	TextRenderer(int width, int height);

	void RenderText(std::string text, GLfloat x, GLfloat y, GLfloat scale, Vector3 color);

	void OnResize(int width, int height) {
		mWidth = width;
		mHeight = height;
	}
private:
	ShaderPtr mShader;
	GLuint VAO, VBO;
	int mWidth, mHeight;
	struct Character {
		GLuint TextureID;   // ID handle of the glyph texture
		unsigned int Sizex,Sizey;    // Size of glyph
		unsigned int Bearingx, Bearingy;  // Offset from baseline to left/top of glyph
		GLuint Advance;    // Horizontal offset to advance to next glyph
	};

	std::map<GLchar, Character> Characters;
};

