#include "GUI.h"
#include <citro2d.h>
#include <citro3d.h>
#include <math.h>
#include <stdlib.h>

void createButton(Button *btn,int left,int top,int width,int height)
{
	btn->X = left;
	btn->Y = top;
	btn->W = width;
	btn->Z = height;
	btn->state = 0;
}
int isPressed(Button *btn,int X, int Y)
{
	bool xAxis = btn->X <= X && X <= btn->X+btn->W;
	bool yAxis = btn->Y <= Y && Y <= btn->Y+btn->Z;
	bool bAxis = xAxis && yAxis;
	if(btn->state == 2)
	{
		btn->state = 0;
	}
	else if(bAxis && btn->state == 0)
	{
		btn->state = 1;
	}
	else if(!bAxis && btn->state == 1)
	{
		btn->state = 2;
	}
	return btn->state;
}
void drawSurfaceButton(Button *btn,ButtonSurface *surface)
{
	if(btn->state == 0)C2D_DrawImageAt(surface->img1,btn->X,btn->Y,0,NULL,1,1);
	else C2D_DrawImageAt(surface->img2,btn->X,btn->Y,0,NULL,1,1);
}
void createSurface(ButtonSurface *surface,char* dir, int I1,int I2)
{
	surface->sheet = C2D_SpriteSheetLoad(dir);
	surface->img1 = C2D_SpriteSheetGetImage(surface->sheet,I1);
	surface->img2 = C2D_SpriteSheetGetImage(surface->sheet,I2);
}
void createLoading(loadingSquares *Sq,int x,int y,int w,int h,int between,float D1,float D2, float D3)
{
	Sq->X = x;Sq->Y = y;
	Sq->width = w;Sq->height = h;
	Sq->betwDistance = between;
	Sq->degrees[0]=D1;Sq->degrees[1]=D2;Sq->degrees[2]=D3;
	Sq->initDegrees[0]=D1;Sq->initDegrees[1]=D2;Sq->initDegrees[2]=D3;
	Sq->stop = false;
}
void updateLoading(loadingSquares *Sq,float Deg,float stopDegree)
{
	int allCorrect = 0;
	for(int i = 0; i < 3; i++)
	{
		if(Sq->stop == 0)Sq->degrees[i] += Deg;
		else if(Sq->stop == 2)
		{
			if(Sq->degrees[i] < Sq->initDegrees[i]-Deg || Sq->degrees[i] > Deg+Sq->initDegrees[i])Sq->degrees[i] += Deg;
			else allCorrect += 1;
		}
		else
		{
			if(Sq->degrees[i] < stopDegree-Deg || Sq->degrees[i] > Deg+stopDegree)Sq->degrees[i] += Deg;
			else Sq->degrees[i] = stopDegree;
		}
		if(Sq->degrees[i] > M_PI*2)
		{
			Sq->degrees[i] -= M_PI*2;
		}
	}
	if(allCorrect == 3)Sq->stop = 0;
}
void drawLoading(loadingSquares *Sq,float mult,u32 Color)
{
	int distance = 0; 
	for(int i = 0; i < 3; i++)
	{
		C2D_DrawRectSolid(Sq->X+distance,Sq->Y+cos(Sq->degrees[i])*mult,0,Sq->width,Sq->height,Color);
		distance+=Sq->betwDistance;
	}
}
void createAnimation(animationObject *Anm,int max,int every,char* dir)
{
	Anm->imgs = (C2D_Image*)malloc(sizeof(C2D_Image)*max);
	Anm->sheet = C2D_SpriteSheetLoad(dir);
	Anm->changeFrames = every;
	Anm->maxFrames = max;
	Anm->actualFrame = 0;
	Anm->opacity = 0;
	for(int i = 0; i < max; i++)
	{
		Anm->imgs[i] = C2D_SpriteSheetGetImage(Anm->sheet,i);
	}
}
void updateAnimation(animationObject *Anm)
{
	Anm->timer += 1;
	if(Anm->timer == Anm->changeFrames)
	{
		Anm->timer = 0;
		Anm->actualFrame+=1;
	}
	if(Anm->actualFrame == Anm->maxFrames)Anm->actualFrame = 0;
}
void drawAnimation(animationObject *Anm,int x,int y,float depth)
{
	C2D_ImageTint tint;
	C2D_PlainImageTint(&tint,C2D_Color32(0,0,0,Anm->opacity),0);
	C2D_DrawImageAt(Anm->imgs[Anm->actualFrame],x,y,depth,&tint,1,1);
}
void freeAnimation(animationObject *Anm)
{
	free(Anm->imgs);
}