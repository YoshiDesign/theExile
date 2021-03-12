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
#include <algorithm>
#include "includes/olcPixelGameEngine.h"
#include "includes/Player.h"
#include "includes/Helpers.h"
#include "includes/Camera.h"
#include "includes/World.h"

/*									   

	NOTE! This program requires a tile spritesheet NOT
	provided in this github. You only need a few tiles,
	see video for details.

*/

class olcDungeon : public olc::PixelGameEngine
{

	int EPOCH = 0;

	int EPOCH_MOD = 90;
	float VTWEAK = 76.0f;
	float DX = 0.0f;
	float DY = 0.0f;
	float DT = 0.0f;
	float C_DELTA = 0.0;
	float VSPEED_X = 415.0f;
	const int WIGGLE_ROOM_TOP = 0;
	const int WIGGLE_ROOM_BOTTOM = 10;
	int Tindex = 0;
	int WorldUpdated = 0;

	// Alters spaceship spritesheet Y
	int ev = 0;

	// Effect magnitudes
	float gravity = 9.0f;
	float thrust = 36.0f;

public:
	olcDungeon()
	{
		sAppName = "YTE";
	}

	Player player;
	World world;
	//Renderable rendPlayer;
	Renderable rendAllWalls;

	vec3d vSpace;	// Initialized using the camera's initial vector. Used to move the world in relation to the camera

	// 1 World has gone by
	int vSpaceMod = vCalc(WORLD_WIDTH * 2, vSpace.x, vSpace.y).x / 100 ;
	// 1 Plane has gone by
	int vSpaceModH = (vCalc(WORLD_WIDTH * 2, vSpace.x, vSpace.y).x / 100) / 2 ;

