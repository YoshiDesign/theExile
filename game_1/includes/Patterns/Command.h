#pragma once
#include "../Player.h"
#include "../olcPixelGameEngine.h"

class Command
{
public:
	virtual ~Command() {}
	virtual void execute(Player& actor, float fTime) = 0;
};