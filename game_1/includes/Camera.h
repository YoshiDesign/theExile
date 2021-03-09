#pragma once
#include "olcPixelGameEngine.h"
#include "Helpers.h"

olc::vf2d vCameraPos;
float fCameraAngle;					// Allows rotation of the world
float fCameraAngleTarget;
float fCameraPitch;					// Rotation in X axis
float fCameraZoom;
const float CAMERA_PITCH = 12.361f;
const float CAMERA_ANGLE = -0.29f;
const float CAMERA_ZOOM = 12.0f;
const olc::vf2d cameraPos = { 0.0f, 0.0f };

void resetCamera() {
	olc::vf2d vCameraPos = cameraPos;
	fCameraAngle = CAMERA_ANGLE;					// Allows rotation of the world
	fCameraAngleTarget = fCameraAngle;
	fCameraPitch = CAMERA_PITCH;					// Rotation in X axis
	fCameraZoom = CAMERA_ZOOM;
}
