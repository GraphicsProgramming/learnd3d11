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
    DirectX::XMVECTOR position = DirectX::XMLoadFloat3(&_position);
    DirectX::XMVECTOR direction = DirectX::XMLoadFloat3(&_direction);
    DirectX::XMVECTOR up = DirectX::XMLoadFloat3(&_up);

    DirectX::XMVECTOR lookAt = DirectX::XMVectorAdd(position, direction);

    DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtRH(position, lookAt, up);

    DirectX::XMStoreFloat4x4(&_viewMatrix, viewMatrix);
}

void Camera::UpdateVectors()
{
    constexpr DirectX::XMFLOAT3 unitY = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);

    float pitchSine = 0.0f;
    float pitchCosine = 0.0f;
    float yawSine = 0.0f;
    float yawCosine = 0.0f;

    DirectX::XMScalarSinCos(&pitchSine, &pitchCosine, _pitch);
    DirectX::XMScalarSinCos(&yawSine, &yawCosine, _yaw);

    DirectX::XMFLOAT3 direction = DirectX::XMFLOAT3(
        pitchCosine * yawCosine,
        pitchSine,
        pitchCosine * yawSine);

    DirectX::XMVECTOR directionVector = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&direction));
    DirectX::XMVECTOR right = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(directionVector, DirectX::XMLoadFloat3(&unitY)));
    DirectX::XMVECTOR up = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(right, directionVector));

    DirectX::XMStoreFloat3(&_direction, directionVector);
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

void Camera::Move(const float& speed)
{
    DirectX::XMVECTOR scaled = DirectX::XMVectorScale(DirectX::XMLoadFloat3(&_direction), speed);
    DirectX::XMVECTOR advancedPosition = DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&_position), scaled);

    DirectX::XMStoreFloat3(&_position, advancedPosition);
}

void Camera::Slide(const float& speed)
{
    DirectX::XMVECTOR scaled = DirectX::XMVectorScale(DirectX::XMLoadFloat3(&_right), speed);
    DirectX::XMVECTOR advancedPosition = DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&_position), scaled);

    DirectX::XMStoreFloat3(&_position, advancedPosition);
}

void Camera::Lift(const float& speed)
{
    DirectX::XMVECTOR scaled = DirectX::XMVectorScale(DirectX::XMLoadFloat3(&_up), speed);
    DirectX::XMVECTOR advancedPosition = DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&_position), scaled);

    DirectX::XMStoreFloat3(&_position, advancedPosition);

}

void Camera::AddYaw(const float yawInDegrees)
{
    _yaw += DirectX::XMConvertToRadians(yawInDegrees);
}

void Camera::AddPitch(float pitchInDegrees)
{
    if (pitchInDegrees >= 89.0f)
    {
        pitchInDegrees = 89.0f;
    }
    if (pitchInDegrees <= -89.0f)
    {
        pitchInDegrees = -89.0f;
    }

    _pitch -= DirectX::XMConvertToRadians(pitchInDegrees);
}

PerspectiveCamera::PerspectiveCamera(
    const float fieldOfViewInDegrees,
    const int32_t width,
    const int32_t height,
    const float nearPlane,
    const float farPlane) : Camera(nearPlane, farPlane)
{
    _width = static_cast<float>(width);
    _height = static_cast<float>(height);
    _fieldOfViewInRadians = DirectX::XMConvertToRadians(fieldOfViewInDegrees);
}

void PerspectiveCamera::Resize(
    const int32_t width,
    const int32_t height)
{
    _width = static_cast<float>(width);
    _height = static_cast<float>(height);

    UpdateProjectionMatrix();
}

void PerspectiveCamera::UpdateProjectionMatrix()
{
    DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovRH(
        _fieldOfViewInRadians,
        _width / static_cast<float>(_height),
        _nearPlane,
        _farPlane);
    DirectX::XMStoreFloat4x4(&_projectionMatrix, projectionMatrix);
}
