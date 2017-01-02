/*
 * sh-cgi a Stronghelp CGI gateway
 * Copyright (C) 2000 V.R.Sanders
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * To contact V Sanders email
 * vince@kyllikki.fluff.org
 *
 * The main web site for this program is
 * http://www.kyllikki.fluff.org/software
 *
 * sh-cgi uses cgi-util LGPL library by Bill Kendrick (see cgi-util directory)
 */

/* standard headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

/* library headers */
#include "cgi-util.h"

/* program headers */
#include "shhtml.h"
#include "shif.h"
#include "sh-cgi.h"

int put_footer(int back_ptr)
{
    printf("<!-- This document was produced by sh-cgi (version "VERSION" Built "BUILDDATE") a cgi gateway for stronghelp files.\n");
    printf("sh-cgi was written by V.R.Sanders (vince@kyllikki.fluff.org).\n");
    printf("sh-cgi is released under GPL version 2 and uses cgi-util a LGPL library by Bill Kendrick.\n");
    printf("sh-cgi homepage is http://www.kyllikki.fluff.org/software -->\n");

    if (back_ptr)
	printf("<hr><div align=left><small>["CGI_NAME"]&nbsp;<a href=\""CGI_NAME"\">Back to list of manuals</a></div>\n");

    return back_ptr;

}

int send_error(const char * err_txt)
{
    printf("<head>\n<title>Error :%s</title>\n</head><body>\n<h1>Error :%s</h1>\n</body>\n",err_txt,err_txt);
    return 0;
}

struct treenode **find_treenode(struct treenode** parent,char * key)
{
    struct treenode * node;
    int result;

    while((node=*parent) !=NULL)
    {
	result=strcmp(key,node->filename);
	if (result>0)
	    parent=&node->right;
	else if (result <0)
	    parent=&node->left;
	else
	    break;
    }
    return parent;
}

int add_treenode(struct treenode** parent,char * new_entry)
{
    struct treenode ** link;
    struct treenode * node;

    link=find_treenode(parent,new_entry);
    if (*link==NULL)
    {
	node = (struct treenode*)malloc(sizeof(struct treenode));
	/* memory error abort the add */
	if (node==NULL)
	    return 1;
	node->left=node->right=NULL;
	node->filename=strdup(new_entry);
	if(node->filename == NULL)
	{
	    /* dont add if strdup failed */
	    free(node);
	    return 1;
	}
	else
	    *link=node;
    }
    else
    {
	/* already in tree - ignore atm */
	return 1;
    }
    return 0;
}

int walk_tree(struct treenode* node)
{
    if (node->left != NULL)
	walk_tree(node->left);

    printf("<p>");
    put_sh_href(node->filename,"",node->filename);
    printf("\n");

    if (node->right != NULL)
	walk_tree(node->right);

    free(node->filename);
    free(node);

    return 0;
}


int sh_manual_list(char* doc_root)
{
    int file_name_len = 0;
    char * manual_path;
    struct stat manual_dir;
    struct treenode * treeroot=NULL;

    /* for dir reading */
    DIR *manual_dir_stream;
    struct dirent *dir_entry;


    /* find length of full manual path (extra 4 for termination and separators) */
    file_name_len = strlen(doc_root) + strlen(SH_CGI_ROOT) + 4;

    manual_path=malloc(file_name_len);
    if (manual_path==NULL)
    {
	send_error("Unable to claim temporary memory");
	return -1;
    }
    strcpy(manual_path,doc_root);
    strcat(manual_path,"/");
    strcat(manual_path,SH_CGI_ROOT);

    /* check if manual directory exists */
    if (stat(manual_path,&manual_dir)==-1)
    {
	send_error("Manual Directory doesnt exist or is not acesible");
	return -2;
    }

    if (!S_ISDIR(manual_dir.st_mode))
    {
	send_error("Manual Directory is not a Directory!");
	return -3;
    }

    /* check for files in directory */
    manual_dir_stream=opendir(manual_path);
    if (manual_dir_stream==NULL)
    {
	send_error("Manual Directory open failed");
	return -4;
    }

    printf("<head>\n<title>List of available StrongHelp manuals</title>\n</head><BODY BGColor=\"#DDDDDD\" TEXT=\"#000000\" BackGround=\"/"SH_CGI_ROOT"/."BACKGROUND_IMAGE"\">\n<h1>List of available StrongHelp manuals</h1>\n");

    dir_entry=readdir(manual_dir_stream);
    while(dir_entry!=NULL)
    {
	if (index(dir_entry->d_name,'.')==NULL)
	    add_treenode(&treeroot,dir_entry->d_name);

	dir_entry=readdir(manual_dir_stream);
    }

    walk_tree(treeroot);

    put_footer(0);

    printf("</body>");

    free(manual_path);

    return 0;
}

