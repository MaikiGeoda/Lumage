#ifndef GUI_H
#define GUI_H
#include <stdbool.h>
#include <citro2d.h>

typedef struct
{
	int X;
	int Y;
	int W;
	int Z;
	bool pressed;
	int state;
}Button;
typedef struct
{
	C2D_Image img1;
	C2D_Image img2;
	C2D_SpriteSheet sheet;
}ButtonSurface;
typedef struct
{
	float degrees[3];
	float initDegrees[3];
	int width,height;
	int X,Y;
	int betwDistance;
	int stop;
}loadingSquares;
typedef struct
{
	C2D_Image *imgs;
	int actualFrame;
	int timer;
	int changeFrames;
	int maxFrames;
	C2D_SpriteSheet sheet;
	int opacity;
}animationObject;
void createButton(Button *btn,int left,int top,int width,int height);
int isPressed(Button *btn,int X, int Y);
void drawSurfaceButton(Button *btn,ButtonSurface *surface);
void createSurface(ButtonSurface *surface,char* dir, int I1,int I2);
void createLoading(loadingSquares *Sq,int x,int y,int w,int h,int between,float D1,float D2, float D3);
void updateLoading(loadingSquares *Sq,float Deg,float stopDegree);
void drawLoading(loadingSquares *Sq,float mult,u32 Color);
void createAnimation(animationObject *Anm,int max,int every,char* dir);
void updateAnimation(animationObject *Anm);
void drawAnimation(animationObject *Anm,int x,int y,float depth);
void freeAnimation(animationObject *Anm);

#endif