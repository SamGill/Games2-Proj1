#include "Camera.h"

Camera::Camera()
{
	speed = 0.5;
	FoV = 0.26*PI;
	aspectRatio = 640.0f/480.0f;
	nearClippingPlane = 0.5f;
	farClippingPlane = 1000.0f;
	up = Vector3(0.0f, 1.0f, 0.0f);
	position = Vector3(10.0f,2.0f,0);
	lookAt = Vector3(0.0f, 0.0f, 0.0f);
	yaw = 0;
	roll = 0;
	pitch = 0;
	direction = Vector3(0.0f, 0.0f, 0.0f);
	maxLookAt = Vector3(0.0f, 0.0f, 2.0f);
	minLookAt = Vector3(0.0f, 0.0f, -2.0f);
	canCameraMoveLeft = true;
	canCameraMoveRight = true;
	cameraShaking = false;
	shakeTimer = 0.0f;
}

Camera::~Camera()
{
	//nothing to deallocate
}

void Camera::init(Vector3 position, Vector3 direction, Vector3 _lookAt)
{
}

void Camera::setPerspective()
{// only change if change field of view, then generate new perspective matrix
	D3DXMatrixPerspectiveFovLH(&mProj, FoV, aspectRatio, nearClippingPlane,farClippingPlane); 
}
void Camera::update(float dt)
{
	bool movement = false;
	Vector3 oldposition = getPosition();
	direction = Vector3(0.0f, 0.0f, 0.0f);

	if (gsm->getGameState() == GameStateManager::IN_GAME) {
		cameraShaking = false;
		shakeTimer = 0.0f;

		if((GetAsyncKeyState('A') & 0x8000) && !(GetAsyncKeyState('D') & 0x8000)) {
			direction.z = -1.0f;
			movement = true;
		}

		else if((GetAsyncKeyState('D') & 0x8000) && !(GetAsyncKeyState('A') & 0x8000)) {
			direction.z = 1.0;
			movement = true;
		}
		else {
			direction.z = 0.0f;
			movement = false;
		}

		if (movement)
			D3DXVec3Normalize(&direction, &direction);

		if (direction.z > 0 && !canCameraMoveRight)
			direction.z = 0;
		else if (direction.z < 0 && !canCameraMoveLeft)
			direction.z = 0;

		if (getPosition().z < -PLAYER_Z_RANGE){
			setPosition(Vector3(oldposition.x, oldposition.y, -PLAYER_Z_RANGE));
		}

		else if (getPosition().z > PLAYER_Z_RANGE){
			setPosition(Vector3(oldposition.x, oldposition.y, PLAYER_Z_RANGE));
		}

		else {
			position += direction*speed*dt;
		}
		if (lookAt.z > maxLookAt.z)
			lookAt.z = maxLookAt.z;
		else if (lookAt.z < minLookAt.z)
			lookAt.z = minLookAt.z;
		else
			lookAt += direction*speed*dt;
	}

	//Generate new matrix
    D3DXMatrixLookAtLH(&mView, &position, &lookAt, &up);
}

void Camera::restart() {
	position  = Vector3(10.0f, 2.0f, 0.0f);
	lookAt    = Vector3(0.0f, 0.0f, 0.0f);
	direction = Vector3(0.0f, 0.0f, 0.0f);
}

void Camera::cameraShake(float dt) {
	if (!cameraShaking) {
		oldPos = position;
		oldLookAt = lookAt;
	}

	cameraShaking = true;
	shakeTimer += dt;
	if (shakeTimer <= 0.05f) {
		position.z += 0.007;
		lookAt.z   += 0.007;
	}
	else if (shakeTimer <= 0.1) {
		position.z -= 0.007;
		lookAt.z   -= 0.007;
	}
	else if (shakeTimer <= 0.15) {
		position.z += 0.007;
		lookAt.z   += 0.007;
	}
	else {
		position.z = oldPos.z;
		lookAt.z   = oldLookAt.z;
		cameraShaking = false;
	}
}