FILE * open_manual(const char * doc_root, const char *manual_name)
{
    int file_name_len = 0;
    char * manual_path;
    FILE * manual_file;
    struct stat manual_dir;

    /* find length of full manual path (extra 4 for termination and separators) */
    file_name_len = strlen(doc_root) + strlen(SH_CGI_ROOT) + strlen(manual_name) + 4;

    manual_path=malloc(file_name_len);
    if (manual_path==NULL) {
	send_error("Unable to claim temporary memory");
	return NULL;
    }
    strcpy(manual_path,doc_root);
    strcat(manual_path,"/");
    strcat(manual_path,SH_CGI_ROOT);

    /* check if manual directory exists */
    if (stat(manual_path,&manual_dir)==-1) {
	send_error("Manual Directory doesnt exist or is not acesible");
	return NULL;
    }

    if (!S_ISDIR(manual_dir.st_mode)) {
	send_error("Manual Directory is not a Directory!");
	return NULL;
    }

    /* manual directory exists and is readable so lets look for file */
    strcat(manual_path,"/");
    strcat(manual_path,manual_name);

    manual_file = fopen(manual_path,"rb");

    free(manual_path);

    return manual_file;
}

int close_manual(FILE * manual_file)
{
    return fclose(manual_file);
}

int
sh_show_page(const char *doc_root,
             const char *manual_name,
             const char *page_name)
{

    FILE * manual_file;
    char * page_text = NULL;
    long page_len;
    long ret_val;

    if (manual_name==NULL)
    {
	send_error("Null manual cannot be acessed");
	return 1;
    }

    manual_file=open_manual(doc_root,manual_name);
    if (manual_file==NULL)
    {
	printf("Manual %s cannot be acessed\n",manual_name);
	return 1;
    }

    /* strong help from image file get page */
    ret_val=shif_getpage(manual_file,page_name,&page_text,&page_len);

    if (ret_val <0)
    {
	switch(ret_val)
	{
	case -1:
	    send_error("Unable to parse image file header - Is it a Stronghelp Manual?");
	    return 1;
	case -2:
	    send_error("Unable to find requested page in manual!");
	    return 1;
	case -3:
	    send_error("Bad \"DATA\" block magic number - Manual corrupt?");
	    return 1;
	case -4:
	    send_error("Unable to allocate temporary storage!");
	    return 1;
	case -5:
	    send_error("Short read from image file - Manual corrupt?");
	    return 1;
	}
    }

    /* convert and display html from input stronghelp file */
    html_output(manual_file,manual_name,page_text,page_len);

    /* stronghelp from image file free page */
    shif_freepage(page_text);

    /* done with file */
    close_manual(manual_file);

    return 0;
}


int main(int argc, char * argv[])
{
  int res;
    char * doc_root;

  /* Initialize the CGI and send out an HTTP header: */

  res = cgi_init();

  printf("Content-type: text/html\n\n");


  /* Was there an error initializing the CGI??? */

  if (res != CGIERR_NONE)
    {
      printf("Error # %d: %s<p>\n", res, cgi_strerror(res));
      exit(0);
    }

    doc_root=getenv("DOCUMENT_ROOT");
    if (doc_root == NULL)
    {
      send_error("Document Root Not Found");
      exit(0);
    }

  /* boilerplate top here */
    printf("<html>\n");


  /* check if we need to display available manuals */

  if (cgi_getentrystr("manual") == NULL)
  {
      /* no manual specified so list available ones */
      sh_manual_list(doc_root);
  }
  else
  {
      sh_show_page(doc_root,cgi_getentrystr("manual"),cgi_getentrystr("page"));
  }

  /* boilerplate bottom here */
    printf("</html>\n");

  cgi_quit();

  return(0);
}
