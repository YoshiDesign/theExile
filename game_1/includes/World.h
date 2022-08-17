#pragma once
#include "./olcPixelGameEngine.h"
#include "Helpers.h"

/*
	Plane's rely on this class to access their cells.
	World has its own allCells<sCell> vector to access on its own and should prefer that method
*/
class GPS
{
private:
	sCell NullCell;

public:
	std::vector<sCell> vCells;
	olc::vi2d size;

public:
	GPS() {};
	GPS(int w, int h) : size{ w,h } { vCells.resize(w * h); };
	~GPS() {};

	// Address a coordinate which exists on a Plane
	sCell& GetCell(const olc::vi2d& v)
	{
		if (v.x >= 0 && v.x < size.x && v.y >= 0 && v.y < size.y)
			// Extraction of a sCell from vCells
			return vCells[v.y * size.x + v.x];
		else
			return NullCell;
	}

	// Address a coordinate which exists in the World
	sCell& GetWorldCell(const olc::vi2d& v, std::vector<sCell>& allCells)
	{
		if (v.x >= 0 && v.x < size.x && v.y >= 0 && v.y < size.y)
			// Extraction of a sCell from vCells
			return allCells[v.y * size.x + v.x];
		else
			return NullCell;
	}

	// Don't use this with World class unless you're calling it from a Plane instance
	const std::vector<sCell> VC()
	{
		return vCells;
	}

};

class Plane
{
private:

public:
	Plane() { gps = GPS(); };
	Plane(int w, int h, int i) : index{ i }, size{ w, h } {

		UpdatePlane();
		gps = GPS(w, h);

	};

	olc::vi2d dimMax() { return { size.x, size.y }; }

	void UpdatePlane()
	{

		for (int y = 0; y < size.y; y++)
			for (int x = 0; x < size.x; x++)
			{
				// Nothing is a wall from the start
				gps.GetCell({ x, y }).wall = false;

				// Assign each face a tile from the spritesheet
				gps.GetCell({ x, y }).id[Face::Floor] = olc::vi2d{ rand() % 3, rand() % 3 } *vTileSize;
				gps.GetCell({ x, y }).id[Face::Top] = olc::vi2d{ 0, 0 } *vTileSize;
				gps.GetCell({ x, y }).id[Face::North] = olc::vi2d{ 0, 0 } *vTileSize;
				gps.GetCell({ x, y }).id[Face::South] = olc::vi2d{ 0, 0 } *vTileSize;
				gps.GetCell({ x, y }).id[Face::West] = olc::vi2d{ 0, 0 } *vTileSize;
				gps.GetCell({ x, y }).id[Face::East] = olc::vi2d{ 0, 0 } *vTileSize;

			}

	}

	~Plane() {};

public:
	GPS gps;
	olc::vi2d size;

	int index;

};
/*
		Round
		Contains 2 Planes that compose the world
	*/
class World
{
public:
	olc::vi2d size;
	std::vector<Plane> planes;
	std::vector<sCell> allCells;

	Plane plane_1;
	Plane plane_2;

public:
	World() {
		size = { WORLD_WIDTH * 2, WORLD_HEIGHT };
	}

	void Create()
	{
		// Setup Planes
		plane_1 = Plane(WORLD_WIDTH, WORLD_HEIGHT, 1);
		plane_2 = Plane(WORLD_WIDTH, WORLD_HEIGHT, 2);

		// These are currently unused
		// planes.push_back(plane_1);
		// planes.push_back(plane_2);
		allCells.clear();
		// Implicitly allocates proper size. Is that bad?
		allCells.insert(allCells.end(), plane_1.gps.vCells.begin(), plane_1.gps.vCells.end());
		allCells.insert(allCells.end(), plane_2.gps.vCells.begin(), plane_2.gps.vCells.end());

	}

	olc::vi2d dimMax() { return { 2 * WORLD_WIDTH, WORLD_HEIGHT }; }
	sCell& GetCell(const olc::vi2d& v) { return gps.GetWorldCell(v, allCells); }

	/*
		@function UpdateWorld
		@param plane_index - Strategy for procedural regen
	*/
	void UpdateWorld(int plane_index)
	{

		/*
			Instead of doing it this way, copy the 2nd half into a buffer, update the 1st half w/ the buffer
			then
		*/

		//if (plane_index == 0) {
		//	plane_2.UpdatePlane();
		//	auto last = std::copy(std::begin(plane_2.gps.vCells), std::end(plane_2.gps.vCells), std::begin(allCells) + plane_1.gps.vCells.size());
		//	std::cout << "Updated 2nd Plane!" << std::endl;
		//}

		//if (plane_index == 1) {
		//	plane_1.UpdatePlane();
		//	auto last = std::copy(std::begin(plane_1.gps.vCells), std::end(plane_1.gps.vCells), std::begin(allCells));
		//	std::cout << "Updated 1st Plane!" << std::endl;
		//}

		// Best Strategy
		if (plane_index == 3) {

			allCells.clear();

			// update any plane
			plane_1.UpdatePlane();
			plane_2.UpdatePlane();

			// copy into the first half of allCells
			auto last = std::copy(std::begin(plane_1.gps.vCells), std::end(plane_1.gps.vCells), std::begin(allCells));

			// We don't need to worry about the 2nd half, right?
			std::copy(std::begin(plane_2.gps.vCells), std::end(plane_2.gps.vCells), last);
			//std::cout << "Updated with new clout." << std::endl;
		}

	}


private:
	GPS gps;
};