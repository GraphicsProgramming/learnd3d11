#include "Camera.hpp"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

Camera::Camera(
    const float nearPlane,
    const float farPlane)
{
    _nearPlane = nearPlane;
    _farPlane = farPlane;
}

glm::mat4 Camera::GetViewMatrix()
{
    return _viewMatrix;
}

glm::mat4 Camera::GetProjectionMatrix()
{
    return _projectionMatrix;
}

CameraConstants& Camera::GetCameraConstants()
{
    return _cameraConstants;
}

void Camera::Update()
{
    UpdateVectors();
    UpdateProjectionMatrix();
    UpdateViewMatrix();

    _cameraConstants.ProjectionMatrix = _projectionMatrix;
    _cameraConstants.ViewMatrix = _viewMatrix;
}

void Camera::UpdateViewMatrix()
{
    _viewMatrix = glm::lookAtRH(_position, _position + _direction, _up);
}

void Camera::UpdateVectors()
{
    float x = glm::cos(_pitch) * glm::cos(_yaw);
    float y = glm::sin(_pitch);
    float z = glm::cos(_pitch) * glm::sin(_yaw);

    constexpr glm::vec3 unitY = glm::vec3(0.0f, 1.0f, 0.0f);

    _direction = glm::normalize(glm::vec3(x, y, z));
    _right = glm::normalize(glm::cross(_direction, unitY));
    _up = glm::normalize(glm::cross(_right, _direction));
}

void Camera::SetPosition(const glm::vec3& position)
{
    _position = position;
}

void Camera::SetDirection(const glm::vec3& direction)
{
    _direction = _direction;
}

void Camera::SetUp(const glm::vec3& up)
{
    _up = _up;
}

void Camera::Move(const float& speed)
{
    _position += _direction * speed;
}

void Camera::Slide(const float& speed)
{
    _position += _right * speed;
}

void Camera::Lift(const float& speed)
{
    _position += _up * speed;
}

void Camera::AddYaw(const float yawInDegrees)
{
    _yaw += glm::radians(yawInDegrees);
}

void Camera::AddPitch(const float pitchInDegrees)
{
    float pitch = glm::clamp(pitchInDegrees, -89.0f, 89.0f);
    _pitch -= glm::radians(pitch);
}

PerspectiveCamera::PerspectiveCamera(
    const float fieldOfViewInDegrees,
    const int32_t width,
    const int32_t height,
    const float nearPlane,
    const float farPlane) : Camera(nearPlane, farPlane)
{
    _width = width;
    _height = height;
    _fieldOfViewInRadians = glm::radians(fieldOfViewInDegrees);
}

void PerspectiveCamera::Resize(
    const int32_t width,
    const int32_t height)
{
    _width = width;
    _height = height;

    UpdateProjectionMatrix();
}

void PerspectiveCamera::UpdateProjectionMatrix()
{
    _projectionMatrix = glm::perspectiveFovRH(
        _fieldOfViewInRadians,
        _width,
        _height,
        _nearPlane,
        _farPlane);
}
