/*
 * shif.h part of sh-cgi
 * Copyright (C) 2000 V.R.Sanders
 *
 * see sh-cgi.c for details
 */

#define MAX_DIR_ENTRY_NAME_LEN 512

#define rev_long(x)\
            x = (((x & 0xff) << 24) | ((x & 0xff00) << 8) |\
                ((x & 0xff0000) >> 8) | ((x & 0xff000000) >> 24));
typedef struct
{
    long offset;
    long load_addr;
    long exec_addr;
    long size;
    long flags;
    long reserved;
} dir_entry;

typedef struct
{
    long dirtag;
    long blocksize;
    long used;
} dir;

typedef struct
{
    long filetag;
    long filesize;
} file;

long shif_getpage(FILE * image_file,const char * page_name,char**,long*);
int shif_freepage(char * page_buff);
long shif_lfromfile(FILE *,long,int);
dir_entry * shif_finddirentry(FILE *,const char * ,long ,int );
