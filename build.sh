#!/usr/bin/env sh

bear -- gcc -lX11 -lm main.c util.c obj.c && time ./a.out
