/*
 * shif.c part of sh-cgi
 * Copyright (C) 2000 V.R.Sanders
 *
 * see sh-cgi.c for details
 *
 *
 * routines to manipulate strong help image files
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include "shif.h"

long lltoh(long value,int bsex)
{
    /* convert little endian long to long in indicated byte sex*/
    if (bsex!=0)
	/* need byte swap */
	rev_long(value);

    return value;
}

long shif_lfromfile(FILE * imagef,long offset,int bsex)
{
    long value;
    fseek(imagef,offset,SEEK_SET);
    if (fread(&value,1,sizeof(long),imagef) != sizeof(long))
	return -1;

    return(lltoh(value,bsex));
}


int shif_check_head(FILE * imagef)
{
    char * buffer;
    int * value;
    int bsex=-1;

    buffer=malloc(16);
    if (buffer==NULL)
	return -1;

    /* load first 16 bytes of file into buffer check we read it all */
    fseek(imagef,0,SEEK_SET);
    if (fread(buffer,1,16,imagef) !=16)
    {
	free(buffer);
	return -1;
    }
    /* 504c4548 - little end HELP */

    value = (int *)buffer;

    if (*value == 0x504c4548)
    {
	/* ok host is little endian */
	bsex = 0;
    }
    else if (*value == 0x48454c50)
    {
	bsex = 1;
    }
    else
    {
	free(buffer);
	return -1;
    }

    /* next check header length is correct (stronghelp used to incorectaly set this)*/
    value++;
    if (lltoh(*value,bsex) == 44 || (lltoh(*value,bsex) == 40))
    {
	free(buffer);
	return bsex;
    }
    else
    {
	free(buffer);
	return -1;
    }

    free(buffer);
    return -1;
}

long shif_sfromfile(FILE *imagef,long offset,int bsex,char *buff,int bufflen)
{
    fseek(imagef,offset,SEEK_SET);
    fread(buff,1,bufflen,imagef);
    /* we add 4 to alow for termintor!*/
    return (strlen(buff)+4)&(~3);
}

dir_entry * shif_make_dir_entry(FILE *imagef,long offset,int bsex)
{
    dir_entry * entry;
    entry=malloc(sizeof(dir_entry));
    if (entry==NULL)
	return NULL;

    entry->offset = shif_lfromfile(imagef,offset,bsex);
    entry->load_addr = shif_lfromfile(imagef,offset+=4,bsex);
    entry->exec_addr = shif_lfromfile(imagef,offset+=4,bsex);
    entry->size = shif_lfromfile(imagef,offset+=4,bsex);
    entry->flags = shif_lfromfile(imagef,offset+=4,bsex);

    return entry;

}


