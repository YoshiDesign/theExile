#pragma once
#include "Command.h"
#include "../Player.h"

class MoveNearCommand : public Command
{
public:
	MoveNearCommand() {}
	virtual void execute(Player& player, float fTime) { player.moveNear(fTime); }

};

class MoveFarCommand : public Command
{
public:
	MoveFarCommand() {}
	virtual void execute(Player& player, float fTime) { player.moveFar(fTime); }
	
};

class ThrottleUpCommand : public Command
{
public:
	ThrottleUpCommand() {}
	virtual void execute(Player& player, float fTime) { player.throttleUp(fTime); }

};

class ThrottleDownCommand : public Command
{
public:
	ThrottleDownCommand() {}
	virtual void execute(Player& player, float fTime) {  player.throttleDown(fTime); }

};

class ClimbCommand : public Command
{
public:
	ClimbCommand() {}
	virtual void execute(Player& player, float fTime) { player.climb(fTime); }

};

class EmptyCommand : public Command
{
public:
	EmptyCommand() {}
	virtual void execute(Player& player, float time) { return; }
};
