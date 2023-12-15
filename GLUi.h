#pragma once
#include "stdafx.h"

class GLUi
{
public:
    GLuint v_pos;
    GLuint v_uv;
	GLuint img[2];
	glm::vec3 leftbottom;
	glm::vec3 righttop;
	int now_img = 0;
	glm::mat4 World_mat;
	glm::vec3 scale = glm::vec3(1.f);
	glm::vec3 pos = glm::vec3(0.f);

	GLUi();
	GLUi(GLfloat x1, GLfloat x2, GLfloat y1, GLfloat y2);
	GLUi(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2, GLfloat z);
	void imgLoad(std::string map);
	void imgLoad(std::string map, std::string map2);
	void draw(std::string draw_Mod);
	void draw_prepare(int Location, std::string Location_str);
	void Update();
};

