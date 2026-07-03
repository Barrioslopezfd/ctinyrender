#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "obj.h"

OBJModel modelget(char *objdir)
{
  OBJModel model = {
    .vertices = mmap(NULL, sizeof(float) * 3 * 100000, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0),
    .faces = mmap(NULL, sizeof(int) * 3 * 100000, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0),
    .nvertex = 0,
    .nface = 0,
  };

  FILE *f = fopen(objdir, "r");
  char line[256];
  int vc = 0;
  int fc = 0;
  while (fgets(line, sizeof(line), f))
  {
    switch (line[0]) {
      case 'v':
        if (line[1] != ' ') break;
        float x, y ,z;
        sscanf(line, "v %f %f %f", &x, &y, &z);
        model.vertices[vc]      = x;
        model.vertices[vc + 1]  = y;
        model.vertices[vc + 2]  = z;
        vc+=3;
        break;
      case 'f':
        sscanf(line, "f %d/%*d/%*d %d/%*d/%*d %d/%*d/%*d", &model.faces[fc], &model.faces[fc+1], &model.faces[fc+2]);
        fc+=3;
        break;
    }
  }
  model.nvertex = vc;
  model.nface = fc;
  fclose(f);
  return model;
}


