#!/usr/bin/env sh

bear -- gcc -fopenmp main.c bmpimage.c util.c obj.c && time ./a.out
