#include "Camera.hpp"

Camera::Camera(
    const float nearPlane,
    const float farPlane)
{
    _nearPlane = nearPlane;
    _farPlane = farPlane;
}

DirectX::XMFLOAT4X4 Camera::GetViewMatrix()
{
    return _viewMatrix;
}

DirectX::XMFLOAT4X4 Camera::GetProjectionMatrix()
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
    DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookToLH(
        DirectX::XMLoadFloat3(&_position),
        DirectX::XMLoadFloat3(&_direction),
        DirectX::XMLoadFloat3(&_up));
    DirectX::XMStoreFloat4x4(&_viewMatrix, viewMatrix);
}

void Camera::UpdateVectors()
{
    float pitchSine;
    float pitchCosine;
    float yawSine;
    float yawCosine;

    DirectX::XMScalarSinCos(&pitchSine, &pitchCosine, _pitch);
    DirectX::XMScalarSinCos(&yawSine, &yawCosine, _yaw);

    float x = pitchCosine * yawCosine;
    float y = pitchSine;
    float z = pitchCosine * yawSine;

    DirectX::XMFLOAT3 rotation = DirectX::XMFLOAT3(x, y, z);
    DirectX::XMFLOAT3 unitY = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
    DirectX::XMVECTOR unitYVector = DirectX::XMLoadFloat3(&unitY);

    DirectX::XMVECTOR direction = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&rotation));
    DirectX::XMVECTOR right = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(direction, unitYVector));
    DirectX::XMVECTOR up = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(right, direction));

    DirectX::XMStoreFloat3(&_direction, direction);
    DirectX::XMStoreFloat3(&_right, right);
    DirectX::XMStoreFloat3(&_up, up);
}

void Camera::SetPosition(const DirectX::XMFLOAT3& position)
{
    _position = position;
}

void Camera::SetDirection(const DirectX::XMFLOAT3& direction)
{
    _direction = _direction;
}

void Camera::SetUp(const DirectX::XMFLOAT3& up)
{
    _up = _up;
}

PerspectiveCamera::PerspectiveCamera(
    const float fieldOfView,
    const int32_t width,
    const int32_t height,
    const float nearPlane,
    const float farPlane) : Camera(nearPlane, farPlane)
{
    _aspectRatio = static_cast<float>(width) / static_cast<float>(height);
    _fieldOfView = DirectX::XMConvertToRadians(fieldOfView);
}

void PerspectiveCamera::Resize(
    const int32_t width,
    const int32_t height)
{
    _aspectRatio = static_cast<float>(width) / static_cast<float>(height);

    UpdateProjectionMatrix();
}

void PerspectiveCamera::UpdateProjectionMatrix()
{
    DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(
        _fieldOfView,
        _aspectRatio,
        _nearPlane,
        _farPlane);
    DirectX::XMStoreFloat4x4(&_projectionMatrix, projectionMatrix);
}
