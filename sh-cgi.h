/*
 * sh-cgi.h part of sh-cgi
 * Copyright (C) 2000 V.R.Sanders
 *
 * see sh-cgi.c for details
 */

#define SH_CGI_ROOT "sh-manuals"
#define DIR_BUF_SIZE 1024

int put_footer(int);

struct treenode
{
    char * filename;
    struct treenode * left;
    struct treenode * right;
};
