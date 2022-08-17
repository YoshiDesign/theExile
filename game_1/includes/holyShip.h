#pragma once

#include "olcPixelGameEngine.h"
#include "Player.h"
#include "Helpers.h"
#include "Camera.h"
#include "World.h"
#include "Meteor.h"
#include "Patterns/Command.h"
#include "Patterns/Actions.h"

#define LOG(a) std::cout << a << std::endl


class holyShip : public olc::PixelGameEngine
{
private:

	// What is currently being pressed
	struct PlayerKeysInUse {
		olc::HWButton W;
		olc::HWButton UP;
		olc::HWButton DOWN;
		olc::HWButton LEFT;
		olc::HWButton RIGHT;
	};

public:

	holyShip()
	{
		sAppName = "Holy Ship!";
	}

	float fTargetFrameTime = 1.0f / 61.0f; // Virtual FPS of 61fps
	float fAccumulatedTime = 0.0f;

	Player player;
	World world;

	/*
	 * All player commands
	 */
	ClimbCommand button_W;
	MoveFarCommand button_UP;
	MoveNearCommand button_DOWN;
	ThrottleDownCommand button_LEFT;
	ThrottleUpCommand button_RIGHT;
	EmptyCommand none;
	PlayerKeysInUse keys;
	
	//Renderable rendPlayer;
	Renderable rendAllWalls;
	vec3d vSpace;	// Initialized using the camera's initial vector. Used to move the world in relation to the camera

	// 1 World has gone by
	int vSpaceMod = vCalc(WORLD_WIDTH * 2, vSpace.x, vSpace.y).x / 100;
	// 1 Plane has gone by
	int vSpaceModH = (vCalc(WORLD_WIDTH * 2, vSpace.x, vSpace.y).x / 100) / 2;

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

		// Button buffer
		PlayerKeysInUse PlayerKeys = {};

		player.Load("./assets/gfx/sprite_sheet_1.png");
		rendAllWalls.Load("./assets/gfx/grounds.png");

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
			projCube[i].x = worldCube[i].x + ScreenWidth() * player.PLAYER_OFFSET_X;
			projCube[i].y = worldCube[i].y + ScreenHeight() * player.PLAYER_OFFSET_Y;
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
			because I've disabled camera control

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
		bVisible[Face::East] = false;	// CheckNormal(7, 4, 0);
		bVisible[Face::West] = false;	// CheckNormal(2, 1, 5);
		bVisible[Face::Top] = false;	// CheckNormal(7, 3, 2);
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
		if (coor.x < -100 || coor.x > ScreenWidth() + 480) {
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
			(int)((index * (fCameraZoom + WORLD_SCALE) - spaceX) + ScreenWidth() * player.PLAYER_OFFSET_X),
			(int)(-spaceY + (ScreenHeight() * player.PLAYER_OFFSET_Y))
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
		const float fScale, const vec3d& vSpace, std::vector<sQuad> &render, Player& player,
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

		if (isPlayer)
		{
			// The player. Offset the player from the cursor
			for (auto &pc : cube)
			{
				pc.y += player.altitude + player.posY;
				pc.x += player.posX;
			}
			MakeEntity(6, 5, 4, 7, Face::North, cube);
		}
		else
		{
			// The cursor
			MakeEntity(4, 0, 1, 5, Face::Floor, cube);
			
		}

	}

	void handleInput(PlayerKeysInUse key, float fTime)
	{

		if (key.W.bHeld)	button_W.execute(player, fTime);
		if (key.UP.bHeld)	button_UP.execute(player, fTime);
		if (key.DOWN.bHeld) button_DOWN.execute(player, fTime);
		if (key.LEFT.bHeld) button_LEFT.execute(player, fTime);
		if (key.RIGHT.bHeld) button_RIGHT.execute(player, fTime);

	}


