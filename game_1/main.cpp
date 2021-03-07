/*
	Dungeon Warping via Orthographic Projections
	"For my Mother-In-Law, you will be missed..." - javidx9

	License (OLC-3)
	~~~~~~~~~~~~~~~

	Copyright 2018-2020 OneLoneCoder.com

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions
	are met:

	1. Redistributions or derivations of source code must retain the above
	copyright notice, this list of conditions and the following disclaimer.

	2. Redistributions or derivative works in binary form must reproduce
	the above copyright notice. This list of conditions and the following
	disclaimer must be reproduced in the documentation and/or other
	materials provided with the distribution.

	3. Neither the name of the copyright holder nor the names of its
	contributors may be used to endorse or promote products derived
	from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
	"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
	A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
	HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
	SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
	LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
	THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	Relevant Video: https://youtu.be/Ql5VZGkL23o

	Links
	~~~~~
	YouTube:	https://www.youtube.com/javidx9
				https://www.youtube.com/javidx9extra
	Discord:	https://discord.gg/WhwHUMV
	Twitter:	https://www.twitter.com/javidx9
	Twitch:		https://www.twitch.tv/javidx9
	GitHub:		https://www.github.com/onelonecoder
	Patreon:	https://www.patreon.com/javidx9
	Homepage:	https://www.onelonecoder.com

	Community Blog: https://community.onelonecoder.com

	Author
	~~~~~~
	David Barr, aka javidx9, ©OneLoneCoder 2018, 2019, 2020
*/

#define OLC_PGE_APPLICATION
#include "includes/olcPixelGameEngine.h"

#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480

/*

	NOTE! This program requires a tile spritesheet NOT
	provided in this github. You only need a few tiles,
	see video for details.

*/

class olcDungeon : public olc::PixelGameEngine
{

	const float CAMERA_PITCH = 12.12f;
	const float CAMERA_ANGLE = 0.0f;
	const int VSPEED_X = 100;

public:
	olcDungeon()
	{
		sAppName = "YTE";
	}

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


	// A container of quads which need to be drawn to the screen
	struct sQuad
	{
		// A quad is defined by these 4 points in world-space (x,y,z)
		vec3d points[4];
		// The texture coordinate for this quad
		olc::vf2d tile;

		// I added this
		bool wall = false;

		// TODO - add which wall this is so we can draw around its base accordingly

	};

	// Represents a cell
	// Wall flag denotes its existence
	struct sCell
	{
		bool wall = false;

		// ID represents each face of a cube in the world
		olc::vi2d id[6]{  };  // e.g. { {0,198}, {0,198}, {0,198}, {0,198}, {0,198}, {0,198} } means each face corresponds to {0,9} on the spritesheet, where each cell is 32x32 pixels
	};

	/*
		Round
		fundamentally a 2D array of sCells
	*/
	class World
	{
	public:
		World()
		{

		}

		// Initializes the size std::vector and resizes
		void Create(int w, int h)
		{
			size = { w, h };

			// vCells is now a vector of sCells
			vCells.resize(w * h);
		}

		// Address a coordinate which exists in the world
		sCell& GetCell(const olc::vi2d& v)
		{
			if (v.x >= 0 && v.x < size.x && v.y >= 0 && v.y < size.y)
				// Extraction of a sCell from vCells
				return vCells[v.y * size.x + v.x];
			else
				return NullCell;
		}

	public:
		olc::vi2d size;

	private:
		// The cells of the world
		std::vector<sCell> vCells;

		// Returned by functions in the event we try to access cells outside of the world
		sCell NullCell;

	};//EndofWorld

	World world;
	Renderable rendSelect;
	Renderable rendAllWalls;

	// Camera parameters
	olc::vf2d vCameraPos;
	float fCameraAngle;					// Allows rotation of the world
	float fCameraAngleTarget;
	float fCameraPitch;					// Rotation in X axis
	float fCameraZoom;
	vec3d vSpace;						// Initialized to the camera's initial vector. Used to move the world in relation to the camera

	bool bVisible[6];

	olc::vf2d vCursor = { 0.0f, 0.0f };
	olc::vf2d vTileCursor = { 9.0f,0.0f };

	// Dimensions of each tile in spritesheet
	olc::vi2d vTileSize = { 32, 32 };

