#include "GLUi.h"

GLUi::GLUi() {}

GLUi::GLUi(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
	const glm::vec3 pos[] = {
			{ x1, y1, 0.f }, { x2, y1, 0.f }, { x1, y2, 0.f },
			{ x1, y2, 0.f }, { x2, y1, 0.f }, { x2, y2, 0.f }
	};
	const glm::vec2 Tex[] = {
		{ 0.0, 0.0 }, { 1.0, 0.0 }, { 0.0, 1.0 },
		{ 0.0, 1.0 }, { 1.0, 0.0 }, { 1.0, 1.0 }
	};

	glGenBuffers(1, &v_pos);
	glBindBuffer(GL_ARRAY_BUFFER, v_pos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pos), pos, GL_STATIC_DRAW);

	glGenBuffers(1, &v_uv);
	glBindBuffer(GL_ARRAY_BUFFER, v_uv);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Tex), Tex, GL_STATIC_DRAW);

	leftbottom = pos[0];
	righttop = pos[5];
}
GLUi::GLUi(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2, GLfloat z)
{
	const glm::vec3 pos[] = {
			{ x1, y1, z }, { x2, y1, z }, { x1, y2, z },
			{ x1, y2, z }, { x2, y1, z }, { x2, y2, z }
	};
	const glm::vec2 Tex[] = {
		{ 0.0, 0.0 }, { 1.0, 0.0 }, { 0.0, 1.0 },
		{ 0.0, 1.0 }, { 1.0, 0.0 }, { 1.0, 1.0 }
	};

	glGenBuffers(1, &v_pos);
	glBindBuffer(GL_ARRAY_BUFFER, v_pos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pos), pos, GL_STATIC_DRAW);

	glGenBuffers(1, &v_uv);
	glBindBuffer(GL_ARRAY_BUFFER, v_uv);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Tex), Tex, GL_STATIC_DRAW);

	leftbottom = pos[0];
	righttop = pos[5];
}

void GLUi::imgLoad(std::string map)
{
	int img_W, img_H, numberOfChannel; // 가로, 세로, 채널 수

	glGenTextures(1, &img[0]);
	glBindTexture(GL_TEXTURE_2D, img[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(map.c_str(), &img_W, &img_H, &numberOfChannel, 0);
	// std::cout << name << " : widthImage - " << widthImage << " , heightImage - " << heightImage << std::endl;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img_W, img_H, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);
}

void GLUi::imgLoad(std::string map, std::string map2)
{
	int img_W, img_H, numberOfChannel; // 가로, 세로, 채널 수

	{
		glGenTextures(1, &img[0]);
		glBindTexture(GL_TEXTURE_2D, img[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		stbi_set_flip_vertically_on_load(true);
		unsigned char* data = stbi_load(map.c_str(), &img_W, &img_H, &numberOfChannel, 0);
		// std::cout << name << " : widthImage - " << widthImage << " , heightImage - " << heightImage << std::endl;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img_W, img_H, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		stbi_image_free(data);
	}
	{
		glGenTextures(1, &img[1]);
		glBindTexture(GL_TEXTURE_2D, img[1]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		stbi_set_flip_vertically_on_load(true);
		unsigned char* data = stbi_load(map2.c_str(), &img_W, &img_H, &numberOfChannel, 0);
		// std::cout << name << " : widthImage - " << widthImage << " , heightImage - " << heightImage << std::endl;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img_W, img_H, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		stbi_image_free(data);
	}
}

void GLUi::draw(std::string draw_Mod) {
	if ("solid" == draw_Mod)
		glDrawArrays(GL_TRIANGLES, 0, 6);
}

void GLUi::draw_prepare(int Location, std::string Location_str) {
	if ("Pos" == Location_str) {
		glBindBuffer(GL_ARRAY_BUFFER, v_pos);
		glVertexAttribPointer(Location, 3, GL_FLOAT, GL_FALSE, 0, 0);
	}
	else if ("UV" == Location_str) {
		glBindBuffer(GL_ARRAY_BUFFER, v_uv);
		glVertexAttribPointer(Location, 2, GL_FLOAT, GL_FALSE, 0, 0);
	}
	else if ("UI_bool" == Location_str) {
		glUniform1i(Location, true);
	}
	else if ("Texture" == Location_str) {
		glBindTexture(GL_TEXTURE_2D, img[Location]);
	}
	else if ("World" == Location_str) {
		glUniformMatrix4fv(Location, 1, GL_FALSE, glm::value_ptr(World_mat));
	}
}

void GLUi::Update()
{
	World_mat = glm::mat4(1.0);
	World_mat = glm::translate(World_mat, pos);
	World_mat = glm::scale(World_mat, scale);
}