	void drawScene(float fElapsedTime) 
	{

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

		//GetPlayerQuads(player.vCursor, fCameraAngle, fCameraPitch, fCameraZoom, { vCameraPos.x, 0.0f, vCameraPos.y }, vQuads, fElapsedTime);
		GetPlayerQuads(player.location, fCameraAngle, fCameraPitch, fCameraZoom, { 0.0f, 0.0f, 0.0f }, vQuads, player, true, true);
		for (auto& q : vQuads) {

			// Draw the player's distance from the ground
			DrawLine(
				q.points[1].x + tweak_x, q.points[1].y + tweak_y,
				q.points[1].x + tweak_x, q.points[1].y - player.altitude,
				olc::CYAN, 0x0A0A0A);
			DrawLine(
				q.points[1].x + tweak_x, q.points[1].y + tweak_y,
				q.points[1].x + tweak_x, q.points[1].y - player.altitude,
				olc::MAGENTA, 0xF6F6F6);

			if (q.isPlayer) {

				DrawPartialWarpedDecal(
					player.p1.decal,
					{ {q.points[0].x, q.points[0].y}, {q.points[1].x, q.points[1].y}, {q.points[2].x, q.points[2].y}, {q.points[3].x, q.points[3].y} },
					player.tile * vTileSize,
					vTileSize
				);
			}
			else
				// Retro mode.
			{
				DrawLine(q.points[0].x, q.points[0].y, q.points[1].x / 2, q.points[1].y / 2, olc::MAGENTA);
				DrawLine(q.points[1].x / 2, q.points[1].y / 2, q.points[3].x, q.points[3].y, olc::MAGENTA);
				DrawLine(q.points[3].x, q.points[3].y, q.points[0].x, q.points[0].y, olc::MAGENTA);
			}

		}


		vQuads.clear();


		drawHud(fElapsedTime);


	}


