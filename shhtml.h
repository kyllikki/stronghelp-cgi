/*
 * shhtml.h part of sh-cgi
 * Copyright (C) 2000 V.R.Sanders
 *
 * see sh-cgi.c for details
 */

/* defines */
#define BACKGROUND_IMAGE "background.gif"

/* font tags */
#define FONT_CODE_TAG "TT"
#define FONT_CITE_TAG "TT"

#define STRONG_TAG "b"
#define EMPHASIS_TAG "em"
#define UNDERLINE_TAG "u"


#define MAX_ENTITY_LEN 16

typedef struct
{
    int bold;
    int emph;
    int align;
    int underline;
    int table;
    char *ftag;
    FILE * imagef;
    char parent[1024];
    char prefix[1024];
    char postfix[1024];
    char manual_name[1024];
} shhtml_status;


/* function protoypes */
int html_output(FILE *,char *,char *,long);
long html_process_line(char *,shhtml_status *);
