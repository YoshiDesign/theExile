#pragma once
#include "olcPixelGameEngine.h"

const int SPRITE_LEN_X = 64;
const int SPRITE_LEN_Y = 64;
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

// Player's offset from the ctr of the screen
float PLAYER_OFFSET_X = 0.15f;
float PLAYER_OFFSET_Y = 0.15f;

const float PLAYER_X_MAX_TOP = 11.909f;
const float PLAYER_X_MAX_BOTTOM = -220.1002f;
const float PLAYER_X_MIN = -170.0f;
const float PLAYER_Y_MIN = 18.0f;
const float PLAYER_Y_MAX = 250.0f;

const float PLAYER_MAX_SPEED = 1000.0f;
const float PLAYER_MIN_SPEED = 500.0f;

// Controls the rate of player Y per unit of VSPEED_X
const float SPEED_DIVISOR = 580.0f;

static const int WORLD_HEIGHT = (int)(SCREEN_HEIGHT / 32);
static const int WORLD_WIDTH  = (int)(SCREEN_WIDTH / 8);
const int MAP_LEFT_EDGE = 0;
const int MAP_TOP_EDGE = -133;
int WORLD_SHIFT = 1058;

int WORLD_SCALE = 85;
//int WORLD_SCALE = 120;
int PLAYER_SCALE = 17;
//int PLAYER_SCALE = 15;

// Dimensions of each tile in spritesheet
olc::vi2d vTileSize = { SPRITE_LEN_X, SPRITE_LEN_Y };


enum Face
{
	Floor = 0,
	North = 1,
	East = 2,
	South = 3,
	West = 4,
	Top = 5
};

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
	olc::vi2d tile;

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
