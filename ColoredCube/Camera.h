/*
Camera Class by Andrew Cunningham
*/

#ifndef _CAMERA_H                // Prevent multiple definitions if this 
#define _CAMERA_H 

#include "constants.h"
#include "d3dUtil.h"
#include "d3dApp.h"
#include <d3dx9math.h>
#include "input.h"
#include "GameStateManager.h"

class Camera
{
public:
	Camera();
	~Camera();
	void init(Vector3 position, Vector3 direction, Vector3 _lookAt);
	Matrix getViewMatrix() {return mView;}
	Matrix getProjectionMatrix() {return mProj;}
	void update(float dt);
	void setPosition(Vector3 pos) {position = pos;}
	Vector3 getPosition() {return position;}
	void setDirection(Vector3 dir) {direction = dir;}
	Vector3 getDirection() {return direction;}
	float getSpeed () {return speed;}
	void setSpeed(float s) {speed = s;}
	float getFoV() {return FoV;}
	void setFoV(float fov){FoV = fov;}
	void setPerspective();
	void setPitch(float p) {pitch = p;}
	void setCameraMoveLeft (bool b) {canCameraMoveLeft = b;}
	void setCameraMoveRight (bool b) {canCameraMoveRight = b;}
	void setGsm(GameStateManager* gs) {gsm = gs;}
	void restart();
	void cameraShake(float dt);
	void stopCameraShake() {cameraShaking = false;}
	bool isCameraShaking() {return cameraShaking;}
private:
	Matrix mView;
	Matrix mProj;
	Vector3 position;
	Vector3 direction;
	float speed;
	float aspectRatio;
	float FoV;
	float nearClippingPlane;
	float farClippingPlane;
	Vector3 lookAt;
	Vector3 up;
	Vector3 right;
	float yaw;
	float pitch;
	float roll;
	Vector3 maxLookAt;
	Vector3 minLookAt;

	bool cameraShaking;
	Vector3 oldPos;
	Vector3 oldLookAt;
	float shakeTimer;
	bool canCameraMoveLeft;
	bool canCameraMoveRight;
	GameStateManager* gsm;
};
#endif
