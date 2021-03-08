#pragma once
#include "olcPixelGameEngine.h"

const float PLAYER_OFFSET_X = 0.25f;
const float PLAYER_OFFSET_Y = 0.25f;


	// A small class used to instantiate a new sprite with a decal
	struct Renderable
	{
		Renderable() {}

		void Load(const std::string& sFile)
		{
			sprite = new olc::Sprite(sFile);
			decal = new olc::Decal(sprite);
		}

		~Renderable()
		{
			delete decal;
			delete sprite;
		}

		olc::Sprite* sprite = nullptr;
		olc::Decal* decal = nullptr;
	};

	struct vec3d
	{
		float x, y, z;
	};

	/*
		A container of quads which need to be drawn to the screen.
		Each sQuad undergoes calculation to acquire its new points after warping
	*/
	struct sQuad
	{
		// A quad is defined by these 4 points in screen-space. Each point is {x,y,z}
		vec3d points[4];

		// The texture coordinate for this quad
		olc::vf2d tile;

		bool gravEnabled = false;

		bool isPlayer = false;

		// TODO - add which wall this is so we can draw around its base accordingly

	};

	/*
		Represents a cell.
		This does not tell us where it is on the screen
	*/
	struct sCell
	{
		bool wall = false;
		bool player = false;
		// ID represents each face of a cube in the world
		olc::vi2d id[6]{  };  // e.g. { {0,198}, {0,198}, {0,198}, {0,198}, {0,198}, {0,198} } means each face corresponds to {0,9} on the spritesheet, where each cell is 32x32 pixels
	};
