#include "gltron.h"

void initArtpacks() {
  const char *art_path;
  List *artList;
  List *p;
  int i;

  art_path = getDirectory( PATH_ART );
  artList = readDirectoryContents(art_path, NULL);
  if(artList->next == NULL) {
    fprintf(stderr, "[fatal] no art files found...exiting\n");
    exit(1); /* OK: critical, installation corrupt */
  }
  
  i = 1;
  for(p = artList; p->next != NULL; p = p->next) {
    scripting_RunFormat("artpacks[%d] = \"%s\"", i, (char*) p->data);
    i++;
  }
  scripting_Run("setupArtpacks()");
}

void loadArt() {
  char *path;
  char *artpack;

	path = getArtPath("default", "artpack.lua");
  if(path != NULL) {
    scripting_RunFile(path);
    free(path);
  }

  scripting_GetStringSetting("current_artpack", &artpack);
  fprintf(stderr, "[status] loading artpack '%s'\n", artpack);
	
  path = getArtPath(artpack, "artpack.lua");
  free(artpack);

  if(path != NULL) {
    scripting_RunFile(path);
    free(path);
  }

  initTexture(gScreen);
  initFonts();
}

void reloadArt() {
  printf("[status] reloading art\n");
  deleteTextures(gScreen);
  loadArt();
}
    