	enum Face
	{
		Floor = 0,
		North = 1,
		East = 2,
		South = 3,
		West = 4,
		Top = 5
	};

public:
	bool OnUserCreate() override
	{
		// The initial position of the World
		vSpace = { vCameraPos.x, 0.0f, vCameraPos.y };

		// The initial position of the camera in the world		
		vCameraPos = { vCursor.x + 0.5f, vCursor.y + 0.5f };
		vCameraPos *= fCameraZoom;

		rendSelect.Load("./gfx/dng_select.png");
		rendAllWalls.Load("./gfx/oldDungeon.png");

		world.Create((int) SCREEN_WIDTH / 8, (int) SCREEN_HEIGHT / 8);

		// World initialization
		for (int y = 0; y < world.size.y; y++)
			for (int x = 0; x < world.size.x; x++)
			{
				// Nothing is a wall from the start
				world.GetCell({ x, y }).wall = false;

				// Assign each face a tile from the spritesheet
				world.GetCell({ x, y }).id[Face::Floor] = olc::vi2d{ 0, 9 } * vTileSize; // Calculates the position of the upper left corner on the spritesheet in pixels
				world.GetCell({ x, y }).id[Face::Top]   = olc::vi2d{ 0, 9 } * vTileSize;
				world.GetCell({ x, y }).id[Face::North] = olc::vi2d{ 9, 0 } * vTileSize;
				world.GetCell({ x, y }).id[Face::South] = olc::vi2d{ 0, 9 } * vTileSize;
				world.GetCell({ x, y }).id[Face::West]  = olc::vi2d{ 0, 9 } * vTileSize;
				world.GetCell({ x, y }).id[Face::East]  = olc::vi2d{ 0, 9 } * vTileSize;

			}

		resetCamera();

		return true;
	}

	/*
		Every tile in the game world can be represented as a cube.
		This function generates vertices in the right places for us

		These parameters parameterize the cube's orthographic calculations
		@return 8 3d vectors
		@arg vCell	- What tile this is from within our world
		@arg fAngle	- The angle of ratation of the world
		@arg fPitch	- Angle of pitch
		@arg fScale	- Zoom
		@arg vCamera - 3D location of the camera
	*/
	std::array<vec3d, 8> CreateCube(const olc::vi2d& vCell, const float fAngle, const float fPitch, const float fScale, const vec3d& vSpace)
	{
		// Many cubes
		std::array<vec3d, 8> unitCube, rotCube, worldCube, projCube;

		// Unit Cube - No transformation, set at the origin
		unitCube[0] = { 0.0f, 0.0f, 0.0f };
		unitCube[1] = { fScale, 0.0f, 0.0f };
		unitCube[2] = { fScale, -fScale, 0.0f };
		unitCube[3] = { 0.0f, -fScale, 0.0f };
		unitCube[4] = { 0.0f, 0.0f, fScale };
		unitCube[5] = { fScale, 0.0f, fScale };
		unitCube[6] = { fScale, -fScale, fScale };
		unitCube[7] = { 0.0f, -fScale, fScale };

		// Translate Cube in X-Z Plane
		for (int i = 0; i < 8; i++)
		{
			unitCube[i].x += (vCell.x * fScale - vSpace.x);
			unitCube[i].y += -vSpace.y;
			unitCube[i].z += (vCell.y * fScale - vSpace.z);
		}

		// Rotate Cube in Y-Axis around origin
		float s = sin(fAngle);
		float c = cos(fAngle);
		for (int i = 0; i < 8; i++)
		{
			// 3D pitch rotation
			rotCube[i].x = unitCube[i].x * c + unitCube[i].z * s;
			rotCube[i].y = unitCube[i].y;
			rotCube[i].z = unitCube[i].x * -s + unitCube[i].z * c;
		}

		// Rotate Cube in X-Axis around origin (tilt slighly overhead)
		s = sin(fPitch);
		c = cos(fPitch);
		for (int i = 0; i < 8; i++)
		{
			// 3D matrix rotation around X axis
			worldCube[i].x = rotCube[i].x;
			worldCube[i].y = rotCube[i].y * c - rotCube[i].z * s;
			worldCube[i].z = rotCube[i].y * s + rotCube[i].z * c;
		}

		// Project Cube Orthographically - Unit Cube Viewport
		//float fLeft = -ScreenWidth() * 0.5f;
		//float fRight = ScreenWidth() * 0.5f;
		//float fTop = ScreenHeight() * 0.5f;
		//float fBottom = -ScreenHeight() * 0.5f;
		//float fNear = 0.1f;
		//float fFar = 100.0f;*/
		//for (int i = 0; i < 8; i++)
		//{
		//	projCube[i].x = (2.0f / (fRight - fLeft)) * worldCube[i].x - ((fRight + fLeft) / (fRight - fLeft));
		//	projCube[i].y = (2.0f / (fTop - fBottom)) * worldCube[i].y - ((fTop + fBottom) / (fTop - fBottom));
		//	projCube[i].z = (2.0f / (fFar - fNear)) * worldCube[i].z - ((fFar + fNear) / (fFar - fNear));
		//  projCube[i].x *= -fRight;
		//  projCube[i].y *= -fTop;
		//  projCube[i].x += fRight;
		//  projCube[i].y += fTop;
		//}

		// Project Cube Orthographically - Full Screen Centered - Optimization of the commented out code above, but assumes full screen space is visible
		// Use the above to project to a smaller viewport - Great for submenus
		for (int i = 0; i < 8; i++)
		{
			// Ensures that the camera's focal point is in the middle of the screen
			projCube[i].x = worldCube[i].x + ScreenWidth() * 0.5f;
			projCube[i].y = worldCube[i].y + ScreenHeight() * 0.5f;
			projCube[i].z = worldCube[i].z;
		}

		// We now know where everything is in screen-space
		return projCube;
	}



