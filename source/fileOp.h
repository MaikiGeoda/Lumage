#ifndef FILEOP_H
#define FILEOP_H

typedef struct
{
    char *Updir;
    char *Dndir;
}linked;

bool compPiece(char *str1,char *str2,size_t piece);
int makeLove(char **Up, char **Down,linked *Imgs,int size);
int scanDirectory(char *path,char **Up,char **Dn);

#endif