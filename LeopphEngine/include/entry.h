#pragma once

#include "object.h"

namespace leopph
{
	// Sets the initial state of the game
	extern void Init();
}


int main(int argc, char** argv)
{
	leopph::Init();

	// DELETE ALL REMANINING OBJECTS

	return 0;
}