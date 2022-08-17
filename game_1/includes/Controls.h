#pragma once
#include "Helpers.h"
#include "Player.h"
#include "Camera.h"
#include "olcPixelGameEngine.h"

class Controls : public olc::PixelGameEngine {
public:
	void FlightControl(float fElapsedTime, int type, Player& player)
	{

		float _PLAYER_Y_MAX = PLAYER_Y_MAX + player.altitude;
		float _PLAYER_Y_MIN = PLAYER_Y_MIN - player.altitude;

		// Keep in mind both are negative values
		player.ascending = player.altitude < player.pAltitude;

		switch (type)
		{


		case 2:

			if (GetKey(olc::Key::RIGHT).bHeld) {


				


			}

			//
			if (GetKey(olc::Key::LEFT).bHeld) {

				
			}

			if (GetKey(olc::Key::UP).bHeld && player.posY >= _PLAYER_Y_MIN) {


				


			}
			else if (player.posY <= _PLAYER_Y_MIN)
			{
				player.posX = PLAYER_X_MAX_TOP;
				player.posY = _PLAYER_Y_MIN + 0.1f; // +0.1 prevents clipping
				DY = 0;
				DX = 0;
			}

			if (GetKey(olc::Key::DOWN).bHeld && player.posY <= _PLAYER_Y_MAX) {

				
			}
			else if (player.posY >= _PLAYER_Y_MAX)
			{
				player.posX = PLAYER_X_MAX_BOTTOM;
				player.posY = _PLAYER_Y_MAX - 0.1f; // -0.1 prevents clipping
				DX = 0;
				DY = 0;
			}

			// Climb
			if (GetKey(olc::Key::W).bHeld && player.altitude > CRUISE_ALTITUDE)
			{
				
			}
			else if (DT < 0.0f)
			{
				DT += 0.04f * fElapsedTime;
			}
			// Prevent thrust clipping
			else if (DT > 0.0f)
			{
				DT = 0.0f;
			}

			// Return to ground
			if (player.altitude < 0) {

				// Apply gravity accel
				player.altitude += (gravity * fElapsedTime);

				// Player is descending
				if (!player.ascending) {

					// Check for new altitude record
					player.highest_altitude = player.pAltitude < player.highest_altitude ? player.pAltitude : player.highest_altitude;

				}
				else
				{
					player.LastMaxAltitude = player.altitude;
				}
			}

			

			movePlayer(player.pAltitude, player);

		default:
			break;
		}

	}

	void movePlayer(float& previous_altitude, Player& player)
	{

		// Increase or decrease our lower & upper world boudaries, respectively, to account for altitude
		PLAYER_Y_MIN = PLAYER_STATIC_Y_MIN + player.altitude;
		PLAYER_Y_MAX = PLAYER_STATIC_Y_MAX - player.altitude;

		// Determine vector deltas
		DT = DT < 1 && DT > -V_MAX ? DT : DT < -V_MAX ? -V_MAX : DT > 1 ? V_MAX : DT = DT;
		DY = DY < 1 && DY > -1 ? DY : DY < -1 ? -1 : DY > 1 ? 1 : DY;
		DX = DX < 1 && DX > -1 ? DX : DX < -1 ? -1 : DX > 1 ? 1 : DX;

		// Apply vector deltas
		player.posY += DY;
		player.posX += DX;
		previous_altitude = player.altitude;
		player.altitude += DT;

	}

};
