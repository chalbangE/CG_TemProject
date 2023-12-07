#define _CRT_SECURE_NO_WARNINGS //--- 프로그램 맨 앞에 선언할 것
#pragma once

#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <glm.hpp>
#include <ext.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <string>
#include <sstream>

#include "stb_image.h"
#pragma comment (lib, "fmod_vc.lib")

static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_real_distribution<GLclampf> colorRd(0.0f, 1.0f);

extern GLuint V_pos_Line;