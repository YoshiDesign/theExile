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