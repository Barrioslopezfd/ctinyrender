#ifndef BMPIMAGE_H
#define BMPIMAGE_H

#include <stdint.h>
#include <stdio.h>

#define BPP 4

typedef struct __attribute__((packed)) {
  char BFType[2]; // First 2 byts in bitmap file have to be BM
  uint32_t BFSize;
  uint16_t BFReserved1;
  uint16_t BFReserved2;
  uint32_t BFOffBit;
} BFHeader; // Bitmap File Header

typedef struct __attribute__((packed)) {
  uint32_t BISize;
  int32_t  BIWidth;
  int32_t  BIHeight;
  uint16_t BIPlanes;
  uint16_t BIBitCount;
  uint32_t BICompression;
  uint32_t BISizeImage;
  uint32_t BIXPelsPerMeter; //|
  uint32_t BIYPelsPerMeter; //| For printers so we ignore it
  uint32_t BIClrUsed;
  uint32_t BIClrImportant;
} BIHeader; // Bitmap Info Header 

#define BFSIZE sizeof(BFHeader)
#define BISIZE sizeof(BIHeader)

typedef struct {
  BFHeader BFH;
  BIHeader BIH;
  uint32_t *pixels;
} BMImage;

BMImage BMSet(int, int);
FILE *BMCreate(char *);
void BMWrite(BMImage *, FILE *);
void BMSetPixel(BMImage *, int, int, uint32_t);

#endif
