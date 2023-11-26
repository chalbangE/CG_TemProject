
#include "stdafx.h"
#include "GLShapes.h"

void GLShapes::Update() {
	World_mat = glm::mat4(1.0);

	World_mat = glm::scale(World_mat, Oscale);

	World_mat = glm::rotate(World_mat, glm::radians(revolve_theta.x), glm::vec3(1.0f, 0.0f, 0.0f));
	World_mat = glm::rotate(World_mat, glm::radians(revolve_theta.y), glm::vec3(0.0f, 1.0f, 0.0f));
	World_mat = glm::rotate(World_mat, glm::radians(revolve_theta.z), glm::vec3(0.0f, 0.0f, 1.0f));

	World_mat = glm::translate(World_mat, pos);
	
	World_mat = glm::translate(World_mat, midpos);

	World_mat = glm::rotate(World_mat, glm::radians(rotate_theta.x), glm::vec3(1.0f, 0.0f, 0.0f));
	World_mat = glm::rotate(World_mat, glm::radians(rotate_theta.y), glm::vec3(0.0f, 1.0f, 0.0f));
	World_mat = glm::rotate(World_mat, glm::radians(rotate_theta.z), glm::vec3(0.0f, 0.0f, 1.0f));

	World_mat = glm::scale(World_mat, scale);

	World_mat = glm::translate(World_mat, -midpos);
}