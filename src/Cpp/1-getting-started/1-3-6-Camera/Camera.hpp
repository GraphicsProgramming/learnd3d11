#pragma once

#include <DirectXMath.h>

struct CameraConstants
{
    DirectX::XMFLOAT4X4 ProjectionMatrix;
    DirectX::XMFLOAT4X4 ViewMatrix;
};

class Camera
{
public:
    virtual void Resize(
        const int32_t width,
        const int32_t height) = 0;

    void Update();

    void SetPosition(const DirectX::XMFLOAT3& position);
    void SetDirection(const DirectX::XMFLOAT3& direction);
    void SetUp(const DirectX::XMFLOAT3& up);

    void Move(const float& speed);
    void Slide(const float& speed);
    void Lift(const float& speed);

    void AddYaw(const float yawInDegrees);
    void AddPitch(float pitchInDegrees);

    [[nodiscard]] DirectX::XMFLOAT4X4 GetViewMatrix();
    [[nodiscard]] DirectX::XMFLOAT4X4 GetProjectionMatrix();
    [[nodiscard]] CameraConstants& GetCameraConstants();

protected:
    Camera(
        const float nearPlane,
        const float farPlane);

    virtual void UpdateProjectionMatrix() = 0;

    float _nearPlane = 0.0f;
    float _farPlane = 0.0f;

    DirectX::XMFLOAT4X4 _projectionMatrix = DirectX::XMFLOAT4X4();
    DirectX::XMFLOAT4X4 _viewMatrix = DirectX::XMFLOAT4X4();
    CameraConstants _cameraConstants = {};

private:
    void UpdateVectors();
    void UpdateViewMatrix();

    DirectX::XMFLOAT3 _position = {};
    DirectX::XMFLOAT3 _direction = {};
    DirectX::XMFLOAT3 _up = {};
    DirectX::XMFLOAT3 _right = {};
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
