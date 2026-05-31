#include "bmpimage.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

BMImage BMSet(int width, int height) {
  BFHeader BFH = {
    .BFType = { 'B', 'M' },
    .BFSize = BFSIZE + BISIZE + (width * height * BPP),
    .BFReserved1 = 0,
    .BFReserved2 = 0,
    .BFOffBit = BFSIZE + BISIZE,
  };

  BIHeader BIH = {
    .BISize = BISIZE,
    .BIWidth = width,
    .BIHeight = height,
    .BIPlanes = 1,
    .BIBitCount = 32,
    .BICompression = 0,
    .BISizeImage = width * height * BPP,
    .BIXPelsPerMeter = 0,
    .BIYPelsPerMeter = 0,
    .BIClrUsed = 0,
    .BIClrImportant = 0,
  };

  uint32_t *pixels = malloc(width * height * sizeof(uint32_t));
  if (!pixels) {
    fprintf(stderr, "Malloc failed for pixels\n");
    exit(1);
  };

  BMImage BMI = {
    .BFH = BFH,
    .BIH = BIH,
    .pixels = pixels,
  };

  return BMI;
}

ZBuffer ZBSet(int width, int height) {
    long pagesize  = sysconf(_SC_PAGESIZE);
    size_t bpx      = width * height * sizeof(int32_t);
    size_t size = ((bpx + pagesize - 1) / pagesize) * pagesize;

    ZBuffer ZBuff = mmap(
        NULL,
        size,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1,
        0
        );

    if (ZBuff == MAP_FAILED) {
        perror("mmap failed for zbuffer");
        exit(1);
    }

    int total_pixels = width * height;
    for (int i = 0; i < total_pixels; i++) {
        ZBuff[i] = 0;
    }

    return ZBuff;
}

FILE *BMCreate(char *fname) {
  return fopen(fname, "wb");
};

void BMWrite(BMImage *BMI, FILE *f){
  fwrite(&BMI->BFH, BFSIZE, 1, f);
  fwrite(&BMI->BIH, BISIZE, 1, f);
  fwrite(BMI->pixels, BMI->BIH.BISizeImage, 1, f);
};

void BMSetPixel(BMImage *BMI, int x, int y, uint32_t color) {
  if (x >= 0 && x < BMI->BIH.BIWidth && y >= 0 && y < BMI->BIH.BIHeight) {
    BMI->pixels[y * BMI->BIH.BIWidth + x] = color;
  }
}
