#pragma once
#include "stdafx.h"

class GLRay
{
public:
	glm::vec3 origin;     // 광선의 시작점
	glm::vec3 direction;  // 광선의 방향

	void ScreenToWorld(int x, int y, const glm::mat4& View_mat, const glm::mat4& Projection_mat, int Viewport_Width, int Viewport_Height);
};

