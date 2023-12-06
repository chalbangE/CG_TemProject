#include "GLRay.h"

void GLRay::ScreenToWorld(int x, int y, const glm::mat4& View_mat, const glm::mat4& Projection_mat, int Viewport_Width, int Viewport_Height) {
	// Unproject를 통해 스크린 좌표를 월드 좌표로 변환
	glm::vec3 winCoord(x, Viewport_Height - y, 0.0f);
	origin = glm::unProject(winCoord, View_mat, Projection_mat, glm::vec4(0, 0, Viewport_Width, Viewport_Height));

	winCoord.z = 1.0f;
	direction = glm::unProject(winCoord, View_mat, Projection_mat, glm::vec4(0, 0, Viewport_Width, Viewport_Height));

	// 방향 벡터 계산
	origin += glm::vec3{ 0.f, -0.1f, -0.1f };
	direction = glm::normalize(direction - origin);
}
