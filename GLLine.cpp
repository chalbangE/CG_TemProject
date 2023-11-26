#include "GLLine.h"

GLLine::GLLine(glm::vec3 m) : GLShapes(m) {
	glm::vec3 color[6]{
		{1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f},
		{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
		{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f},
	};

	glGenBuffers(1, &v_color);
	glBindBuffer(GL_ARRAY_BUFFER, v_color);
	glBufferData(GL_ARRAY_BUFFER, sizeof(color), color, GL_STATIC_DRAW);
}

void GLLine::draw() {
	glDrawArrays(GL_LINES, 0, 6);
}

void GLLine::drawMouse() {
	glDrawArrays(GL_LINES, 0, 2);
}

void GLLine::draw_prepare(int Location, std::string Location_str) {
	if ("Pos" == Location_str) {
		glBindBuffer(GL_ARRAY_BUFFER, V_pos_Line);
		glVertexAttribPointer(Location, 3, GL_FLOAT, GL_FALSE, 0, 0);
	}
	else if ("Color" == Location_str) {
		glBindBuffer(GL_ARRAY_BUFFER, v_color);
		glVertexAttribPointer(Location, 3, GL_FLOAT, GL_FALSE, 0, 0);
	}
	else if ("World" == Location_str) {
		glUniformMatrix4fv(Location, 1, GL_FALSE, glm::value_ptr(World_mat));
	}
	//else if ("Tex" == str) {
	//	glBindBuffer(GL_ARRAY_BUFFER, m_TexVBO);
	//	glVertexAttribPointer(Loc, 2, GL_FLOAT, GL_FALSE, 0, 0);
	//}
}

void GLLine::draw_prepareMouse(int Location, std::string Location_str) {
	if ("Pos" == Location_str) {
		glBindBuffer(GL_ARRAY_BUFFER, v_pos);
		glVertexAttribPointer(Location, 3, GL_FLOAT, GL_FALSE, 0, 0);
	}
	else if ("Color" == Location_str) {
		glBindBuffer(GL_ARRAY_BUFFER, v_color);
		glVertexAttribPointer(Location, 3, GL_FLOAT, GL_FALSE, 0, 0);
	}
	else if ("World" == Location_str) {
		glUniformMatrix4fv(Location, 1, GL_FALSE, glm::value_ptr(World_mat));
	}
}