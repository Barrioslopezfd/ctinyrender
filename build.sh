#!/usr/bin/env sh

bear -- gcc -lX11 main.c bmpimage.c util.c obj.c && time ./a.out
