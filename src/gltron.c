/*
  gltron
  Copyright (C) 1999 by Andreas Umbach <marvin@dataway.ch>
*/

#include "game/init.h"
#include "filesystem/path.h"
#include "base/util.h"
#include "SDL.h"

int main(int argc, const char *argv[] ) {
	initSubsystems(argc, argv);
	runScript(PATH_SCRIPTS, "main.lua");
  return 0;
}