	void CalculateVisibleFaces(std::array<vec3d, 8>& cube)
	{
		// Tells us which faces face us or are facing away
		auto CheckNormal = [&](int v1, int v2, int v3)
		{
			olc::vf2d a = { cube[v1].x, cube[v1].y };
			olc::vf2d b = { cube[v2].x, cube[v2].y };
			olc::vf2d c = { cube[v3].x, cube[v3].y };
			return  (b - a).cross(c - a) > 0;
		};

		bVisible[Face::Floor] = CheckNormal(4, 0, 1);
		bVisible[Face::South] = CheckNormal(3, 0, 1);
		bVisible[Face::North] = CheckNormal(6, 5, 4);
		bVisible[Face::East] = CheckNormal(7, 4, 0);
		bVisible[Face::West] = CheckNormal(2, 1, 5);
		bVisible[Face::Top] = CheckNormal(7, 3, 2);
	}

	/*
		@function
		Get the quads of a cube at location (cell)
		Takes in the cell's X and Y, camera information, and will append
		to a vector (&render) any quads which represent a particular cube (at vCell)
	*/
	void GetFaceQuads(
		const olc::vi2d& vCell, const float fAngle, const float fPitch, 
		const float fScale, const vec3d& vSpace, std::vector<sQuad> &render
	)
	{
		// Essentially 8 3d vectors generated where we want them to be
		std::array<vec3d, 8> projCube = CreateCube(vCell, fAngle, fPitch, fScale, vSpace);

		auto& cell = world.GetCell(vCell);

		// Defines the faces of the cube. Each int is an ID of which vertex we want to acknowledge. Face comes from enum.
		// The projCube pushed onto our render vector will now contain those vertices in screen-space
		auto MakeFace = [&](int v1, int v2, int v3, int v4, Face f, bool isWall)
		{
			// id[f] is one of the 6 faces, determined at the end of this function (these, 4, vertices, are, thisFaceQuad)
			render.push_back({ projCube[v1], projCube[v2], projCube[v3], projCube[v4], cell.id[f], isWall });
		};

		// Drawing a floor. Anything with a surface normal pointing away from the camera 
		// will not be added to vector of things to be drawn
		if (!cell.wall)
		{
			if (bVisible[Face::Floor]) MakeFace(4, 0, 1, 5, Face::Floor, false);
		}
		// Drawing a wall. We can't see the floor. Walls are solid. 5 visible faces
		else
		{
			if (bVisible[Face::South]) MakeFace(3, 0, 1, 2, Face::South, true);
			if (bVisible[Face::North]) MakeFace(6, 5, 4, 7, Face::North, true);
			if (bVisible[Face::East]) MakeFace(7, 4, 0, 3, Face::East, true);
			if (bVisible[Face::West]) MakeFace(2, 1, 5, 6, Face::West, true);
			if (bVisible[Face::Top]) MakeFace(7, 3, 2, 6, Face::Top, true);
		}
	}