dir_entry * shif_finddirentry(FILE * imagef,const char * page_name,long offset,int bsex)
{
    /* recursive directory entry finding routine
     * in theory this routine takes a file offset and a page name and finds the
     * asociated directory entry if present or null if not or will call itself with a
     * sub directory if thats the best partial match
     */

    /* stratagy is search directory elements
     * - if the page name is null return the !root entry
     * - if the page name contains a directory path find that directory
     *     and call ourselves with it clipping the page name as required
     * - if its a file and matches return the dir_entry
     * - if its a directory and is an exact match call ourselves with that
     *     directorys offset and a null page_name
     * - if its a directory and its name matches a portion of the start of
     *     the page name and we havnt seen a directory with a better match
     *     already set it to best match after every entry has been scanned
     *     call ourselves with the best match and the rest of the page name
     */
    static char root_page[]="!root";
    long dir_extent =0;
    long cur_dir_entry=0;
    long dir_entry_name_len;
    long best_match_dir_entry=-1;
    long best_match_dir_entry_len=0;

    char dir_entry_name[MAX_DIR_ENTRY_NAME_LEN];

    /* check for DIR$ magic */
    if (shif_lfromfile(imagef,offset,bsex)!=0x24524944)
    {
	printf("<-- bad dir magic :%lx >",shif_lfromfile(imagef,offset,bsex));
	return NULL;
    }

    if (page_name == NULL)
	page_name = root_page;

    if (index(page_name,'.')!=NULL)
    {
	/*
	 * if the page name contains a directory path find that directory
	 * and call ourselves with it clipping the page name as required
	 */
    }

    dir_extent=shif_lfromfile(imagef,offset+8,bsex);

    cur_dir_entry=12;

    while(cur_dir_entry<dir_extent)
    {
	dir_entry_name_len=shif_sfromfile(imagef,offset+cur_dir_entry+24,bsex,dir_entry_name,MAX_DIR_ENTRY_NAME_LEN);

	/* look for an exact filename match */
	if (strcasecmp(page_name,dir_entry_name)==0)
	{
	    if(!(shif_lfromfile(imagef,(offset+cur_dir_entry+16),bsex) &0x100))
		/* - if its a file and matches return the dir_entry */
		return(shif_make_dir_entry(imagef,offset+cur_dir_entry,bsex));
	    else
		/* - if its a directory and is an exact match call ourselves
		 * with that directorys offset and a null page_name
		 */
		return shif_finddirentry(imagef,NULL,shif_lfromfile(imagef,offset+cur_dir_entry,bsex),bsex);
	}

	/* look for a best match */
	if ((strncasecmp(page_name,dir_entry_name,strlen(dir_entry_name))==0)
	    && (dir_entry_name_len>best_match_dir_entry_len)
	    && (shif_lfromfile(imagef,(offset+cur_dir_entry+16),bsex) &0x100))
	{
	    /* ok a match for all of the dir entry*/
	    best_match_dir_entry=cur_dir_entry;
	    best_match_dir_entry_len=dir_entry_name_len;
	}

	/* point to next dir entry */
	cur_dir_entry=cur_dir_entry+24+dir_entry_name_len;

    }

    /* check to see if we have a best match */
    if (best_match_dir_entry!=-1)
    {
	dir_entry_name_len=shif_sfromfile(imagef,offset+best_match_dir_entry+24,bsex,dir_entry_name,MAX_DIR_ENTRY_NAME_LEN);

	return shif_finddirentry(imagef,&page_name[strlen(dir_entry_name)],shif_lfromfile(imagef,offset+best_match_dir_entry,bsex),bsex);
    }

    /* not found best match yet so lets try to find one with [] round it */

    cur_dir_entry=12;

    while(cur_dir_entry<dir_extent)
    {
	dir_entry_name_len=shif_sfromfile(imagef,offset+cur_dir_entry+24,bsex,dir_entry_name,MAX_DIR_ENTRY_NAME_LEN);
	/* look for a best match inside [] brackets*/
	if (dir_entry_name[0]=='['
	    && dir_entry_name[strlen(dir_entry_name)-1]==']'
	    && (strncasecmp(page_name,&dir_entry_name[1],strlen(dir_entry_name)-2)==0)
	    && (dir_entry_name_len>best_match_dir_entry_len)
	    && (shif_lfromfile(imagef,(offset+cur_dir_entry+16),bsex) &0x100))
	{
	    /* ok a match for all of the dir entry*/
	    best_match_dir_entry=cur_dir_entry;
	    best_match_dir_entry_len=dir_entry_name_len;
	}

	/* point to next dir entry */
	cur_dir_entry=cur_dir_entry+24+dir_entry_name_len;

    }

    /* check to see if we have a best [] surrounded match */
    if (best_match_dir_entry!=-1)
    {
	dir_entry_name_len=shif_sfromfile(imagef,offset+best_match_dir_entry+24,bsex,dir_entry_name,MAX_DIR_ENTRY_NAME_LEN);

	/* find page within best match directory
	 *   unlike previous best match we pass the whole page name
	 */
	return shif_finddirentry(imagef,page_name,shif_lfromfile(imagef,offset+best_match_dir_entry,bsex),bsex);
    }

    /* give up - the page isnt there */
    return NULL;

}

long shif_get_data(FILE * imagef,dir_entry * page_dir_entry,char **page_block,long *page_len,int bsex)
{
    /* check for DATA magic */
    if (shif_lfromfile(imagef,page_dir_entry->offset,bsex)!=0x41544144)
    {
	printf("<-- bad data magic, was %lx >",shif_lfromfile(imagef,page_dir_entry->offset,bsex));

	return -3;
    }
    /* allow for header bytes */
    page_dir_entry->size-=8;

    *page_block=malloc(page_dir_entry->size);

    if (*page_block==NULL)
	return -4;

    fseek(imagef,page_dir_entry->offset+8,SEEK_SET);
    if (fread(*page_block,1,page_dir_entry->size,imagef) != page_dir_entry->size)
    {
	/* short returned data */
	free(*page_block);
	*page_block = NULL;
	return -5;
    }
    else
	*page_len=page_dir_entry->size;

    return 0;

}


long shif_getpage(FILE * imagef,const char * page_name,char ** page_block,long * page_len)
{
    /* byte sex 0 is little */
    int bsex=0;
    dir_entry * page_dir_entry;
    int ret_val;

    *page_block = NULL;
    *page_len=0;

    /* check image header and bail if we dont recognise */
    bsex=shif_check_head(imagef);
    if (bsex==-1)
	return -1;

    /* ok we have a stronghelp file and the systems bytesex! */

    page_dir_entry=shif_finddirentry(imagef,page_name,shif_lfromfile(imagef,16,bsex),bsex);
    if (page_dir_entry==NULL)
	return -2;

    ret_val = shif_get_data(imagef,page_dir_entry,page_block,page_len,bsex);

    free(page_dir_entry);

    return ret_val;


}

int shif_freepage(char * page_buff)
{
    if (page_buff != NULL)
	free(page_buff);
    return 0;
}
