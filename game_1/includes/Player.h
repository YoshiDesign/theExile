#pragma once
#include "olcPixelGameEngine.h"
#include "Helpers.h"
#include "World.h"
#include "Controls.h"
#include "Phys/Physics.h"
#include "Camera.h"
#include <string>


class Player {
public:

	// Sceen Position
	float PLAYER_OFFSET_X = 0.15f;
	float PLAYER_OFFSET_Y = 0.15f;
	float RESET_PLAYER_OFFSET_Y = PLAYER_OFFSET_Y;

	// Speed
	const float SPEED_DIVISOR = 650.0f;
	float VSPEED_X = 415.0f;

	// This means that you steer harder when moving faster
	float speed = (float)(VSPEED_X / SPEED_DIVISOR) + 0.4f;

	// Player start position
	float posY = 120.0f;
	float posX = -90.0f;
	float altitude = 0.0f;
	float pAltitude = 0.0f;			// Previous altitude
	float LastMaxAltitude = 0.0f;	// Highest altitude

	// Deltas
	float DX = 0.0f;
	float DY = 0.0f;
	float DT = 0.0f;

	// Altitude
	const int CRUISE_ALTITUDE = -100; // Thrust cutoff
	const float V_THRUST = 0.1f;	 // Rate of climb
	const float V_MAX = 0.4f;		 // Max vertical thrust
	const float THRUST_WIDTH = CRUISE_ALTITUDE * 2 + CRUISE_ALTITUDE - (2 * V_THRUST * CRUISE_ALTITUDE);

private:

	olc::vf2d screenPosition;
	olc::vf2d worldPosition;

	float max_speed;
	float max_accel;
	float max_speed_delta;



	// Speed
	const int VSPEED_DELTA = 100;
	const float VSPEED_OFFSET = 0.02;
	const float PLAYER_MAX_SPEED = 1000.0f;
	const float PLAYER_MIN_SPEED = 500.0f;
	const float VTHRUST_OFFSET = 0.005;

	// Controls the rate of player Y per unit of VSPEED_X - Lower divisor means faster steering


public:

	Renderable p1;
	olc::vi2d vTileSize = { SPRITE_LEN_X, SPRITE_LEN_Y };
	olc::vf2d location = { 5,0 };
	olc::vi2d tile = {};
	
	// Offsetting constants based on location
	float _PLAYER_Y_MAX = PLAYER_Y_MAX + altitude;
	float _PLAYER_Y_MIN = PLAYER_Y_MIN - altitude;

	// Records
	float highest_altitude = 0.0f;

	// Bools
	bool ascending = false;

	Player() {

	}

	void Load(const std::string& sFile) {
		p1.Load(sFile);
	}

	void Update(float fTime, float vSpaceX, int ev = 0)
	{
		// Tiles from the spritesheet
		tile = { (int) ceil(vSpaceX * .1) % 6, ev };

		// Calculate what's happening to the player
		phys(fTime);

		// Respond to latest calculations
		movePlayer(fTime);
	}

	// Button Down
	void moveNear(float fElapsedTime)
	{
		if (posY <= _PLAYER_Y_MAX) {
			DX -= speed * fElapsedTime;
			DY += speed * fElapsedTime;
		}

	}

	// Button Up
	void moveFar(float fElapsedTime)
	{
		if (posY >= _PLAYER_Y_MIN) 
		{
			DX += speed * fElapsedTime;
			DY -= speed * fElapsedTime;
		}
	}

	// Button RIGHT
	void throttleUp(float fElapsedTime)
	{
		if (VSPEED_X <= PLAYER_MAX_SPEED) {
			VSPEED_X += VSPEED_DELTA * fElapsedTime;
			PLAYER_OFFSET_X += VSPEED_OFFSET * fElapsedTime;
		}
	}

	// Button LEFT
	void throttleDown(float fElapsedTime)
	{
		if (VSPEED_X >= PLAYER_MIN_SPEED) {
			VSPEED_X -= VSPEED_DELTA * fElapsedTime;
			PLAYER_OFFSET_X -= VSPEED_OFFSET * fElapsedTime;
		}
	}

	// Button W
	void climb(float fElapsedTime)
	{
		if (altitude > CRUISE_ALTITUDE) 
		{  
			DT -= V_THRUST * fElapsedTime;
		}

	}

	void phys(float fTime)
	{

		// Thrust attenuation
		if (DT < 0.0f)
		{
			DT += 0.04f * fTime;
		}
		else if (DT > 0.0f)
		{
			DT = 0.0f;
		}

		// Maximum near boundary
		if (posY >= _PLAYER_Y_MAX)
		{
			posX = PLAYER_X_MAX_BOTTOM;
			posY = _PLAYER_Y_MAX - 0.1f; // -0.1 prevents clipping
			DX = 0;
			DY = 0;
		}

		// Maximum far boundary
		if (posY <= _PLAYER_Y_MIN)
		{
			posX = PLAYER_X_MAX_TOP;
			posY = _PLAYER_Y_MIN + 0.1f; // +0.1 prevents clipping
			DY = 0;
			DX = 0;
		}


		// Keep in mind both are negative values
		ascending = altitude < pAltitude;

		// Return to ground
		if (altitude < 0) {

			// Apply gravity accel
			altitude += (gravity * fTime);

			// Player is descending
			if (!ascending) {

				// Check for new altitude record
				highest_altitude = pAltitude < highest_altitude ? pAltitude : highest_altitude;

			}
			else
			{
				LastMaxAltitude = altitude;
			}
		}

		// TODO Camera motion while ascending / descending
		if (ascending && altitude < CRUISE_ALTITUDE - 12)
		{
			PLAYER_OFFSET_Y += (.01f * fTime) - (0.003 * DT);
		}
		else if (!ascending && altitude < CRUISE_ALTITUDE || PLAYER_OFFSET_Y > RESET_PLAYER_OFFSET_Y) {
			PLAYER_OFFSET_Y -= .02f * fTime;
		}

		// Start moving the camera down
		//else if (!ascending && /*(-1 * LastMaxAltitude) - (-1 * altitude) > 10.0f */ altitude < CAMERA_ASCEND)
		//{
		//	PLAYER_OFFSET_Y = DT * fTime;
		//}
		//if (PLAYER_OFFSET_Y < 0)
		//{
		//	PLAYER_OFFSET_Y = RESET_PLAYER_OFFSET_Y;
		//}

	}

	void movePlayer(float fTime)
	{

		// Increase or decrease our lower or upper world boudaries, respectively, to account for altitude
		PLAYER_Y_MIN = PLAYER_STATIC_Y_MIN + altitude;
		PLAYER_Y_MAX = PLAYER_STATIC_Y_MAX - altitude;

		// Determine vector deltas
		DT = DT < 1 && DT > -V_MAX ? DT : DT < -V_MAX ? -V_MAX : DT > 1 ? V_MAX : DT;
		DY = DY < 1 && DY > -1 ? DY : DY < -1 ? -1 : DY > 1 ? 1 : DY;
		DX = DX < 1 && DX > -1 ? DX : DX < -1 ? -1 : DX > 1 ? 1 : DX;

		// Apply vector deltas
		posY += DY;
		posX += DX;
		pAltitude = altitude;
		altitude += DT;

	}

	~Player() {}

};