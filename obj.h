#ifndef OBJ_H
#define OBJ_H

typedef struct {
    float *vertices;
    int *faces;
    int vertexc;
    int facec;
} OBJModel;

OBJModel modelget(void);

#endif
