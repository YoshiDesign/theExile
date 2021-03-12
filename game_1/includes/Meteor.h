#include "olcPixelGameEngine.h"


class Meteor {

private:

public:

	Renderable m1;
	olc::vi2d tile = {};
	bool flying;


public:

	Meteor() {
	
	}

	void Load(const std::string& sFile) 
	{
		m1.Load(sFile);
	}

	void Update(int time, int ev = 0)
	{
		tile = { (int)ceil(time * .1) % 6, ev };
	}
	
	~Meteor() {}

};