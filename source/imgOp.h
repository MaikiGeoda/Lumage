#ifndef IMGOP_H
#define IMGOP_H
#include <citro2d.h>

C2D_Image createImage(int res);
void setPixelRGB8(C2D_Image* image, int y, int x, u8 red, u8 green, u8 blue);
void readBMP(char *dir, C2D_Image *image,int width, int height,int Jx, int Jy,volatile int *flag);
void cleanImage(C2D_Image *image);
#endif