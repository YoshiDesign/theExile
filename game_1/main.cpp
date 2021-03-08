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
#include "includes/Player.h"
#include "includes/Helpers.h"
#include "includes/Camera.h"

#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480
#define SPRITE_LEN_X 32
#define SPRITE_LEN_Y 32

/*

	NOTE! This program requires a tile spritesheet NOT
	provided in this github. You only need a few tiles,
	see video for details.

*/

class olcDungeon : public olc::PixelGameEngine
{
	const int WORLD_HEIGHT = SCREEN_HEIGHT / 8;
	const int WORLD_WIDTH = SCREEN_WIDTH / 8;
	const int MAP_LEFT_EDGE = 0;
	const int MAP_TOP_EDGE = -133;

	const int VSPEED_X = 125;

	// Player's offset from the cursor
	float pDeltaY = -100.0f;
	float gravity = 9.0f;
	float thrust = 16.0f;

public:
	olcDungeon()
	{
		sAppName = "YTE";
	}

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

		void update()
		{
		
			
		
		}

	public:
		olc::vi2d size;
		int distance;

	private:
		// The cells of the world
		std::vector<sCell> vCells;

		// Returned by functions in the event we try to access cells outside of the world
		sCell NullCell;

	};//EndofWorld

	Player player;
	World world;
	Renderable rendSelect;
	Renderable rendAllWalls;

	vec3d vSpace;	// Initialized to the camera's initial vector. Used to move the world in relation to the camera

	bool bVisible[6];

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

