#include <3ds.h>
#include <citro2d.h>
#include <citro3d.h>
#include <pthread.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "imgOp.h"
#include "GUI.h"
#include "fileOp.h"

bool DEBUG_MODE = false;

#define LINES 6

typedef struct
{
    C2D_Image Image;
    char *dir;
    int width,height;
    int Jx,Jy;
    volatile int flag;
    int LId;
}ThreadData;
typedef struct
{
    ThreadData Img;
    Button button;
    ButtonSurface surface;
    int x,y;
    int id;
}ImgPreview;

LightLock locks[10];
Thread threads[10];

void loadImageThread(void *arg)
{
    ThreadData *data = (ThreadData*)arg;
    LightLock_Lock(&locks[data->LId]);
    data->flag = 1;
    cleanImage(&data->Image);
    readBMP(data->dir, &data->Image, data->width, data->height,data->Jx,data->Jy,&data->flag);
    if(data->flag < 3)data->flag = 0;
    LightLock_Unlock(&locks[data->LId]);
    threadExit(0);
}
int checkPages(int Count, int forPage)
{
    int Pages = 0;
    for(int C = 0;C < Count;C+=forPage)
    {
        if(Count - C > 6)Pages+=1;
    }
    return Pages;
}
int changePage(ImgPreview *previews,linked *LinkedImg,int page,int Count)
{
    int thisPage = 0;
    for(int i = 0;i < LINES; i++)
    {
        if((page*6)+i < Count)
        {
            char top[100] = "sdmc:/luma/screenshots/";
            previews[i].id = (page*6)+i;
            strcat(top,LinkedImg[previews[i].id].Updir);
            previews[i].Img.flag = 2;
            previews[i].Img.dir = strdup(top);
            //threadJoin(threads[previews[i].Img.LId],UINT64_MAX);
            threads[previews[i].Img.LId] = threadCreate(loadImageThread,&previews[i].Img,4096,40,-2,true);
            thisPage += 1;
        }
        else
        {
           previews[i].id = -1; 
        }
    }
    return thisPage;
}
void loadImgPreview(linked *LinkedImg,ImgPreview *previews,int Offsetx,int Offsety,int InitOffX)
{
    int PosX = 0;
    int OffsetX = InitOffX;
    int OffsetY = Offsety;
    int PosY = 0;
    for(int i = 0; i < LINES; i++)
    {
        previews[i].Img.Image = createImage(128);
        previews[i].Img.width = 400;previews[i].Img.height = 240;
        previews[i].Img.Jx = 8;previews[i].Img.Jy = 8;
        previews[i].Img.flag = 0;
        previews[i].Img.LId = i + 2;
        if(PosX == 0) previews[i].x = InitOffX;
        else previews[i].x = (PosX * Offsetx)+OffsetX;
        previews[i].y = (PosY + 48 - 128) + OffsetY;
        createButton(&previews[i].button,previews[i].x,(80 + previews[i].y),80,48);
        PosX+=1;
        if(PosX == 3)
        {
            PosX = 0;
            PosY += 48;
            OffsetX = InitOffX;
            OffsetY += Offsety;
        }
    }
}
void CursorMove(int Sum, int *CursorPos,int Max, bool *Update)
{
    *CursorPos+=Sum;
    while(*CursorPos >= Max)*CursorPos -= Max;
    while(*CursorPos < 0)*CursorPos += Max;
    *Update = true;
}
void initLocks(LightLock *locks,size_t size){for(int i = 0;i < size;i++){LightLock_Init(&locks[i]);}}
int GiveMeYcoord(ThreadData Img,int textureSize,int SizeScale)
{
    return (Img.height*SizeScale) - ((textureSize/Img.Jy)*SizeScale);
}

