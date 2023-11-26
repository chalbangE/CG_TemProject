#pragma once
#include "stdafx.h"
#include "GLShapes.h"
class GLLine :
    public GLShapes
{
public:
	GLuint v_pos;
	GLLine() {};
	GLLine(glm::vec3 m);
	virtual ~GLLine() {};

	void draw();
	void drawMouse();

	void draw_prepare(int Location, std::string Location_str);
	void draw_prepareMouse(int Location, std::string Location_str);
};