public:
	bool OnUserCreate() override
	{
		// The initial position of the World
		vSpace = { vCameraPos.x, 0.0f, vCameraPos.y };
		std::cout << "Camera: (" << vCameraPos.x << ", " << vCameraPos.y << ")";

		// The initial position of the camera in the world		
		vCameraPos = { player.vCursor.x + 0.5f, player.vCursor.y + 0.5f };
		vCameraPos *= fCameraZoom;

		rendSelect.Load("./gfx/dng_select.png");
		rendAllWalls.Load("./gfx/oldDungeon.png");

		world.Create((int) WORLD_WIDTH , (int) WORLD_HEIGHT);

		// World initialization
		for (int y = 0; y < world.size.y; y++)
			for (int x = 0; x < world.size.x; x++)
			{
				// Nothing is a wall from the start
				world.GetCell({ x, y }).wall = false;

				// Assign each face a tile from the spritesheet
				world.GetCell({ x, y }).id[Face::Floor] = olc::vi2d{ 0, 0 } * vTileSize; // Calculates the position of the upper left corner on the spritesheet in pixels
				world.GetCell({ x, y }).id[Face::Top]   = olc::vi2d{ 0, 0 } * vTileSize;
				world.GetCell({ x, y }).id[Face::North] = olc::vi2d{ 0, 0 } * vTileSize;
				world.GetCell({ x, y }).id[Face::South] = olc::vi2d{ 0, 0 } * vTileSize;
				world.GetCell({ x, y }).id[Face::West]  = olc::vi2d{ 0, 0 } * vTileSize;
				world.GetCell({ x, y }).id[Face::East]  = olc::vi2d{ 0, 0 } * vTileSize;

			}

		resetCamera();

		return true;
	}


	std::array<vec3d, 8> CreateCube(const olc::vi2d& vCell, const float fAngle,
		const float fPitch, const float fScale, const vec3d& vSpace)
	{
		// Many cubes
		std::array<vec3d, 8> unitCube, rotCube, worldCube, projCube;

		// Unit Cube - No transformation, set at the origin
		// 3. This unit cube, after just translation on the X-Z plane (below)
		//    will result in each of its points being adapted to screen space.
		//    These first 8 assignments create a normalized cube based on our current zoom level
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

		// We could return the unitCube here, and it would be properly allocated to screen
		// space, albeit in 2 dimension with no orthog

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
			projCube[i].x = worldCube[i].x + ScreenWidth() * PLAYER_OFFSET_X;
			projCube[i].y = worldCube[i].y + ScreenHeight() * PLAYER_OFFSET_Y;
			projCube[i].z = worldCube[i].z;
		}

		// We now know where everything is in screen-space
		return projCube;
	}

	/*
		@function CalculateVisibleFaces
	*/
	void CalculateVisibleFaces(std::array<vec3d, 8>& cube)
	{
		/*
		
			Currently disfunctional. Always returns the same results
		
		*/

		// Tells us which faces face us or are facing away
		//auto CheckNormal = [&](int v1, int v2, int v3)
		//{
		//	olc::vf2d a = { cube[v1].x, cube[v1].y };
		//	olc::vf2d b = { cube[v2].x, cube[v2].y };
		//	olc::vf2d c = { cube[v3].x, cube[v3].y };
		//	return  (b - a).cross(c - a) > 0;
		//};

		// The perspective is locked here. Use CheckNormal to free it up
		bVisible[Face::Floor] = true;	// CheckNormal(4, 0, 1);
		bVisible[Face::North] = true;   // CheckNormal(6, 5, 4);
		bVisible[Face::South] = false;  // CheckNormal(3, 0, 1);
		bVisible[Face::East]  = false;	// CheckNormal(7, 4, 0);
		bVisible[Face::West]  = false;	// CheckNormal(2, 1, 5);
		bVisible[Face::Top]   = false;	// CheckNormal(7, 3, 2);
	}

	/*
		@function
		Get the quads of a cube at location (cell)
		Takes in the cell's X and Y, camera information, and will append
		to a vector (&render) any quads which represent a particular cube (at vCell)
	*/
	void GetWorldQuads(
		const olc::vi2d& vCell, const float fAngle, const float fPitch, 
		const float fScale, const vec3d& vSpace, std::vector<sQuad> &render, bool player = false
	)
	{
		// 2. vCell is the {x,y} in world space of our cell. In order to know
		//    where it should be drawn on the screen( screen space ), it must undergo at least
		//    the first transformation of CreateCube() ->

		// Don't render things that exit the map area, render them on the other side of the plane
		if (vCell.x * fScale - vSpace.x < MAP_LEFT_EDGE) {
			return;
		}
			


		std::array<vec3d, 8> projCube = CreateCube(vCell, fAngle, fPitch, fScale, vSpace);

		// 4. Every vertex of the cube is now properly positioned

		auto& cell = world.GetCell(vCell);

		// Defines the faces of the cube. Each int is an ID of which vertex we want to acknowledge. Face comes from enum.
		// The projCube pushed onto our render vector will now contain those vertices in screen-space
		auto MakeFace = [&](int v1, int v2, int v3, int v4, Face f)
		{
			// id[f] is one of the 6 faces, determined at the end of this function (these, 4, vertices, are, thisFaceQuad)
			render.push_back({ projCube[v1], projCube[v2], projCube[v3], projCube[v4], cell.id[f] });
		};

		// 5. Determine which faces are visible, and subesequently their vertices. e.g. (3,0,1,2 is the South face's vertices)

		// As of right now these always true
		if (bVisible[Face::Floor] && !player) MakeFace(4, 0, 1, 5, Face::Floor);
		if (bVisible[Face::North] && player) MakeFace(6, 5, 4, 7, Face::North);

		// As of right now these are always false
		//if (bVisible[Face::South]) MakeFace(3, 0, 1, 2, Face::South);
		//if (bVisible[Face::East]) MakeFace(7, 4, 0, 3, Face::East);
		//if (bVisible[Face::West]) MakeFace(2, 1, 5, 6, Face::West);
		//if (bVisible[Face::Top]) MakeFace(7, 3, 2, 6, Face::Top);
		// }
	}

	/*
	@function GetPLayerQuads
	The player comes with its position tracker
*/
	void GetPlayerQuads(const olc::vi2d& vCell, const float fAngle, const float fPitch,
		const float fScale, const vec3d& vSpace, std::vector<sQuad> &render, float fElapsedTime)
	{
		std::array<vec3d, 8> cursCube = CreateCube(vCell, fAngle, fPitch, fScale, vSpace);
		std::array<vec3d, 8> playCube = CreateCube(vCell, fAngle, fPitch, fScale, vSpace);

		// Offset the player from the cursor
		for (auto &pc : playCube)
		{
			// The players separation from its ground marker
			pc.y += pDeltaY;
		}

		// Get the cell that the cursor sits on
		auto& cell = world.GetCell(vCell);

		// Defines the faces of the cube. Each int is an ID of which vertex we want to acknowledge. Face comes from enum.
		// The projCube pushed onto our render vector will now contain those vertices in screen-space
		auto MakeEntity = [&](int v1, int v2, int v3, int v4, Face f, std::array<vec3d, 8> cube, bool gravEnabled)
		{
			// id[f] is one of the 6 faces, determined at the end of this function (these, 4, vertices, are, thisFaceQuad)
			render.push_back({ cube[v1], cube[v2], cube[v3], cube[v4], cell.id[f], gravEnabled });
		};

		/*if (bVisible[Face::Floor] && !player)*/ 
		MakeEntity(4, 0, 1, 5, Face::Floor, cursCube, false);
		/*if (bVisible[Face::North] && player)*/ 
		MakeEntity(6, 5, 4, 7, Face::North, playCube, true);

	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		float x = ScreenHeight();
		// Grab mouse for convenience
		olc::vi2d vMouse = { GetMouseX(), GetMouseY() };

		// WS keys to tilt camera
		if (GetKey(olc::Key::W).bHeld) fCameraPitch += 1.0f * fElapsedTime;
		if (GetKey(olc::Key::S).bHeld) fCameraPitch -= 1.0f * fElapsedTime;

		// DA Keys to manually rotate camera
		if (GetKey(olc::Key::D).bHeld) fCameraAngleTarget += 1.0f * fElapsedTime;
		if (GetKey(olc::Key::A).bHeld) fCameraAngleTarget -= 1.0f * fElapsedTime;

		// QZ Keys to zoom in or out
		// if (GetKey(olc::Key::Q).bHeld) fCameraZoom += 5.0f * fElapsedTime;
		// if (GetKey(olc::Key::Z).bHeld) fCameraZoom -= 5.0f * fElapsedTime;

		if (GetKey(olc::Key::R).bPressed) 
			resetCamera();

		// Numpad keys used to rotate camera to fixed angles
		//if (GetKey(olc::Key::P).bPressed) fCameraAngleTarget = 3.14159f * 0.0f;
		//if (GetKey(olc::Key::O).bPressed) fCameraAngleTarget = 3.14159f * 0.25f;
		//if (GetKey(olc::Key::I).bPressed) fCameraAngleTarget = 3.14159f * 0.5f;
		//if (GetKey(olc::Key::U).bPressed) fCameraAngleTarget = 3.14159f * 0.75f;
		//if (GetKey(olc::Key::Y).bPressed) fCameraAngleTarget = 3.14159f * 1.0f;
		//if (GetKey(olc::Key::T).bPressed) fCameraAngleTarget = 3.14159f * 1.25f;
		//if (GetKey(olc::Key::L).bPressed) fCameraAngleTarget = 3.14159f * 1.75f;

		// Smooth camera
		fCameraAngle += (fCameraAngleTarget - fCameraAngle) * 10.0f * fElapsedTime;

		// Arrow keys to move the selection cursor around map (boundary checked)
		// if (GetKey(olc::Key::LEFT).bPressed) vCursor.x--;
		if (GetKey(olc::Key::LEFT).bHeld) player.vCursor.x -= 15.0f * fElapsedTime;
		// if (GetKey(olc::Key::RIGHT).bPressed) vCursor.x++;
		if (GetKey(olc::Key::RIGHT).bHeld) player.vCursor.x += 15.0f * fElapsedTime;
		// if (GetKey(olc::Key::UP).bPressed) vCursor.y--;
		if (GetKey(olc::Key::UP).bHeld) player.vCursor.y -= 15.0f * fElapsedTime;
		// if (GetKey(olc::Key::DOWN).bPressed) vCursor.y++;
		if (GetKey(olc::Key::DOWN).bHeld) player.vCursor.y += 15.0f * fElapsedTime;


		if (player.vCursor.x < 0) player.vCursor.x = 0;
		if (player.vCursor.y < 0) player.vCursor.y = 0;
		if (player.vCursor.x >= world.size.x) player.vCursor.x = world.size.x - 1;
		if (player.vCursor.y >= world.size.y) player.vCursor.y = world.size.y - 1;


		// Apply gravity to the situation, or thrust
		if (GetKey(olc::Key::SPACE).bHeld)
		{
			if (pDeltaY > MAP_TOP_EDGE)
				pDeltaY -= thrust * fElapsedTime;
		}
		else if (pDeltaY < 0) // Delta is negative
			pDeltaY += (gravity * fElapsedTime);

		// Rendering
		
		// Alter VSpace
		vSpace.x += VSPEED_X * fElapsedTime;

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

				// Each cell consists of 6 quads which are the faces of a cube. 
				// vQuads will contain any quads which make up that particular cube at location {x, y} in world space
				// 1. We begin with the x,y of world space. ->
				GetWorldQuads({ x, y }, fCameraAngle, fCameraPitch, fCameraZoom, vSpace, vQuads);

			}

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
			//DrawPartialWarpedDecal
			//(
			//	rendAllWalls.decal,
			//	{ {q.points[0].x, q.points[0].y}, {q.points[1].x, q.points[1].y}, {q.points[2].x, q.points[2].y}, {q.points[3].x, q.points[3].y} },
			//	q.tile,
			//	vTileSize
			//);

			if (q.points[0].x <= 120) {
				DrawLine(q.points[0].x, q.points[0].y, q.points[1].x, q.points[1].y, olc::DARK_GREEN);
				DrawLine(q.points[1].x, q.points[1].y, q.points[2].x, q.points[2].y, olc::DARK_GREEN);
			}

			else {
				DrawLine(q.points[0].x, q.points[0].y, q.points[1].x, q.points[1].y, olc::GREEN);
				DrawLine(q.points[1].x, q.points[1].y, q.points[2].x, q.points[2].y, olc::GREEN);
			}

		}

		/*
			5) Draw current tile selection in corner
		*/
		//DrawPartialDecal({ 10,10 }, rendAllWalls.decal, player.vTileCursor * vTileSize, vTileSize);
		vQuads.clear();
		/*
			6) Draw Player	
		*/ 

		GetPlayerQuads(player.vCursor, fCameraAngle, fCameraPitch, fCameraZoom, { vCameraPos.x, 0.0f, vCameraPos.y }, vQuads, fElapsedTime);
		for (auto& q : vQuads) {
			DrawWarpedDecal(rendSelect.decal, { {q.points[0].x, q.points[0].y}, {q.points[1].x, q.points[1].y}, {q.points[2].x, q.points[2].y}, {q.points[3].x, q.points[3].y} });
		}
		vQuads.clear();

		DrawStringDecal({ 500,0  + 20}, "P-0: "+ std::to_string(vQuads[1].points[0].x) + ", " + std::to_string(vQuads[1].points[0].y), olc::CYAN, { 0.5f, 0.5f });
		DrawStringDecal({ 500,8  + 20}, "P-1: "+ std::to_string(vQuads[1].points[1].x) + ", " + std::to_string(vQuads[1].points[1].y), olc::CYAN, { 0.5f, 0.5f });
		DrawStringDecal({ 500,16 + 20 }, "P-2: " + std::to_string(vQuads[1].points[2].x) + ", " + std::to_string(vQuads[1].points[2].y), olc::CYAN, { 0.5f, 0.5f });
		DrawStringDecal({ 500,24 + 20 }, "P-3: " + std::to_string(vQuads[1].points[3].x) + ", " + std::to_string(vQuads[1].points[3].y), olc::CYAN, { 0.5f, 0.5f });
											  
		DrawStringDecal({ 500,32 + 20 }, "V-0: " + std::to_string(vQuads[0].points[0].x) + ", " + std::to_string(vQuads[0].points[0].y), olc::CYAN, { 0.5f, 0.5f });
		DrawStringDecal({ 500,40 + 20 }, "V-1: " + std::to_string(vQuads[0].points[1].x) + ", " + std::to_string(vQuads[0].points[1].y), olc::CYAN, { 0.5f, 0.5f });
		DrawStringDecal({ 500,48 + 20 }, "V-2: " + std::to_string(vQuads[0].points[2].x) + ", " + std::to_string(vQuads[0].points[2].y), olc::CYAN, { 0.5f, 0.5f });
		DrawStringDecal({ 500,56 + 20 }, "V-3: " + std::to_string(vQuads[0].points[3].x) + ", " + std::to_string(vQuads[0].points[3].y), olc::CYAN, { 0.5f, 0.5f });
		DrawStringDecal({ 500,64 + 20 }, "Gravity: " + std::to_string(gravity), olc::RED, { 0.5f, 0.5f });
		DrawStringDecal({ 500,72 + 20 }, "Delta-Y: " + std::to_string(pDeltaY), olc::WHITE, { 0.5f, 0.5f });

		/*
			7) Draw some debug info
		*/
		DrawStringDecal({ 0,0 }, "Player Cursor: " + std::to_string(player.vCursor.x) + ", " + std::to_string(player.vCursor.y), olc::YELLOW, { 0.5f, 0.5f });
		DrawStringDecal({ 0,8 }, "Angle: " + std::to_string(fCameraAngle) + ", " + std::to_string(fCameraPitch), olc::YELLOW, { 0.5f, 0.5f });
		DrawStringDecal({ 0,16 }, "Time: " + std::to_string(fElapsedTime * 1000));
		DrawStringDecal({ 0,48 }, "vSpace.x: " + std::to_string(vSpace.x), olc::CYAN, { 0.5f, 0.5f });
		DrawStringDecal({ 0,56 }, "vSpace.y: " + std::to_string(vSpace.y), olc::CYAN, { 0.5f, 0.5f });
		DrawStringDecal({ 0,64 }, "vSpace.z: " + std::to_string(vSpace.z), olc::CYAN, { 0.5f, 0.5f });
		DrawStringDecal({ 0,72 }, "Zoom: " + std::to_string(fCameraZoom), olc::CYAN, { 0.5f, 0.5f });


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