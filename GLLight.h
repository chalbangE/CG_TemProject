#pragma once
#include "stdafx.h"
#include "GLObj.h"

class GLLight :
    public GLObj
{
public:
	glm::vec3 L_color;

	void draw_prepare(int Location, std::string Location_str);
};

