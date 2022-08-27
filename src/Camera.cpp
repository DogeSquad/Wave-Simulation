#include "Camera.h"

Camera::Camera(glm::vec3 cameraPos)
{
	this->cameraPos = cameraPos;
	this->cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	this->cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    this->yaw = -90.0f;
    this->pitch = 0.0f;
}

glm::mat4 Camera::getViewMatrix()
{
	return glm::lookAt(this->cameraPos, this->cameraPos + this->cameraFront, this->cameraUp);
}
void Camera::updateRotations(float xOffset, float yOffset)
{
    yaw += xOffset;
    pitch += yOffset;
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}
glm::vec3 Camera::getCameraPos()
{
    return cameraPos;
}
void Camera::setCameraPos(glm::vec3 pos)
{
    cameraPos = pos;
}
void Camera::update(GLFWwindow *window, float deltaTime)
{
    float speed = cameraSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) // Speedup
        speed += speed * 1.8f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)          // Forward
        cameraPos += speed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)          // Backward
        cameraPos -= speed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)          // Strafe Left
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)          // Strafe Right
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)      // Up
        cameraPos += speed * cameraUp;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) // Down
        cameraPos -= speed * cameraUp;
}