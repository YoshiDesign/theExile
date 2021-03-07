#pragma once
#include "olcPixelGameEngine.h"
#include "Helpers.h"
#include <string>
class Player {


private:

	Renderable vPlayer;
	olc::vf2d screenPosition;
	olc::vf2d worldPosition;

	

public:
	olc::vf2d vCursor = { 0.0f, 0.0f };
	olc::vi2d vTileCursor = { 9.0f,0.0f };
	Player() {
		
	};

	Player(std::string file, olc::vf2d pStart) {
		
		vPlayer.Load("../gfx/dng_select.png");

	}

	void update()
	{

	}

	~Player() {}

};