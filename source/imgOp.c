#include "imgOp.h"
#include <citro2d.h>
#include <citro3d.h>
#include <stdlib.h>

size_t calculateSwizzledTiledPosition(int x, int y, int width, int tileSize);

C2D_Image createImage(int res)
{
	int w = res;int h = res;//Abreviar las variables para no escribir tanto.

	C3D_Tex *tex = (C3D_Tex*)malloc(sizeof(C3D_Tex));//Reservar memoria para la textura.
	C3D_TexInit(tex,w,h,GPU_RGBA8);

	Tex3DS_SubTexture *subtex = (Tex3DS_SubTexture*)linearAlloc(sizeof(Tex3DS_SubTexture));//Reservar memoria para la subtextura.

	//Ubicar donde se va a dibujar la textura.
	subtex->left   = 0;
	subtex->top    = 0;
	subtex->right  = 1;
	subtex->bottom = 1;
	subtex->width  = w;
	subtex->height = h;
	C3D_TexSetFilter(tex, GPU_LINEAR, GPU_LINEAR);

	return (C2D_Image){tex,subtex};
}

void setPixelRGB8(C2D_Image* image, int y, int x, u8 red, u8 green, u8 blue)
{
	int width = image->tex->width;
    int height = image->tex->height;

    // Verifica los límites de la textura
    if (x < 0 || x >= width || y < 0 || y >= height) {
        printf("Error: Posición (%d, %d) fuera de los límites (%d x %d)\n", x, y, width, height);
        return;
    }

    // Tamaño del tile (usualmente 8x8)
    int tileSize = 8;

    // Calcular la posición tiled + swizzled
    size_t pos = calculateSwizzledTiledPosition(x, y, width, tileSize);

    // Acceder al buffer y escribir los valores RGB
    u8* data = (u8*)image->tex->data;
    size_t pixelOffset = pos * 4;
    data[pixelOffset]     = 255;
    data[pixelOffset + 1] = red;
    data[pixelOffset + 2] = green;
    data[pixelOffset + 3] = blue;
}

size_t calculateSwizzledTiledPosition(int x, int y, int width, int tileSize)
{
    int tilesPerRow = width / tileSize;
    int tileX = x / tileSize;
    int tileY = y / tileSize;

    size_t tileIndex = (tileY * tilesPerRow) + tileX;

    int localX = x % tileSize;
    int localY = y % tileSize;

    // Calcular el número de bits necesarios sin usar log2()
    int numBits = 0;
    for (int temp = tileSize; temp > 1; temp >>= 1)
        numBits++;

    size_t localSwizzled = 0;
    for (int bit = 0; bit < numBits; ++bit) {
        localSwizzled |= ((localX & (1 << bit)) << bit) | ((localY & (1 << bit)) << (bit + 1));
    }

    return (tileIndex * tileSize * tileSize) + localSwizzled;
}

void readBMP(char *dir, C2D_Image *image,int width, int height,int Jx, int Jy,volatile int *flag)
{
	FILE *file = fopen(dir,"rb");
	if(!file)return;

	int Fp = 0;
	fseek(file,10,SEEK_SET);
	if(!fread(&Fp,1,sizeof(u8),file))
	{
		*flag = 3;
		fclose(file);
		return;}
	fseek(file,Fp,SEEK_SET);

	u8 BGR[3];
	int X,Y = 0;

	for(int y = 0; y < height; y+=Jy)
	{
		for(int x = 0; x < width; x+=Jx)
		{
			if(*flag == 2)
			{
				fclose(file);
				return;
			}
			if(!fread(&BGR,3,sizeof(u8),file))
			{
				*flag = 3;
				fclose(file);
				return;}
			fseek(file,ftell(file)+3*(Jy-1),SEEK_SET);//Haciendo esto porque SEEK_CUR no funciona por alguna razon.
			setPixelRGB8(image,X,Y,BGR[0],BGR[1],BGR[2]);
			X++;
		}
		fseek(file,ftell(file)+3*width*(Jy-1),SEEK_SET);
		Y++;X=0;
	}
	fclose(file);
	return;
}

void cleanImage(C2D_Image *image)
{
	u32 size = image->tex->width * image->tex->height;
    u8 *b = (u8*)malloc(size * 4);
    memset(b,0,size * 4);
	C3D_TexUpload(image->tex,b);
    free(b);
}