	bool bVisible[6];


public:
	bool OnUserCreate() override
	{
		// The initial position of the World
		
		resetCamera();
		// The initial position of the camera in the world		
		vCameraPos = cameraPos;
		vCameraPos *= fCameraZoom;
		vSpace = { vCameraPos.x, 0.0f, vCameraPos.y };

		player.Load("./gfx/sprite_sheet_1.png");
		rendAllWalls.Load("./gfx/grounds.png");

		world.Create();
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
		// space, albeit in 2 dimensional space, and with no orthog

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
	void CalculateVisibleFaces(/*std::array<vec3d, 8>& cube*/)
	{
		/*
		
			Currently, this always returns the same visible faces
		
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
		const float fScale, vec3d& vSpace, std::vector<sQuad> &render, olc::vf2d coor
	)
	{
		// 2. vCell is the {x,y} in world space of our cell. In order to know
		//    where it should be drawn on the screen( screen space ), it must undergo at least
		//    the first transformation of CreateCube() ->

		// Don't render that which lurks beyond our realm
		if (coor.x < -100 || coor.x > ScreenWidth() + 480 ) {
			return;
		}

		std::array<vec3d, 8> projCube = CreateCube(vCell, fAngle, fPitch, fScale, vSpace);

		// 4. Every vertex of the cube is now properly positioned

		sCell& cell = world.GetCell({ vCell.x % WORLD_WIDTH, vCell.y });

		// Defines the faces of the cube. Each int is an ID of which vertex we want to acknowledge. Face comes from enum.
		// The projCube pushed onto our render vector will now contain those vertices in screen-space
		auto MakeFace = [&](int v1, int v2, int v3, int v4, Face f)
		{
			// id[f] is one of the 6 faces, determined at the end of this function (these, 4, vertices, are, thisFaceQuad)
			render.push_back({ projCube[v1], projCube[v2], projCube[v3], projCube[v4], cell.id[f] });
		};

		// 5. Determine which faces are visible, and subesequently their vertices. e.g. (3,0,1,2 is the South face's vertices)

		// As of right now these always true
		if (bVisible[Face::Floor]) MakeFace(4, 0, 1, 5, Face::Floor);
		//if (bVisible[Face::North]) MakeFace(6, 5, 4, 7, Face::North);

		// As of right now these are always false
		//if (bVisible[Face::South]) MakeFace(3, 0, 1, 2, Face::South);
		//if (bVisible[Face::East]) MakeFace(7, 4, 0, 3, Face::East);
		//if (bVisible[Face::West]) MakeFace(2, 1, 5, 6, Face::West);
		//if (bVisible[Face::Top]) MakeFace(7, 3, 2, 6, Face::Top);
		// }
	}



	/*	///	///	///	///	///	///  /
	|            <>				 |
	|			  <>		     |
	|	 <Useful calculations>	 |
	|	         <>				 |
	|			  <>			 |
	/	///	///	///	///	///	/// */

	/*
		@function vCalc - Get the (x,y) of any world's cell in screen-space.
		@param index - The cell in the World's vector of cells (World->vSpace)
		@param spaceX - The distance the camera has moved from its starting location X
		@param spaceY - The distance the camera has moved from its starting location Y (unused, might be wrong at the moment)
	*/
	olc::vi2d vCalc(int index, float spaceX, float spaceY) {

		return {
			(int)((index * (fCameraZoom + WORLD_SCALE) - spaceX) + ScreenWidth() * PLAYER_OFFSET_X),
			(int)(-spaceY + (ScreenHeight() * PLAYER_OFFSET_Y))
		};
	}

	/*
		@function GetWorldLength
		The length of this world in pixels. Reaches to the farthest edge of the last column's top cell
	*/ 
	float GetWorldLength(vec3d vSpace)
	{
		return vCalc(WORLD_WIDTH, vSpace.x, vSpace.y).x;
	}
	
	// <END> Useful calculations

	/*
		@function GetPLayerQuads
		The player comes with its position tracker
	*/
	void GetPlayerQuads(const olc::vi2d& vCell, const float fAngle, const float fPitch,
		const float fScale, const vec3d& vSpace, std::vector<sQuad> &render, float fElapsedTime,
		bool gravEnabled = false, bool isPlayer = false)
	{
		std::array<vec3d, 8> cube = CreateCube(vCell, fAngle, fPitch, fScale + PLAYER_SCALE, vSpace);

		// Get the cell that the cursor sits on
		sCell cell = { false, true, {{256,256},{256,256}, {256,256}, {256,256}, {256,256}, {256,256}} };

		// Defines the faces of the cube. Each int is an ID of which vertex we want to acknowledge. Face comes from enum.
		// The projCube pushed onto our render vector will now contain those vertices in screen-space
		auto MakeEntity = [&](int v1, int v2, int v3, int v4, Face f, std::array<vec3d, 8> cube)
		{
			// This is pushing an sQuad. id[f] is one of the 6 faces, determined at the end of this function (these, 4, vertices, are, thisFaceQuad)
			render.push_back({ cube[v1], cube[v2], cube[v3], cube[v4], cell.id[f], gravEnabled, isPlayer });
		};

		switch (isPlayer)
		{
			case false : 
				// The cursor
				MakeEntity(4, 0, 1, 5, Face::Floor, cube);
			case true: 
				// The player. Offset the player from the cursor
				for (auto &pc : cube)
				{
					pc.y += player.altitude + player.posY;
					pc.x += player.posX;
				}
				MakeEntity(6, 5, 4, 7, Face::North, cube);
		}

	}

	void FlightControl(float fElapsedTime, int type)
	{
		float speed = (float)(VSPEED_X / SPEED_DIVISOR);
		float _PLAYER_Y_MAX = PLAYER_Y_MAX + player.altitude;
		float _PLAYER_Y_MIN = PLAYER_Y_MIN - player.altitude;

		switch (type)
		{

		// ScrubMode
		case 1:
			// Arrow keys to move the selection cursor around map (boundary checked)
			if (GetKey(olc::Key::LEFT).bHeld)
			{
				if (player.altitude > MAP_TOP_EDGE)
					player.altitude -= thrust * fElapsedTime;
			}
			else if (GetKey(olc::Key::RIGHT).bHeld) {
				if (player.altitude < MAP_TOP_EDGE) {
					player.altitude += thrust * fElapsedTime;
					player.posX += 35 * fElapsedTime;
				}
			}
			else if (player.altitude < 0) {
				player.altitude += (gravity * fElapsedTime);
			}

			// if (GetKey(olc::Key::UP).bPressed) vCursor.y--;
			if (GetKey(olc::Key::UP).bHeld) {
				player.location.y -= C_DELTA * fElapsedTime;
				player.posX += 35 * fElapsedTime;
				player.posY -= 35 * fElapsedTime;
			}
			// if (GetKey(olc::Key::DOWN).bPressed) vCursor.y++;
			if (GetKey(olc::Key::DOWN).bHeld) {
				player.location.y += C_DELTA * fElapsedTime;
				player.posY += 35 * fElapsedTime;
				player.posX -= 35 * fElapsedTime;
			}

		// ProMode
		case 2:

			if (GetKey(olc::Key::RIGHT).bHeld) {


				if (VSPEED_X <= PLAYER_MAX_SPEED) {
					VSPEED_X += 100 * fElapsedTime;
					PLAYER_OFFSET_X += .014 * fElapsedTime;
				}


			}
			if (GetKey(olc::Key::RIGHT).bReleased)
			{

			}

			//
			if (GetKey(olc::Key::LEFT).bHeld) {

				if (VSPEED_X >= PLAYER_MIN_SPEED) {
					VSPEED_X -= 100 * fElapsedTime;
					PLAYER_OFFSET_X -= .014 * fElapsedTime;
				}
			}
			if (GetKey(olc::Key::LEFT).bReleased)
			{

			}
			
			if (GetKey(olc::Key::UP).bHeld && player.posY >= _PLAYER_Y_MIN) {

	
					DX += speed * fElapsedTime;
					DY -= speed * fElapsedTime;


			}
			else if (player.posY <= _PLAYER_Y_MIN)
			{
				player.posX = PLAYER_X_MAX_TOP;
				player.posY = _PLAYER_Y_MIN + 0.1f;
				DY = 0;
				DX = 0;
			}

			if (GetKey(olc::Key::DOWN).bHeld && player.posY <= _PLAYER_Y_MAX) {

					DX -= speed * fElapsedTime;
					DY += speed * fElapsedTime;


			}
			else if (player.posY >= _PLAYER_Y_MAX)
			{
				player.posX = PLAYER_X_MAX_BOTTOM;
				player.posY = _PLAYER_Y_MAX - 0.1f;
				DX = 0;
				DY = 0;
			}

			if (GetKey(olc::Key::W).bHeld)
			{
				DT -= 0.09f * fElapsedTime;
			}
			else if (GetKey(olc::Key::W).bReleased)
			{
				do
				{
					DT += 0.0001f * fElapsedTime;
				} while (DT < 0.0f);
			}

			// Return to ground
			if (player.altitude < 0) {
				player.altitude += (gravity * fElapsedTime);
			}

			movePlayer();

		default:
			break;
		}
		
	}

	void movePlayer()
	{
		DT = DT < 1 && DT > -0.4f ? DT : DT < -0.4f ? -0.4f : DT > 1 ? 0.4f : DT = DT;
		DY = DY < 1 && DY > -1 ? DY : DY < -1 ? -1 : DY > 1 ? 1 : DY;
		DX = DX < 1 && DX > -1 ? DX : DX < -1 ? -1 : DX > 1 ? 1 : DX;

		player.posY += DY;
		player.posX += DX;
		player.altitude += DT;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		float x = ScreenHeight();

		// Grab mouse for convenience
		olc::vi2d vMouse = { GetMouseX(), GetMouseY() };

		// WS keys to tilt camera
		//if (GetKey(olc::Key::W).bHeld) fCameraPitch += 1.0f * fElapsedTime;
		//if (GetKey(olc::Key::S).bHeld) fCameraPitch -= 1.0f * fElapsedTime;

		//// DA Keys to manually rotate camera
		//if (GetKey(olc::Key::D).bHeld) fCameraAngleTarget += 1.0f * fElapsedTime;
		//if (GetKey(olc::Key::A).bHeld) fCameraAngleTarget -= 1.0f * fElapsedTime;

		// QZ Keys to zoom in or out
		//if (GetKey(olc::Key::Q).bHeld) fCameraZoom += 5.0f * fElapsedTime;
		// if (GetKey(olc::Key::Z).bHeld) fCameraZoom -= 5.0f * fElapsedTime;

		//if (GetKey(olc::Key::R).bPressed)
			//resetCamera();

		if (GetKey(olc::Key::I).bPressed) PLAYER_SCALE--;
		if (GetKey(olc::Key::U).bPressed) PLAYER_SCALE++;
		if (GetKey(olc::Key::L).bPressed) WORLD_SCALE++;
		if (GetKey(olc::Key::K).bPressed) WORLD_SCALE--;
		if (GetKey(olc::Key::H).bPressed) C_DELTA += 1.0f;
		if (GetKey(olc::Key::G).bPressed) C_DELTA -= 1.0f;
		if (GetKey(olc::Key::X).bHeld) DX += 1.0f;
		if (GetKey(olc::Key::Z).bHeld) DX -= 1.0f;
		if (GetKey(olc::Key::C).bHeld) DY -= 1.0f;
		if (GetKey(olc::Key::V).bHeld) DY += 1.0f;
		if (GetKey(olc::Key::F).bHeld) Tindex += 1;
		
		if (GetKey(olc::Key::F3).bPressed) VTWEAK -= 1;
		if (GetKey(olc::Key::F4).bPressed) VTWEAK += 1;
		if (GetKey(olc::Key::F5).bPressed) ev -= 1 || 0;
		if (GetKey(olc::Key::F6).bPressed) ev += 1;

		// Smooth camera
		fCameraAngle += (fCameraAngleTarget - fCameraAngle) * 10.0f * fElapsedTime;

		// Check key pressess
		FlightControl(fElapsedTime, 2);

		/*if (player.vCursor.x < 0) player.vCursor.x = 0;
		if (player.vCursor.y < 0 - WIGGLE_ROOM_TOP) player.vCursor.y = 0;
		if (player.vCursor.x >= world.size.x) player.vCursor.x = world.size.x - 1;
		if (player.vCursor.y >= world.size.y + WIGGLE_ROOM_BOTTOM) player.vCursor.y = world.size.y - 1 + WIGGLE_ROOM_BOTTOM;*/


		// Alter VSpace - This moves the map
		vSpace.x += (int) VSPEED_X * fElapsedTime;
		
		// Passing the 2nd Plane
		if ( (int) vSpace.x > 0 && (int) ceil(vSpace.x / 100) % (int) ( vSpaceMod + ceil((ScreenWidth() / 55))) == 0)
		{
			EPOCH++;
			vSpace.x = VTWEAK;
			std::cout << "Updating Planes..." << std::endl;
			world.UpdateWorld(3);

		}
		// Passing the 1st Plane
		//else if ((int)vSpace.x > vSpaceMod / 2 && (int)ceil(vSpace.x / 100) % vSpaceModH == 0)
		//{
		//	std::cout << "Updating 1st Plane..." << std::endl;
		//	EPOCH++;
		//	world.UpdateWorld(1);
		//	vSpace.x += 10;
		//}

		/*
			Create dummy cube to extract visible face information
		*/

		// Cull faces that cannot be seen
		//std::array<vec3d, 8> cullCube = CreateCube({ 0, 0 }, fCameraAngle, fCameraPitch, fCameraZoom, vSpace);

		// Sets the booleans determining which faces we can see
		CalculateVisibleFaces(/*cullCube*/);

		// A container filled with the things we're going to draw
		std::vector<sQuad> vQuads;

		// Accounts for both Planes
		for (int y = 0; y < world.size.y; y++) {
			for (int x = 0; x < world.size.x; x++) {

				float xx = (float)vCalc(x, vSpace.x, vSpace.y).x;
				float yy = (float)vCalc(x + y, vSpace.x, vSpace.y).y;
				olc::vf2d coor{ xx, yy };

				// Each cell consists of 6 quads which are the faces of a cube. 
				// vQuads will contain any quads which make up that particular cube at location {x, y} in world space
				// 1. We begin with the x,y of world space. ->

				GetWorldQuads({ x, y }, fCameraAngle, fCameraPitch, (fCameraZoom + WORLD_SCALE), vSpace, vQuads, coor);
				//DrawStringDecal(coor, std::to_string((int) coor.x), olc::RED, { 2.47f, 1.47f });
			}

		}

		/*
			Sort in order of depth, from farthest away to closest
		*/
		std::sort(vQuads.begin(), vQuads.end(), [](const sQuad& q1, const sQuad& q2)
		{
			float z1 = (q1.points[0].z + q1.points[1].z + q1.points[2].z + q1.points[3].z) * 0.25f;
			float z2 = (q2.points[0].z + q2.points[1].z + q2.points[2].z + q2.points[3].z) * 0.25f;
			return z1 < z2;
		});

		/*
			Iterate through all "tile cubes" and draw their visible faces
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

				DrawLine(q.points[0].x, q.points[0].y, q.points[1].x, q.points[1].y, olc::DARK_CYAN);
				DrawLine(q.points[1].x, q.points[1].y, q.points[2].x, q.points[2].y, olc::DARK_CYAN);

			}

			else {
				DrawLine(q.points[0].x, q.points[0].y, q.points[1].x, q.points[1].y, olc::CYAN);
				DrawLine(q.points[1].x, q.points[1].y, q.points[2].x, q.points[2].y, olc::CYAN);
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


		player.Update(vSpace.x, ev);
		//GetPlayerQuads(player.vCursor, fCameraAngle, fCameraPitch, fCameraZoom, { vCameraPos.x, 0.0f, vCameraPos.y }, vQuads, fElapsedTime);
		GetPlayerQuads(player.location, fCameraAngle, fCameraPitch, fCameraZoom, { 0.0f, 0.0f, 0.0f }, vQuads, fElapsedTime, true, true);
		for (auto& q : vQuads) {
			if (q.isPlayer) {
				
				DrawPartialWarpedDecal(
					player.p1.decal,
					{ {q.points[0].x, q.points[0].y}, {q.points[1].x, q.points[1].y}, {q.points[2].x, q.points[2].y}, {q.points[3].x, q.points[3].y} },
					player.tile * vTileSize,
					vTileSize
				);
			}
			else
			// Retro mode
			{
				DrawLine(q.points[0].x, q.points[0].y, q.points[1].x / 2, q.points[1].y / 2, olc::MAGENTA);
				DrawLine(q.points[1].x / 2, q.points[1].y / 2, q.points[3].x, q.points[3].y, olc::MAGENTA);
				DrawLine(q.points[3].x, q.points[3].y, q.points[0].x, q.points[0].y, olc::MAGENTA);
			}

		}
		vQuads.clear();
		
		//DrawLine(vCalc(0,vSpace.x,vSpace.y).x, vCalc(WORLD_HEIGHT, vSpace.x, vSpace.y).y, vCalc(WORLD_WIDTH, vSpace.x, vSpace.y).x, vCalc(WORLD_HEIGHT, vSpace.x, vSpace.y).y, olc::GREEN);
		//DrawLine(vCalc(WORLD_WIDTH, vSpace.x, vSpace.y).x, vCalc(WORLD_HEIGHT, vSpace.x, vSpace.y).y, vCalc(WORLD_WIDTH * 2, vSpace.x, vSpace.y).x, vCalc(WORLD_HEIGHT, vSpace.x, vSpace.y).y, olc::MAGENTA);
		//
		//DrawStringDecal({ 500,0  + 20}, "P-0: "+ std::to_string(vQuads[1].points[0].x) + ", " + std::to_string(vQuads[1].points[0].y), olc::CYAN, { 0.5f, 0.5f });
		//DrawStringDecal({ 500,8  + 20}, "P-1: "+ std::to_string(vQuads[1].points[1].x) + ", " + std::to_string(vQuads[1].points[1].y), olc::CYAN, { 0.5f, 0.5f });
		//DrawStringDecal({ 500,16 + 20 }, "P-2: " + std::to_string(vQuads[1].points[2].x) + ", " + std::to_string(vQuads[1].points[2].y), olc::CYAN, { 0.5f, 0.5f });
		//DrawStringDecal({ 500,24 + 20 }, "P-3: " + std::to_string(vQuads[1].points[3].x) + ", " + std::to_string(vQuads[1].points[3].y), olc::CYAN, { 0.5f, 0.5f });
		//									  
		//DrawStringDecal({ 500,32 + 20 }, "V-0: " + std::to_string(vQuads[0].points[0].x) + ", " + std::to_string(vQuads[0].points[0].y), olc::CYAN, { 0.5f, 0.5f });
		//DrawStringDecal({ 500,40 + 20 }, "V-1: " + std::to_string(vQuads[0].points[1].x) + ", " + std::to_string(vQuads[0].points[1].y), olc::CYAN, { 0.5f, 0.5f });
		//DrawStringDecal({ 500,48 + 20 }, "V-2: " + std::to_string(vQuads[0].points[2].x) + ", " + std::to_string(vQuads[0].points[2].y), olc::CYAN, { 0.5f, 0.5f });
		//DrawStringDecal({ 500,56 + 20 }, "V-3: " + std::to_string(vQuads[0].points[3].x) + ", " + std::to_string(vQuads[0].points[3].y), olc::CYAN, { 0.5f, 0.5f });
		//DrawStringDecal({ 500,64 + 20 }, "Gravity: " + std::to_string(gravity), olc::RED, { 0.5f, 0.5f });
		//DrawStringDecal({ 500,72 + 20 }, "Delta-Y: " + std::to_string(player.posY), olc::WHITE, { 0.5f, 0.5f });

		/*
			7) Draw some debug info
		*/
		DrawStringDecal({ 10,10 }, "Score: " + std::to_string((int)vSpace.x), olc::CYAN, { 0.72f, 0.72f });
		//DrawStringDecal({ 300,10 }, "MOD: " + std::to_string((int)(vSpace.x / 10) % 6), olc::CYAN, { 0.72f, 0.72f });
		//DrawStringDecal({ 10 ,19 }, "" + std::to_string(), olc::CYAN, { 0.72f, 0.72f });
		DrawStringDecal({ 10 ,27 }, "Delta-X: " + std::to_string(player.posX), olc::CYAN, { 0.72f, 0.72f });
		DrawStringDecal({ 10 ,35 }, "Delta-Y: " + std::to_string(player.posY), olc::CYAN, { 0.72f, 0.72f });
		DrawStringDecal({ 10 ,48 }, "DX: " + std::to_string(DX), olc::CYAN, { 0.47f, 0.47f });
		DrawStringDecal({ 10 ,55 }, "DY: " + std::to_string(DY), olc::CYAN, { 0.47f, 0.47f });
		DrawStringDecal({ 10 ,62 }, "DT: " + std::to_string(DT), olc::CYAN, { 0.47f, 0.47f });
		DrawLine(10, 70, 70, 70, olc::MAGENTA);
		
		DrawStringDecal({ 10,78}, "Epoch: " + std::to_string(EPOCH), olc::WHITE, { 1.1f, 1.1f });
		DrawStringDecal({ 10 ,91 }, "Altitude: " + std::to_string(player.altitude), olc::GREEN, { 0.6f, 0.6f });
		DrawStringDecal({ 10 ,102 }, "World Planes: " + std::to_string(world.planes.size()), olc::WHITE, { 0.47f, 0.47f });		
		DrawStringDecal({ 10 ,112 }, "V-TWEAK: " + std::to_string(VTWEAK), olc::WHITE, { 0.47f, 0.47f });
		DrawStringDecal({ 10 ,122 }, "World VCells: " + std::to_string(world.allCells.size()), olc::WHITE, { 0.47f, 0.47f });
		DrawStringDecal({ 10 ,132 }, "Vspace %: " + std::to_string(vSpaceMod), olc::WHITE, { 0.47f, 0.47f });
		//DrawStringDecal({ 10 ,110 }, "Plane-2 VCells: " + std::to_string(world.plane_2.gps.vCells.size()), olc::WHITE, { 0.47f, 0.47f });
		//DrawStringDecal({ 10 ,130 }, "World VCells: " + std::to_string(world.allCells.size()), olc::WHITE, { 0.47f, 0.47f });
		//DrawStringDecal({ 10 ,140 }, "World VCells: " + std::to_string(world.allCells.size()), olc::WHITE, { 0.47f, 0.47f });
		DrawLine(10,148,70,148, olc::MAGENTA);
		DrawStringDecal({ 10 ,154 }, "VC.x: " + std::to_string(vCalc(WORLD_WIDTH, vSpace.x, vSpace.y).x), olc::RED, { 0.47f, 0.47f });
		DrawStringDecal({ 10 ,162 }, "Tile-Index: " + std::to_string(Tindex), olc::RED, { 0.47f, 0.47f });
		DrawLine(10, 170, 70, 170, olc::MAGENTA);
		DrawStringDecal({ 10 ,178 }, "Max Coordinates: (" + std::to_string(world.dimMax().x) + "," + std::to_string(world.dimMax().y) + ")", olc::RED, { 0.47f, 0.47f });
		
		//DrawStringDecal({ 10,8 }, "Angle: " + std::to_string(fCameraAngle) + ", " + std::to_string(fCameraPitch), olc::YELLOW, { 0.5f, 0.5f 
		//DrawStringDecal({ 10,72 }, "Epoch-Modulus: " + std::to_string(EPOCH_MOD), olc::CYAN, { 0.5f, 0.5f });
		DrawStringDecal({ 460,10 }, "(Unassigned) wShift: " + std::to_string(WORLD_SHIFT), olc::GREEN, { 0.7f, 0.7f });
		DrawStringDecal({ 460,18 }, "(I-)(U+) Player Scale: " + std::to_string(PLAYER_SCALE), olc::GREEN, { 0.7f, 0.7f });
		DrawStringDecal({ 460,26 }, "(K-)(L+) World Scale: " + std::to_string(WORLD_SCALE), olc::GREEN, { 0.7f, 0.7f });
		DrawStringDecal({ 460,34 }, "(H+)(L-) Cursor Delta: " + std::to_string(C_DELTA), olc::GREEN, { 0.7f, 0.7f });
		DrawStringDecal({ 460,42 }, "World Width: " + std::to_string(WORLD_WIDTH), olc::RED, { 0.7f, 0.7f });
		DrawStringDecal({ 460,52 }, "World Height: " + std::to_string(WORLD_HEIGHT), olc::RED, { 0.7f, 0.7f });
		DrawStringDecal({ 10, 480 }, "Player Cursor: " + std::to_string(player.location.x) + ", " + std::to_string(player.location.y), olc::RED, { 0.5f, 0.5f });

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