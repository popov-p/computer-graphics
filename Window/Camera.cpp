#include "Camera.h"

void Camera::getState() {
	XMFLOAT3 position = XMFLOAT3(
		cosf(m_theta) * cosf(m_phi),
		sinf(m_theta),
		cosf(m_theta) * sinf(m_phi));

	position.x = position.x * m_dist + m_poi.x;
	position.y = position.y * m_dist + m_poi.y;
	position.z = position.z * m_dist + m_poi.z;

	float updateTheta = m_theta + XM_PIDIV2;

	XMFLOAT3 updatePosition = XMFLOAT3(
		cosf(updateTheta) * cosf(m_phi),
		sinf(updateTheta),
		cosf(updateTheta) * sinf(m_phi));

	m_viewMatrix = XMMatrixLookAtLH(
		XMVectorSet(position.x, position.y, position.z, 0.0f),
		XMVectorSet(m_poi.x, m_poi.y, m_poi.z, 0.0f),
		XMVectorSet(updatePosition.x, updatePosition.y, updatePosition.z, 0.0f)
	);
}

void Camera::getMouseState(float dx, float dy, float wheel) {
	m_phi += dx / 100.0f;
	m_theta += dy / 100.0f;
	m_theta = min(max(m_theta, -XM_PIDIV2), XM_PIDIV2);
	m_dist -= wheel / 100.0f;
	if (m_dist < 1.0f) {
		m_dist = 1.0f;
	}
}