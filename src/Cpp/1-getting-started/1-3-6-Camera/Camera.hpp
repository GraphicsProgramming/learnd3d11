#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

struct CameraConstants
{
    glm::mat4 ProjectionMatrix;
    glm::mat4 ViewMatrix;
};

class Camera
{
public:
    virtual void Resize(
        const int32_t width,
        const int32_t height) = 0;

    void Update();

    void SetPosition(const glm::vec3& position);
    void SetDirection(const glm::vec3& direction);
    void SetUp(const glm::vec3& up);

    void Move(const float& speed);
    void Slide(const float& speed);
    void Lift(const float& speed);

    void AddYaw(const float yawInDegrees);
    void AddPitch(const float pitchInDegrees);

    [[nodiscard]] glm::mat4 GetViewMatrix();
    [[nodiscard]] glm::mat4 GetProjectionMatrix();
    [[nodiscard]] CameraConstants& GetCameraConstants();

protected:
    Camera(
        const float nearPlane,
        const float farPlane);

    virtual void UpdateProjectionMatrix() = 0;

    float _nearPlane = 0.0f;
    float _farPlane = 0.0f;

    glm::mat4 _projectionMatrix = glm::mat4(1.0f);
    glm::mat4 _viewMatrix = glm::mat4(1.0f);
    CameraConstants _cameraConstants = {};

private:
    void UpdateVectors();
    void UpdateViewMatrix();

    glm::vec3 _position = {};
    glm::vec3 _direction = {};
    glm::vec3 _up = {};
    glm::vec3 _right = {};
    float _pitch = 0.0f;
    float _yaw = -90.0f;
};

class PerspectiveCamera final : public Camera
{
public:
    PerspectiveCamera(
        const float fieldOfViewInDegrees,
        const int32_t width,
        const int32_t height,
        const float nearPlane,
        const float farPlane);

    void Resize(
        const int32_t width,
        const int32_t height) override;

protected:
    void UpdateProjectionMatrix() override;

private:
    float _fieldOfViewInRadians = 0.0f;
    float _width = 0.0f;
    float _height = 0.0f;
};