int main()
{
    romfsInit();
    cfguInit();
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();

    C3D_RenderTarget *left = C2D_CreateScreenTarget(GFX_TOP,GFX_LEFT);
    //C3D_RenderTarget *right = C2D_CreateScreenTarget(GFX_TOP,GFX_RIGHT);
    C3D_RenderTarget *bottom = C2D_CreateScreenTarget(GFX_BOTTOM,GFX_LEFT);

    ThreadData upImg;
    upImg.Image = createImage(512);
    upImg.dir = "";
    upImg.width = 400;upImg.height = 240;
    upImg.Jx = 1;upImg.Jy = 1;
    upImg.flag = 0;
    upImg.LId = 0;

    ThreadData downImg;
    downImg.Image = createImage(512);
    downImg.dir = "";
    downImg.width = 320;downImg.height = 240;
    downImg.Jx = 1;downImg.Jy = 1;
    downImg.flag = 0;
    downImg.LId = 1;
    initLocks(locks,10);

    char *UpPhotos[500];
    char *DownPhotos[500];
    linked LinkedImg[250];

    bool showBottom = false;

    int maxSize = scanDirectory("sdmc:/luma/screenshots",UpPhotos,DownPhotos);
    int Count = makeLove(UpPhotos,DownPhotos,LinkedImg,maxSize);
    int page = 0;

    float previewsCursorD = 0;
    C2D_TextBuf pageBuf = C2D_TextBufNew(4096);
    C2D_Text pageText;

    C2D_TextBuf corruptedTextBuf = C2D_TextBufNew(824);
    C2D_Text corruptedText;
    C2D_TextParse(&corruptedText,corruptedTextBuf,"The file is corrupted");
    C2D_TextOptimize(&corruptedText);
    int corruptedAlpha = 0;

    ImgPreview previews[LINES];
    loadingSquares Sq;

    animationObject Walk;
    createAnimation(&Walk,4,10,"romfs:/GUI2.t3x");

    loadImgPreview(LinkedImg,previews,105,46,15);
    createLoading(&Sq,100,100,30,30,80,0,3.14,0);
    C2D_SpriteSheet GUIsheet = C2D_SpriteSheetLoad("romfs:/GUI.t3x");
    C2D_Image BGimage = C2D_SpriteSheetGetImage(GUIsheet,2);

    if(DEBUG_MODE)consoleInit(GFX_BOTTOM,NULL);

    C2D_ImageTint tint;
    C2D_PlainImageTint(&tint,C2D_Color32(0,0,0,255),0);

    int fade = 0;
    float fading = 255;
    bool canLoad = false;
    int preload = -1;int savedPage = -1;
    int Cursor = 0;int maxCursor = changePage(previews,LinkedImg,0,Count);
    bool UpdateUp = true;bool stopLoad = false;
    while(aptMainLoop())
    {
        hidScanInput();
        u32 kDown = hidKeysDown();
        //u32 kHeld = hidKeysHeld();
        touchPosition Pos;
        hidTouchRead(&Pos);
        bool anotherCheck = downImg.flag == 0 && !stopLoad;
        if(stopLoad) showBottom = false;
        if(kDown & KEY_A && anotherCheck)
        {
            if(showBottom)showBottom = false;
            else showBottom = true;}
        if(kDown & KEY_R)
        {
            page+=1;
            if(Count - (page-1)*6 < LINES)page=0;
            maxCursor = changePage(previews,LinkedImg,page,Count);}

        if(kDown & KEY_L)
        {
            page-=1;
            if(page < 0)page = checkPages(Count,6);
            maxCursor = changePage(previews,LinkedImg,page,Count);}
        if(kDown & KEY_RIGHT)CursorMove(1,&Cursor,maxCursor,&UpdateUp);
        if(kDown & KEY_LEFT)CursorMove(-1,&Cursor,maxCursor,&UpdateUp);
        if(kDown & KEY_UP)CursorMove(-3,&Cursor,maxCursor,&UpdateUp);
        if(kDown & KEY_DOWN)CursorMove(3,&Cursor,maxCursor,&UpdateUp);

        for(int i=0; i < LINES; i++){
            if(!showBottom){
            bool both = UpdateUp && Cursor == i; 
            if(isPressed(&previews[i].button,Pos.px,Pos.py) == 2 || UpdateUp)
            {
                if(both == true)UpdateUp = false;
                if(Cursor != i && previews[i].button.state == 2) Cursor = i;
                if(previews[i].id > -1)
                {
                    preload = Cursor;
                    savedPage = page;
                    if(previews[i].Img.flag == 3)stopLoad = true;
                    else stopLoad = false;
                    if(fade == 3 || fade == 1)fade = 0;
                    if(fade == 0)
                    {
                        fade = 1;
                        char top[100] = "sdmc:/luma/screenshots/";
                        char bot[100] = "sdmc:/luma/screenshots/";
                        strcat(top,LinkedImg[previews[preload].id].Updir);
                        strcat(bot,LinkedImg[previews[preload].id].Dndir);
                        upImg.dir = strdup(top);
                        downImg.dir = strdup(bot);
                    }
                }
            }}}
        if(fade == 1)
        {
            fading -= 20;
            if(fading < 0)
            {
                fading = 0;
                fade = 2;
            }
        }
        if(fade == 2)
        {
            upImg.flag = 2;
            downImg.flag = 2;
            threadJoin(threads[0],UINT64_MAX);
            threadJoin(threads[1],UINT64_MAX);
            threads[0] = threadCreate(loadImageThread,&upImg,4096,24,-1,true);
            canLoad = true;
            downImg.flag = 2;
            fade = 3;
            fading = 0;
        }
        if(fade == 3 && upImg.flag == 0)
        {
           fading += 20;
            if(fading > 255)
            {
                fading = 255;
                fade = 0;
            } 
        }
        if(canLoad)
        {
            if(upImg.flag == 0 && downImg.flag == 2)
            {
                threads[0] = threadCreate(loadImageThread,&downImg,4096,24,-1,true);
            }
        }
        if(stopLoad || upImg.flag == 3)
        {
            Sq.stop = 1;stopLoad = true;
            if(corruptedAlpha < 255)corruptedAlpha += 5;
            if(corruptedAlpha > 255)corruptedAlpha = 255;
        }
        else 
        {
            if(Sq.stop == 1)Sq.stop = 2;
            if(corruptedAlpha > 0)corruptedAlpha -= 15;
            if(corruptedAlpha < 0) corruptedAlpha = 0;
        }
        bool someCheck = downImg.flag != 0 && downImg.flag != 3;
        if(someCheck && !stopLoad)
        {
            if(Walk.opacity < 255)Walk.opacity += 20;
            if(Walk.opacity > 255) Walk.opacity = 255;
        }
        else
        {
            if(Walk.opacity > 0)Walk.opacity -= 20;
            if(Walk.opacity < 0 ) Walk.opacity = 0;
        }

        updateLoading(&Sq,0.2,1.7);
        previewsCursorD += 0.03;
        C2D_TextBufClear(pageBuf);
        char buf[160];
        snprintf(buf,sizeof(buf),"Page %i of %i",page+1,checkPages(Count,6)+1);
        C2D_TextParse(&pageText,pageBuf,buf);
        C2D_TextOptimize(&pageText);
        updateAnimation(&Walk);

        if(previewsCursorD > M_PI*2)previewsCursorD-=M_PI*2;
        C2D_PlainImageTint(&tint,C2D_Color32(0,0,0,fading),0);
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

        C2D_TargetClear(left,C2D_Color32f(0,0,0,1));
        if(!DEBUG_MODE)C2D_TargetClear(bottom,C2D_Color32f(0,0,0,1));

        C2D_SceneBegin(left);
        {
            drawLoading(&Sq,40,C2D_Color32(203,235,98,255));
            if(upImg.flag == 0 || fade != 0)C2D_DrawImageAt(upImg.Image,0,240-512,0,&tint,-1,-1);
            C2D_DrawText(&corruptedText,C2D_WithColor | C2D_AlignCenter,200,200,0.1,0.8,0.8,C2D_Color32(153,175,38,corruptedAlpha));
            //C2D_DrawImageAt(upImg.Image,0,240-512,0,&tint,-1,-1);
            //C2D_DrawImageAt(previews[preload].Img.Image,-4.4,(240-1024)-18,0,NULL,-8.16,-8.16);
        }
        if(!DEBUG_MODE)
        {
            C2D_SceneBegin(bottom);
            C2D_DrawImageAt(BGimage,0,0,0,NULL,1.038,1.34);
            float C = cos(previewsCursorD)*50;
            if(preload > -1 && savedPage == page)C2D_DrawRectSolid(previews[preload].x-5,previews[preload].y+76,0.1,90,58,C2D_Color32(183+C,205+C,68+C,255));
            C2D_DrawText(&pageText,C2D_WithColor,15,200,0.2,0.8,0.8,C2D_Color32(153,175,38,255));
            for(int i = 0; i < LINES; i++)
            {
                //drawSurfaceButton(&previews[i].button,&previews[i].surface);
                if(previews[i].id > -1 && previews[i].Img.flag != 3)C2D_DrawImageAt(previews[i].Img.Image,previews[i].x,previews[i].y-76,0.3,NULL,-1.6,-1.6);
            }
            drawAnimation(&Walk,280,200,0.4);
            bool uAdown = downImg.flag == 0 && upImg.flag == 0;
            if(uAdown && showBottom)C2D_DrawImageAt(downImg.Image,0,240-512,0.5,NULL,-1,-1);

        }
        C3D_FrameEnd(0);
    }
    freeAnimation(&Walk);
    C2D_Fini();
    C3D_Fini();
    gfxExit();
    romfsExit();
}