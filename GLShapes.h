#pragma once
#include "stdafx.h"

class GLShapes
{
public:
	glm::vec3 rotate_theta{ 0.0f, 0.0f, 0.0f }; // 회전값 x y z
	glm::vec3 revolve_theta{ 0.0f, 0.0f, 0.0f }; // 회전값 x y z
	glm::vec3 pos{ 0.0f, 0.0f, 0.0f }; // 객체의 중점
	glm::vec3 scale{ 1.0f, 1.0f, 1.0f }; // 크기만 확대축소용
	glm::vec3 Oscale{ 1.0f, 1.0f, 1.0f }; // 원점에 대해 확대축소용
	glm::mat4 World_mat;
	glm::mat4 View_mat;
	glm::vec3 midpos{ 0.f, 0.f, 0.f };
	GLuint v_color{};

	GLShapes() {}
	GLShapes(glm::vec3 m) : pos(m) { }
	virtual ~GLShapes() {};

	void Update();
};

