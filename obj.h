#ifndef OBJ_H
#define OBJ_H

typedef struct {
    float *vertices;
    int *faces;
    int nvertex;
    int nface;
} OBJModel;

OBJModel modelget(char *);

#endif
