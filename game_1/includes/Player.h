#pragma once
#include "olcPixelGameEngine.h"
#include "Helpers.h"
#include "World.h"
#include <string>



class Player {


private:

	olc::vf2d screenPosition;
	olc::vf2d worldPosition;
	

public:
	Renderable p1;
	olc::vi2d vTileSize = { SPRITE_LEN_X, SPRITE_LEN_Y };
	olc::vf2d location = { 5,0 };
	olc::vi2d tile = {};
	float max_speed;
	float max_accel;
	float max_speed_delta;


	enum Aframes {

	};
	
	Player() {
		
	};

	void Load(const std::string& sFile) {
		std::cout << "LOADING PLAYER!" << std::endl;
		p1.Load(sFile);
	}

	void Update(int time, int ev = 0)
	{
		tile = { (int) ceil(time * .1) % 6, ev };
	}

	~Player() {}

};