#pragma once
#include "stdafx.h"

class GLUi
{
public:
    GLuint v_pos;
    GLuint v_uv;
	GLuint img;
	glm::vec3 leftbottom;
	glm::vec3 righttop;

	GLUi();
	GLUi(GLfloat x1, GLfloat x2, GLfloat y1, GLfloat y2);
	void imgLoad(std::string map);
	void draw(std::string draw_Mod);
	void draw_prepare(int Location, std::string Location_str);
};

