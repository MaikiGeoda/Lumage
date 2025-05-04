#include <3ds.h>
#include "fileOp.h"
#include <string.h>
#include <dirent.h>
#include <stdio.h>

bool compPiece(char *str1,char *str2,size_t piece)
{
    if(strlen(str1) < piece || strlen(str2) < piece)return false;
    for(int i = 0;i < piece; i++)
    {
        if(str1[i] != str2[i])return false;
    }
    return true;
}
int makeLove(char **Up, char **Down,linked *Imgs,int size)
{
    int count = 0;
    for(int U = 0; U < size; U++)
    {
        for(int D = 0; D < size; D++)
        {
            if(compPiece(Up[U],Down[D],strlen(Up[U])-7))
            {
                Imgs[count].Updir = strdup(Up[U]);
                Imgs[count].Dndir = strdup(Down[D]);
                count++;
            }
        }
    }
    return count;
}
int scanDirectory(char *path,char **Up,char **Dn)
{
    struct dirent* entry;
    DIR *dp = opendir(path);
    char top[] = "top.bmp";
    char btm[] = "bot.bmp";

    int Uc = 0;
    int Dc = 0;
    while((entry = readdir(dp)))
    {
        if(strcmp(entry->d_name+strlen(entry->d_name)-7,top) == 0)
        {
            printf("This is top: %s\n",entry->d_name);
            Up[Uc] = strdup(entry->d_name);
            Uc++;
        }
        if(strcmp(entry->d_name+strlen(entry->d_name)-7,btm) == 0)
        {
            printf("This is bottom: %s\n",entry->d_name);
            Dn[Dc] = strdup(entry->d_name);
            Dc++;
        }
    }
    closedir(dp);
    if(Uc > Dc)return Uc;
    else return Dc;
}