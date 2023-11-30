#include "GLLight.h"

void GLLight::draw_prepare(int Location, std::string Location_str) {
	if ("Pos" == Location_str) {
		glBindBuffer(GL_ARRAY_BUFFER, v_pos);
		glVertexAttribPointer(Location, 3, GL_FLOAT, GL_FALSE, 0, 0);
	}
	else if ("Color" == Location_str) {
		glBindBuffer(GL_ARRAY_BUFFER, v_color);
		glVertexAttribPointer(Location, 3, GL_FLOAT, GL_FALSE, 0, 0);
	}
	else if ("Color_bool" == Location_str) {
		glUniform1i(Location, false);
	}
	else if ("Normal" == Location_str) {
		glBindBuffer(GL_ARRAY_BUFFER, v_nor);
		glVertexAttribPointer(Location, 3, GL_FLOAT, GL_FALSE, 0, 0);
	}
	else if ("World" == Location_str) {
		glUniformMatrix4fv(Location, 1, GL_FALSE, glm::value_ptr(World_mat));
	}
	else if ("LightPos" == Location_str) {
		glm::vec3 temp = glm::vec3(World_mat * glm::vec4(pos, 1.f));
		glUniform3f(Location, temp.x, temp.y, temp.z);
	}
	else if ("LightColor" == Location_str) {
		glUniform3f(Location, L_color.x, L_color.y, L_color.z);
	}
	else if ("UV" == Location_str) {
		glBindBuffer(GL_ARRAY_BUFFER, v_uv);
		glVertexAttribPointer(Location, 2, GL_FLOAT, GL_FALSE, 0, 0);
	}
	else if ("Texture_bool" == Location_str) {
		glUniform1i(Location, true);
	}
	else if ("Texture" == Location_str) {
		glBindTexture(GL_TEXTURE_2D, img);
	}
}