	void drawHud(float fElapsedTime)
	{
		/*
			   HUD
			*/
		if (player.DT == 0.0)
			DrawStringDecal({ 20 ,340 }, "V" + std::to_string((int)floor(1.8 * (int)ceil(player.DT * 100))), olc::WHITE, { 0.72f, 0.72f });
		else
			DrawStringDecal({ 20 ,340 }, "V" + std::to_string((int)floor(-1.8 * (int)ceil(player.DT * 100))), olc::WHITE, { 0.72f, 0.72f });

		/*
			Climb Meter
			The length depends upon the relationship between TW and CRUISE_ALTITUDE
			where TW * CRUISE_ALTITUDE + tLEFT.x is the distance from the start of the meter
		*/
		DrawLine(25, 350, tLEFT.x + -player.THRUST_WIDTH, 350, olc::MAGENTA);	// Top
		DrawLine(tLEFT.x, 350, bLEFT.x, 370, olc::MAGENTA);	// Left Side
		DrawLine(20, 370, bLEFT.x + -player.THRUST_WIDTH, 370, olc::MAGENTA);	// Bottom
		DrawLine(tLEFT.x + -player.THRUST_WIDTH, 350, bLEFT.x + -player.THRUST_WIDTH, 370, olc::MAGENTA); // Right side

		//float r2 = player.DT * 50;
		//float x1 = -50 - (600 * player.DT);
		//float y1 = (int)sqrt(r2 - x1 * x1) + 0.1;
		//DrawLine(70 + x1, 349 - y1, 27 - player.CRUISE_ALTITUDE * 2, 348, olc::WHITE);

		//DrawRect({20,350}, {130,20}, olc::MAGENTA);

		for (int i = 1, j = 0; i <= (int)-player.altitude; i++) {
			if (i % 2 == 0) {
				if (i < -player.CRUISE_ALTITUDE)
				{
					DrawLine(25 + (i * 2) / 2, 352, 20 + (i * 2) / 2, 368, olc::Pixel(7, 248, 73));
				}
				else
				{
					DrawLine(25 + (i * 2) / 2, 352, 20 + (i * 2) / 2, 368, olc::Pixel(7, 248 - (j * 3 >= 73 ? 73 : j * 3), 73 + j));
					j += 2;
				}
			}
		}

		// Thrust Cutoff
		DrawLine(26 - player.CRUISE_ALTITUDE, 348, 19 - player.CRUISE_ALTITUDE, 370, olc::WHITE);

		/*
			ENDHUD
		*/

		//FillRect({ 22, 352 }, { int(20 - player.altitude), 17}, olc::DARK_RED);

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
		DrawStringDecal({ 10,18 }, "Score: " + std::to_string((int)vSpace.x), olc::CYAN, { 0.72f, 0.72f });
		//DrawStringDecal({ 300,10 }, "MOD: " + std::to_string((int)(vSpace.x / 10) % 6), olc::CYAN, { 0.72f, 0.72f });
		//DrawStringDecal({ 10 ,19 }, "" + std::to_string(), olc::CYAN, { 0.72f, 0.72f });
		DrawStringDecal({ 10 ,27 }, "Pos-X: " + std::to_string(player.posX), olc::CYAN, { 0.72f, 0.72f });
		DrawStringDecal({ 10 ,35 }, "Pos-Y: " + std::to_string(player.posY), olc::CYAN, { 0.72f, 0.72f });
		DrawStringDecal({ 10 ,48 }, "DX: " + std::to_string(player.DX), olc::CYAN, { 0.88f, 0.88f });
		DrawStringDecal({ 10 ,56 }, "DY: " + std::to_string(player.DY), olc::CYAN, { 0.88f, 0.88f });
		DrawStringDecal({ 10 ,64 }, "DT: " + std::to_string(player.DT), olc::CYAN, { 0.88f, 0.88f });
		DrawStringDecal({ 10 ,72 }, "fTime: " + std::to_string(fElapsedTime), olc::CYAN, { 0.88f, 0.88f });
		//DrawLine(10, 70, 70, 70, olc::MAGENTA);

		DrawStringDecal({ 10,82 }, "Epoch: " + std::to_string(EPOCH), olc::WHITE, { 1.1f, 1.1f });
		DrawStringDecal({ 10 ,98 }, "Altitude: " + std::to_string(player.altitude), olc::GREEN, { 0.6f, 0.6f });
		DrawStringDecal({ 10 ,106 }, "pAltitude: " + std::to_string(player.pAltitude), olc::GREEN, { 0.6f, 0.6f });
		DrawStringDecal({ 10 ,114 }, "Ascending: " + std::to_string(player.ascending), olc::GREEN, { 0.6f, 0.6f });
		DrawStringDecal({ 10 ,122 }, "Offset-Y: " + std::to_string(player.PLAYER_OFFSET_Y), olc::CYAN, { 0.6f, 0.6f });
		//DrawStringDecal({ 10 ,119 }, "TW: " + std::to_string(TW), olc::WHITE, { 0.47f, 0.47f });
		//DrawStringDecal({ 10 ,127 }, "V-TWEAK: " + std::to_string(VTWEAK), olc::WHITE, { 0.47f, 0.47f });
		//DrawStringDecal({ 10 ,135 }, "tweak_x: " + std::to_string(tweak_x), olc::WHITE, { 0.47f, 0.47f });
		//DrawStringDecal({ 10 ,143 }, "tweak_y: " + std::to_string(tweak_y), olc::WHITE, { 0.47f, 0.47f });
		//DrawStringDecal({ 10 ,110 }, "Plane-2 VCells: " + std::to_string(world.plane_2.gps.vCells.size()), olc::WHITE, { 0.47f, 0.47f });
		//DrawStringDecal({ 10 ,130 }, "World VCells: " + std::to_string(world.allCells.size()), olc::WHITE, { 0.47f, 0.47f });
		//DrawStringDecal({ 10 ,140 }, "World VCells: " + std::to_string(world.allCells.size()), olc::WHITE, { 0.47f, 0.47f });
		/*DrawLine(10, 148, 70, 148, olc::MAGENTA);
		DrawStringDecal({ 10 ,154 }, "VC.x: " + std::to_string(vCalc(WORLD_WIDTH, vSpace.x, vSpace.y).x), olc::RED, { 0.47f, 0.47f });
		DrawStringDecal({ 10 ,162 }, "Tile-Index: " + std::to_string(Tindex), olc::RED, { 0.47f, 0.47f });
		DrawLine(10, 170, 70, 170, olc::MAGENTA);
		DrawStringDecal({ 10 ,178 }, "Max Coordinates: (" + std::to_string(world.dimMax().x) + "," + std::to_string(world.dimMax().y) + ")", olc::RED, { 0.47f, 0.47f });
		*/
		//DrawStringDecal({ 10,8 }, "Angle: " + std::to_string(fCameraAngle) + ", " + std::to_string(fCameraPitch), olc::YELLOW, { 0.5f, 0.5f 
		//DrawStringDecal({ 10,72 }, "Epoch-Modulus: " + std::to_string(EPOCH_MOD), olc::CYAN, { 0.5f, 0.5f });
		/*DrawStringDecal({ 460,10 }, "(Unassigned) wShift: " + std::to_string(WORLD_SHIFT), olc::GREEN, { 0.7f, 0.7f });
		DrawStringDecal({ 460,18 }, "(I-)(U+) Player Scale: " + std::to_string(PLAYER_SCALE), olc::GREEN, { 0.7f, 0.7f });
		DrawStringDecal({ 460,26 }, "(K-)(L+) World Scale: " + std::to_string(WORLD_SCALE), olc::GREEN, { 0.7f, 0.7f });

		DrawStringDecal({ 460,42 }, "World Width: " + std::to_string(WORLD_WIDTH), olc::RED, { 0.7f, 0.7f });
		DrawStringDecal({ 460,52 }, "World Height: " + std::to_string(WORLD_HEIGHT), olc::RED, { 0.7f, 0.7f });
		DrawStringDecal({ 10, 480 }, "Player Cursor: " + std::to_string(player.location.x) + ", " + std::to_string(player.location.y), olc::RED, { 0.5f, 0.5f });
		*/
	
	}