	/*
		@function
		Only prepare the corners of the world
	*/
	void GetCornerQuads(const float fAngle, const float fPitch,const float fScale, const vec3d& vCamera, std::vector<sQuad> &render)
	{
		// Coordinates of each of our world's corner cells
		olc::vi2d corners[4] = { 
			{0,0},
			{world.size.x-1,0},
			{0,world.size.y-1},
			{world.size.x-1, world.size.y-1} 
		};

		// Lambda - Once we've calculated our cube's parameters, push them onto the array of stuff to render
		auto MakeFace = [&](auto scell, std::array<vec3d, 8> pCube, int v1, int v2, int v3, int v4, Face f)
		{
			render.push_back({ pCube[v1], pCube[v2], pCube[v3], pCube[v4], scell.id[f] });
		};

		for (auto& c : corners) {

			// Get the cell - The location of this corner in world-space
			auto& cell = world.GetCell(c);

			// Create a cube (calculate vertices), passing in the cell so that it knows where it is will be drawn
			std::array<vec3d, 8> projCube = CreateCube(c, fAngle, fPitch, fScale, vCamera);

			// Determine what is to be rendered
			if (!cell.wall)
			{
				if (bVisible[Face::Floor]) MakeFace(cell, projCube, 4, 0, 1, 5, Face::Floor);
			}
			// Drawing a wall. We can't see the floor. Walls are solid. 5 visible faces
			else
			{
				if (bVisible[Face::South]) MakeFace(cell, projCube, 3, 0, 1, 2, Face::South);
				if (bVisible[Face::North]) MakeFace(cell, projCube, 6, 5, 4, 7, Face::North);
				if (bVisible[Face::East]) MakeFace(cell, projCube, 7, 4, 0, 3, Face::East);
				if (bVisible[Face::West]) MakeFace(cell, projCube, 2, 1, 5, 6, Face::West);
				if (bVisible[Face::Top]) MakeFace(cell, projCube, 7, 3, 2, 6, Face::Top);
			}

		}
		
	}

	/*
		@function
	*/
	void GetSidesQuads(const float fAngle, const float fPitch, const float fScale, const vec3d& vCamera, std::vector<sQuad> &render)
	{
		auto MakeFace = [&](auto scell, std::array<vec3d, 8> pCube, int v1, int v2, int v3, int v4, Face f)
		{
			render.push_back({ pCube[v1], pCube[v2], pCube[v3], pCube[v4], scell.id[f] });
		};

		// If not a wall / object / enemy - Return. We won't render anything there for now
		for (size_t y = 0; y < world.size.y; y++)
			for (size_t x = 0; x < world.size.x; x++)
			{
				if (y == 0 || x == 0 || y == world.size.y || x == world.size.x)
				{
					
				}
			}
	}

