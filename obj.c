#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "obj.h"

OBJModel modelget(void)
{
  OBJModel model = {
    .vertices = mmap(NULL, sizeof(float) * 3 * 100000, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0),
    .faces = mmap(NULL, sizeof(int) * 3 * 100000, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0),
    .vertexc = 0,
    .facec = 0,
  };

  FILE *f = fopen("obj/diablo3_pose/diablo3_pose.obj", "r");
  char line[256];
  int vc = 0;
  int fc = 0;
  while (fgets(line, sizeof(line), f))
  {
    switch (line[0]) {
      case 'v':
        if (line[1] != ' ') break;
        sscanf(line, "v %f %f %f", &model.vertices[vc], &model.vertices[vc+1], &model.vertices[vc+2]);
        vc+=3;
        break;
      case 'f':
        sscanf(line, "f %d/%*d/%*d %d/%*d/%*d %d/%*d/%*d", &model.faces[fc], &model.faces[fc+1], &model.faces[fc+2]);
        fc+=3;
        break;
    }
  }
  model.vertexc = vc;
  model.facec = fc;
  // printf("vertexc = %d\n", model.vertexc);
  // printf("facec = %d\n", model.facec);
  fclose(f);
  return model;
}


