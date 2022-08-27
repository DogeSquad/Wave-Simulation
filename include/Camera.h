#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

class Camera
{
public:
	Camera(glm::vec3 cameraPos);
	float cameraSpeed = 2.5f;
	glm::mat4 getViewMatrix();
	glm::vec3 getCameraPos();
	void setCameraPos(glm::vec3 pos);
	void update(GLFWwindow *window, float deltaTime);
	void updateRotations(float yaw, float pitch);
private:
	glm::vec3 cameraPos;
	glm::vec3 cameraFront;
	glm::vec3 cameraUp;
	float yaw, pitch;
};