	void resetCamera() {
		olc::vf2d vCameraPos = { 0.0f, 0.0f };
		fCameraAngle = CAMERA_ANGLE;					// Allows rotation of the world
		fCameraAngleTarget = fCameraAngle;
		fCameraPitch = CAMERA_PITCH;					// Rotation in X axis
		fCameraZoom = 16.0f;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		// Grab mouse for convenience
		olc::vi2d vMouse = { GetMouseX(), GetMouseY() };

		// Edit mode - Selection from tile sprite sheet
		if (GetKey(olc::Key::TAB).bHeld)
		{
			DrawSprite({ 0, 0 }, rendAllWalls.sprite);
			DrawRect(vTileCursor * vTileSize, vTileSize);
			if (GetMouse(0).bPressed) vTileCursor = vMouse / vTileSize;
			return true;
		}

		// WS keys to tilt camera
		if (GetKey(olc::Key::W).bHeld) fCameraPitch += 1.0f * fElapsedTime;
		if (GetKey(olc::Key::S).bHeld) fCameraPitch -= 1.0f * fElapsedTime;

		// DA Keys to manually rotate camera
		if (GetKey(olc::Key::D).bHeld) fCameraAngleTarget += 1.0f * fElapsedTime;
		if (GetKey(olc::Key::A).bHeld) fCameraAngleTarget -= 1.0f * fElapsedTime;

		// QZ Keys to zoom in or out
		if (GetKey(olc::Key::Q).bHeld) fCameraZoom += 5.0f * fElapsedTime;
		if (GetKey(olc::Key::Z).bHeld) fCameraZoom -= 5.0f * fElapsedTime;

		if (GetKey(olc::Key::R).bPressed) 
			resetCamera();

		// Numpad keys used to rotate camera to fixed angles
		if (GetKey(olc::Key::P).bPressed) fCameraAngleTarget = 3.14159f * 0.0f;
		if (GetKey(olc::Key::O).bPressed) fCameraAngleTarget = 3.14159f * 0.25f;
		if (GetKey(olc::Key::I).bPressed) fCameraAngleTarget = 3.14159f * 0.5f;
		if (GetKey(olc::Key::U).bPressed) fCameraAngleTarget = 3.14159f * 0.75f;
		if (GetKey(olc::Key::Y).bPressed) fCameraAngleTarget = 3.14159f * 1.0f;
		if (GetKey(olc::Key::T).bPressed) fCameraAngleTarget = 3.14159f * 1.25f;
		if (GetKey(olc::Key::L).bPressed) fCameraAngleTarget = 3.14159f * 1.75f;

		// Numeric keys apply selected tile to specific face
		if (GetKey(olc::Key::K1).bPressed) world.GetCell(vCursor).id[Face::North] = vTileCursor * vTileSize;
		if (GetKey(olc::Key::K2).bPressed) world.GetCell(vCursor).id[Face::East] = vTileCursor * vTileSize;
		if (GetKey(olc::Key::K3).bPressed) world.GetCell(vCursor).id[Face::South] = vTileCursor * vTileSize;
		if (GetKey(olc::Key::K4).bPressed) world.GetCell(vCursor).id[Face::West] = vTileCursor * vTileSize;
		if (GetKey(olc::Key::K5).bPressed) world.GetCell(vCursor).id[Face::Floor] = vTileCursor * vTileSize;
		if (GetKey(olc::Key::K6).bPressed) world.GetCell(vCursor).id[Face::Top] = vTileCursor * vTileSize;

		// Smooth camera
		fCameraAngle += (fCameraAngleTarget - fCameraAngle) * 10.0f * fElapsedTime;

		// Arrow keys to move the selection cursor around map (boundary checked)
		// if (GetKey(olc::Key::LEFT).bPressed) vCursor.x--;
		if (GetKey(olc::Key::LEFT).bHeld) vCursor.x -= 2.0f * fElapsedTime;
		// if (GetKey(olc::Key::RIGHT).bPressed) vCursor.x++;
		if (GetKey(olc::Key::RIGHT).bHeld) vCursor.x += 2.0f * fElapsedTime;
		// if (GetKey(olc::Key::UP).bPressed) vCursor.y--;
		if (GetKey(olc::Key::UP).bHeld) vCursor.y -= 2.0f * fElapsedTime;
		// if (GetKey(olc::Key::DOWN).bPressed) vCursor.y++;
		if (GetKey(olc::Key::DOWN).bHeld) vCursor.y += 2.0f * fElapsedTime;
		if (vCursor.x < 0) vCursor.x = 0;
		if (vCursor.y < 0) vCursor.y = 0;
		if (vCursor.x >= world.size.x) vCursor.x = world.size.x - 1;
		if (vCursor.y >= world.size.y) vCursor.y = world.size.y - 1;

		// Place block with space
		if (GetKey(olc::Key::SPACE).bPressed)
		{
			world.GetCell(vCursor).wall = !world.GetCell(vCursor).wall;
		}

		// Rendering

		vSpace.x += VSPEED_X * fElapsedTime;
		DrawStringDecal({ 0,48 }, "vSpace.x: " + std::to_string(vSpace.x));
		//vSpace.y += 1.0 * fElapsedTime;
		//vSpace.z += 1.0 * fElapsedTime;

		/*
			1) Create dummy cube to extract visible face information
		*/

		// Cull faces that cannot be seen
		std::array<vec3d, 8> cullCube = CreateCube({ 0, 0 }, fCameraAngle, fCameraPitch, fCameraZoom, vSpace);
		// Sets the booleans determining which faces we can see
		CalculateVisibleFaces(cullCube);

		/*
			2) Get all visible sides of all visible "tile cubes"
		*/ 

		// A container filled with the things we're going to draw
		std::vector<sQuad> vQuads;

		// Non optimized. We're iterating over the whole world and redrawing it all the time.
		for (int y = 0; y < world.size.y; y++)
			for (int x = 0; x < world.size.x; x++) {

				// Each cell consists of 6 quads which are the faces of a cube. vQuads will contain any quads which make up that particular cube at location {x, y}
				GetFaceQuads({ x, y }, fCameraAngle, fCameraPitch, fCameraZoom, vSpace, vQuads);


				//DrawLine(q.points[0].x, q.points[0].y, q.points[3].x, q.points[3].y);
			}

		// GetCornerQuads(fCameraAngle, fCameraPitch, fCameraZoom, { vCameraPos.x, 0.0f, vCameraPos.y }, vQuads);

		/*
			3) Sort in order of depth, from farthest away to closest
		*/
		std::sort(vQuads.begin(), vQuads.end(), [](const sQuad& q1, const sQuad& q2)
		{
			float z1 = (q1.points[0].z + q1.points[1].z + q1.points[2].z + q1.points[3].z) * 0.25f;
			float z2 = (q2.points[0].z + q2.points[1].z + q2.points[2].z + q2.points[3].z) * 0.25f;
			return z1 < z2;
		});

		/*
			4) Iterate through all "tile cubes" and draw their visible faces
		*/

		Clear(olc::BLACK);

		// Iterate through the vector of quads that we want to draw
		// Remember that an sQuad is just a face on a cube, composed of 4 3d points in world-space

		for (auto& q : vQuads) {

			// PGE function. Takes screen-space quad coordinates and texture coordinates, and draws them appropriately
	/*		DrawPartialWarpedDecal
			(
				rendAllWalls.decal,
				{ {q.points[0].x, q.points[0].y}, {q.points[1].x, q.points[1].y}, {q.points[2].x, q.points[2].y}, {q.points[3].x, q.points[3].y} },
				q.tile,
				vTileSize
			);*/
			
			if (!q.wall) {
				DrawLine(q.points[0].x, q.points[0].y, q.points[1].x, q.points[1].y);
				DrawLine(q.points[1].x, q.points[1].y, q.points[2].x, q.points[2].y);
			}

		}


		/*
			5) Draw current tile selection in corner
		*/
		DrawPartialDecal({ 10,10 }, rendAllWalls.decal, vTileCursor * vTileSize, vTileSize);
		

		/*
			6) Draw selection "tile cube"	
		*/ 
		vQuads.clear();
		GetFaceQuads(vCursor, fCameraAngle, fCameraPitch, fCameraZoom, { vCameraPos.x, 0.0f, vCameraPos.y }, vQuads);
		for (auto& q : vQuads) {
			DrawWarpedDecal(rendSelect.decal, { {q.points[0].x, q.points[0].y}, {q.points[1].x, q.points[1].y}, {q.points[2].x, q.points[2].y}, {q.points[3].x, q.points[3].y} });
		}

		

		/*
		
			7) Draw some debug info
		
		*/
		DrawStringDecal({ 0,0 }, "Cursor: " + std::to_string(vCursor.x) + ", " + std::to_string(vCursor.y), olc::YELLOW, { 0.5f, 0.5f });
		DrawStringDecal({ 0,8 }, "Angle: " + std::to_string(fCameraAngle) + ", " + std::to_string(fCameraPitch), olc::YELLOW, { 0.5f, 0.5f });
		DrawStringDecal({ 0,16 }, "Time: " + std::to_string(fElapsedTime * 1000));

		// Graceful exit if user is in full screen mode
		return !GetKey(olc::Key::ESCAPE).bPressed;
	}
};

int main()
{
	olcDungeon demo;
	if (demo.Construct(SCREEN_WIDTH, SCREEN_HEIGHT, 2, 2, false))
		demo.Start();
	return 0;
}