	bool OnUserUpdate(float fElapsedTime) override
	{

		// Grab mouse for convenience
		olc::vi2d vMouse = { GetMouseX(), GetMouseY() };

		//keys.W = GetKey(olc::Key::W);
		keys.UP = GetKey(olc::Key::UP);
		keys.DOWN = GetKey(olc::Key::DOWN);
		keys.LEFT = GetKey(olc::Key::LEFT);
		keys.RIGHT = GetKey(olc::Key::RIGHT);

		// WS keys to tilt camera
		if (GetKey(olc::Key::W).bHeld) fCameraPitch += 1.0f * fElapsedTime;
		if (GetKey(olc::Key::S).bHeld) fCameraPitch -= 1.0f * fElapsedTime;

		//// DA Keys to manually rotate camera
		if (GetKey(olc::Key::D).bHeld) fCameraAngleTarget += 1.0f * fElapsedTime;
		if (GetKey(olc::Key::A).bHeld) fCameraAngleTarget -= 1.0f * fElapsedTime;

		// QZ Keys to zoom in or out
		if (GetKey(olc::Key::Q).bHeld) fCameraZoom += 5.0f * fElapsedTime;
		if (GetKey(olc::Key::Z).bHeld) fCameraZoom -= 5.0f * fElapsedTime;

		//if (GetKey(olc::Key::R).bPressed)
			//resetCamera();

		if (GetKey(olc::Key::I).bPressed) PLAYER_SCALE--;
		if (GetKey(olc::Key::U).bPressed) PLAYER_SCALE++;
		if (GetKey(olc::Key::L).bPressed) WORLD_SCALE++;
		if (GetKey(olc::Key::K).bPressed) WORLD_SCALE--;

		//if (GetKey(olc::Key::X).bHeld) DX += 1.0f;
		//if (GetKey(olc::Key::Z).bHeld) DX -= 1.0f;
		//if (GetKey(olc::Key::C).bHeld) DY -= 1.0f;
		//if (GetKey(olc::Key::V).bHeld) DY += 1.0f;
		if (GetKey(olc::Key::F).bHeld) Tindex += 1;

		if (GetKey(olc::Key::F3).bPressed) VTWEAK -= 1;
		if (GetKey(olc::Key::F4).bPressed) VTWEAK += 1;
		if (GetKey(olc::Key::F5).bPressed) ev -= 1 || 0;
		if (GetKey(olc::Key::F6).bPressed) ev += 1;

		if (GetKey(olc::Key::F7).bHeld) TW += .01f;
		if (GetKey(olc::Key::F8).bHeld) TW -= .01f;

		// Smooth camera
		fCameraAngle += (fCameraAngleTarget - fCameraAngle) * 10.0f * fElapsedTime;

		/*if (player.vCursor.x < 0) player.vCursor.x = 0;
		if (player.vCursor.y < 0 - WIGGLE_ROOM_TOP) player.vCursor.y = 0;
		if (player.vCursor.x >= world.size.x) player.vCursor.x = world.size.x - 1;
		if (player.vCursor.y >= world.size.y + WIGGLE_ROOM_BOTTOM) player.vCursor.y = world.size.y - 1 + WIGGLE_ROOM_BOTTOM;*/

		fAccumulatedTime += fElapsedTime;
		if (fAccumulatedTime >= fTargetFrameTime)
		{
			drawScene(fElapsedTime);
			player.Update(fElapsedTime, vSpace.x, ev);
			handleInput(keys, fElapsedTime);
			fAccumulatedTime -= fTargetFrameTime;
			fElapsedTime = fTargetFrameTime;

			// Alter VSpace - This moves the map
			vSpace.x += (int)player.VSPEED_X * fElapsedTime;

			// Passing the 2nd Plane
			if ((int)vSpace.x > 0 && (int)ceil(vSpace.x / 100) % (int)(vSpaceMod + ceil((ScreenWidth() / 55))) == 0)
			{
				EPOCH++;
				vSpace.x = VTWEAK;
				std::cout << "Updating Planes..." << std::endl;
				world.UpdateWorld(3);
			}

		}
		else {
			// Draw but do not calculate
			drawScene(fElapsedTime);
			return true;
		}

		// Graceful exit if user is in full screen mode
		return !GetKey(olc::Key::ESCAPE).bPressed;
	}

};
