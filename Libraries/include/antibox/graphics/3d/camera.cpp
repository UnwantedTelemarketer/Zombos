#include "camera.h"

using namespace antibox;

Camera::Camera(int width, int height, glm::vec3 position)
{
	Camera::width = width;
	Camera::height = height;
	pos = position;
}

void Camera::Matrix(float FOVdeg, float nearPlane, float farPlane, Shader* shader, const char* uniform) {
	glm::mat4 view = glm::mat4(1.f);
	glm::mat4 projection = glm::mat4(1.f);

	view = glm::lookAt(pos, pos + orientation, up);
	projection = glm::perspective(glm::radians(FOVdeg), (float)width / height, nearPlane, farPlane);

	shader->SetUniformMat4(uniform, projection * view);
}

void Camera::MoveCamera(int dir) {
	switch (dir) {
	case 0:
		pos += speed * orientation;
		break;
	case 1:
		pos += speed * -glm::normalize(glm::cross(orientation, up));
		break;
	case 2:
		pos += speed * -orientation;
		break;
	case 3:
		pos += speed * glm::normalize(glm::cross(orientation, up));
		break;
	case 4:
		pos += speed * up;
		break;
	case 5:
		pos += speed * -up;
		break;
	}
}