#pragma once
#include "stdafx.h"

class GLCamera
{
public:
	glm::vec3 rotate_theta{ 0.0f, 0.0f, 0.0f }; // 회전값 x y z
	glm::vec3 revolve_theta{ 0.0f, 0.0f, 0.0f }; // 회전값 x y z
	glm::vec3 pos{ 0.0f, 0.0f, 0.0f };
	glm::mat4 Camera_Mat;

	void Update();
	void Top_Update();
	void draw_prepare(int Location, std::string Location_str);
};

