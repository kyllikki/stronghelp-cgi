
/*
 * shhtml.c part of sh-cgi
 * Copyright (C) 2000 V.R.Sanders
 *
 * see sh-cgi.c for licence details
 *
 * stronghelp to html conversion routines
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

#include "sh-cgi.h"
#include "shhtml.h"
#include "shif.h"

static const char *hexchars = "0123456789ABCDEF";
static const char *formchars = "?=+";

char * escape_url(const char * s)
{
    /* Escape a URL, or bits of one Only escape space, top bit set and special
       form characters. */
    char *buffer = malloc(strlen(s)*3 + 1);
    unsigned char c;
    char *outp;

    outp = buffer;

    while ( (c = *s++) != 0)
    {
	if (c < 32 || c == 127)
	{
	    /* ignore */
	}
	else if (c >= 128 || (strchr (formchars,c) != 0))
	{
	    *outp++ = '%';
	    *outp++ = hexchars[(c>>4) & 0xf];
	    *outp++ = hexchars[(c>>0) & 0xf];
	}
        else if (c == ' ')
        {
            *outp++ = '+';
        }
	else
	{
	    *outp++ = c;
	}
    }

    *outp = 0;

    return buffer;
}

char * html_get_line(char * sh_page,long *cur_offset,long max_offset)
{
    long wrk_offset = *cur_offset;
    char * line_end;
    long tmp_offset = *cur_offset;

    line_end = index(&sh_page[wrk_offset],'\n');

    if (line_end == NULL)
	line_end = index(&sh_page[wrk_offset],'\r');

    /* if we cant find a lf or cr simply get the rest of the block */
    if (line_end==NULL)
	line_end=&sh_page[max_offset];

    /* find the end of line offset */
    wrk_offset = line_end - sh_page;

    /* ensure we dont overrun the file */
    if (wrk_offset>max_offset)
	wrk_offset=max_offset;

    /* null terminate line so it can be used by standard c string operators */
    sh_page[wrk_offset++] = 0;

    /* skip any trailing junk */
    /*    for (;(wrk_offset<max_offset) && (sh_page[wrk_offset]<32);wrk_offset++)
	  sh_page[wrk_offset] = 0;*/

    *cur_offset=wrk_offset;
    return &sh_page[tmp_offset];

}
char * convert_entity(unsigned char src_chr,char * buff)
{
    switch(src_chr)
    {
    case '\"' : /* These characters are pased through unchanged. */
    case '#' :
    case '{' :
    case '}' :
    case '\\' :
    case '/' :
    case '_' :
    case '*' :
    case '=' :
    case '-' : buff[0] = src_chr; buff[1] = '\0'; break;

    case '<' : strcpy(buff,"&lt;"); break;
    case '>' : strcpy(buff,"&gt;"); break;
    case '&' : strcpy(buff,"&amp;"); break;
    case 9 : strcpy(buff,"&nbsp;&nbsp;"); break;
    case 0x8f : strcpy(buff,"&#183;"); break;
    case 0xdc : strcpy(buff,"*"); break;
    case 0xde : strcpy(buff,"*"); break;
    default: buff[0]=0; break;
    }
    /* return the end of the string */
    return &buff[strlen(buff)];
}

long html_put_entity(char src_chr)
{
    char temp[MAX_ENTITY_LEN];
    convert_entity(src_chr,temp);
    printf("%s",temp);
    return 0;
}

char *html_deescape_text(char * text)
{
    int loop;
    int link_len;
    char *deesc_text;
    char *cur_text;

    link_len=strlen(text);

    /* allow for every char to  be expanded to longest entity - not that big */
    deesc_text=malloc(link_len*MAX_ENTITY_LEN);
    if (deesc_text==NULL)
    return NULL;

    cur_text=deesc_text;

    for(loop=0;loop<link_len;loop++)
    {
	/* this check for < is specificaly for the font text stuff which
	 * assumes the < char after the : doesnt need escaping - very odd
	 */
	if (text[loop]=='<')
	    cur_text=convert_entity(text[loop],cur_text);
	else
	{
	    if (text[loop]=='\\')
	    {
		loop++;
		cur_text=convert_entity(text[loop],cur_text);
	    }
	    else
	    {
		cur_text[0]=text[loop];
		cur_text++;
	    }
	}
    }
    /* terminate the string */
    cur_text[0]=0;
    return deesc_text;
}

int put_href(char * url,char * link_text)
{
    char * lnk_txt;
    /* we need to de-escape the link text here */
    lnk_txt=html_deescape_text(link_text);
    if (lnk_txt!=NULL)
    {
	printf("<a href=%s>%s</a>\n",url,lnk_txt);

	free(lnk_txt);
    }
    else
	printf("<!-- ran out of memory whilst producing link text -->\n");

    return 0;
}

int put_sh_href(char * manual_url,char * page_url,char * link_text)
{
    char *p;
    char *href_url;
    char *e_manual_url = NULL;
    char *e_page_url = NULL;
    size_t size = 0;
    static char manual_prefix[] = "/cgi-bin/"CGI_NAME"?manual=";
    static char page_prefix[] = "&page=";

    if ((manual_url != NULL) && (*manual_url != '\0'))
    {
	e_manual_url = escape_url(manual_url);
	if(e_manual_url==NULL)
	{
	    printf("<!-- ran out of memory while producing sh href (e_manual_url) -->\n");
	    return 1;
	}
	size += strlen(manual_prefix) + strlen(e_manual_url);
    }
    else
    {
	printf("<!-- bad manual url while producing sh href -->\n");
	return 1;
    }

    if ((page_url != NULL) && (*page_url != '\0'))
    {
	e_page_url = escape_url(page_url);
	if(e_page_url==NULL)
	{
	    free(e_manual_url);

	    printf("<!-- ran out of memory while producing sh href (e_page_url) -->\n");
	    return 1;
	}
	size += strlen(page_prefix) + strlen(e_page_url);
    }

    href_url = malloc(size + 1);
    if (href_url == NULL)
    {
	free(e_manual_url);

	if(e_page_url!=NULL)
	    free(e_page_url);

	printf("<!-- ran out of memory while producing sh href (href_url) -->\n");
	return 1;
    }

    p = &href_url[sprintf(href_url,"%s%s", manual_prefix, e_manual_url)];
    if (e_page_url != NULL)
    {
	sprintf(p,"%s%s", page_prefix, e_page_url);
	free(e_page_url);
    }
    free(e_manual_url);

    put_href(href_url, link_text);

    free(href_url);

    return 0;
}

long html_fontprint(char *text,shhtml_status *page_status)
{
    char *tmp_text;
    char *font_text;
    char * font_tag=NULL;

    /* if its an empty tag we are switching the global font off */
    if (strlen(text)==0 && page_status->ftag!=NULL)
    {
	printf("</%s>",page_status->ftag);
	free(page_status->ftag);
	page_status->ftag=NULL;
	return 0;
    }

    /* see if there is text to be outputing */
    tmp_text=index(text,':');
    if (tmp_text!=NULL)
    {
	tmp_text[0]=0;
	tmp_text++;
    }

    /* check for font name/number definition */
    if(index(text,'=')!=NULL)
    {
	/* not currently important enuf to implement */
	return 0;
    }

    /* font names or types here */
    if (strncasecmp(text,"code",4)==0)
    {
	font_tag=malloc(strlen(FONT_CODE_TAG));
	strcpy(font_tag,FONT_CODE_TAG);
    }
    else if (strncasecmp(text,"cite",4)==0)
    {
	font_tag=malloc(strlen(FONT_CITE_TAG));
	strcpy(font_tag,FONT_CITE_TAG);
    }
    else if (strncasecmp(text,"strong",6)==0)
    {
	font_tag=malloc(strlen(STRONG_TAG));
	strcpy(font_tag,STRONG_TAG);
    }
    else if (strncasecmp(text,"underline",9)==0)
    {
	font_tag=malloc(strlen(UNDERLINE_TAG));
	strcpy(font_tag,UNDERLINE_TAG);
    }
    else if (strncasecmp(text,"emphasis",8)==0)
    {
	font_tag=malloc(strlen(EMPHASIS_TAG));
	strcpy(font_tag,EMPHASIS_TAG);
    }
    else if (tolower(text[0])=='h')
    {
	/* h tags are compatablie with stronghelp! */
	font_tag=malloc(12);
	strcpy(font_tag,text);
    }

    /* if we have text to output - do so */
    if (tmp_text!=NULL)
    {
	if(font_tag!=NULL)
	{
	    if (tolower(font_tag[0])=='h')
		printf("<center>");

	    printf("<%s>",font_tag);
	}


	font_text=html_deescape_text(tmp_text);
	printf("%s",font_text);
	free(font_text);

	if(font_tag!=NULL)
	{
	    printf("</%s>",font_tag);

	    if (tolower(font_tag[0])=='h')
		printf("</center>");

	    free(font_tag);
	    font_tag=NULL;
	}
    }

    /* if a new global font was selected turn the old one off and the new
     * one on
     */
    if (font_tag!=NULL)
    {
	if(page_status->ftag!=NULL)
	{
	    printf("</%s>",page_status->ftag);

	    if (tolower(page_status->ftag[0])=='h')
		printf("</center>");

	    free(page_status->ftag);
	}
	page_status->ftag=font_tag;
	printf("<%s>",font_tag);

	    if (tolower(font_tag[0])=='h')
		printf("<center>");
    }

    return 0;
}

long html_include(char * text,shhtml_status * page_status)
{
    /* vars for getting include file */
    char * page_text = NULL;
    long page_len;
    long ret_val;

    /* vars for processing include text */
    long cur_offset = 0;
    char * line;

    /* strong help from image file get page */
    ret_val=shif_getpage(page_status->imagef,text,&page_text,&page_len);

    /* if for whatever reason we cannot acess the include ignore it */
    if (ret_val<0)
    {
	printf("<!-- dodgy include %s -->",text);
	return 1;
    }

    while (cur_offset<page_len)
    {
	line = html_get_line(page_text,&cur_offset,page_len);
	if (line==NULL) break;

	html_process_line(line,page_status);

    }

    /* stronghelp from image file free page */
    shif_freepage(page_text);
    return 0;

}


long html_process_control_line(char *text,shhtml_status * page_status)
{
    /* # directives either include stuff or change the alignment etc */
    /* any command we dont understand are ignored! */

    char* tmp_text;

    /*    printf("<!-- %s -->\n",text);*/

    switch(tolower(text[0]))
    {
    case ' ':
	/* comment */
	text++;
	tmp_text=html_deescape_text(text);
	printf("<!-- %s -->\n",tmp_text);
	free(tmp_text);
	return 0;
    case 'a':
	/* align */
	if (strncasecmp(text,"align",5)==0)
	{
	    /* turn off current alignment */
	    if(page_status->align)
		printf("</div>");

	    /* left, centre,right*/
	    switch(tolower(text[6]))
	    {
	    case 'l':
		if (strncasecmp(&text[6],"left",4)==0)
		{
		    page_status->align=1;
		    printf("<div align=left>");
		}
		break;
	    case 'c':
		if (strncasecmp(&text[6],"centre",6)==0)
		{
		    printf("<div align=center>");
		    page_status->align=2;
		}
		break;
	    case 'r':
		if (strncasecmp(&text[6],"right",5)==0)
		{
		    printf("<div align=right>");
		    page_status->align=3;
		}
		break;
	    default :
		page_status->align=0;
		break;
	    }
	}
	break;
    case 'b':
	/*
	 * background
	 * below
	 * bottom
	 */
	switch(tolower(text[1]))
	{
	case 'a':
	    if (strncasecmp(text,"background",10)==0)
	    {
	    }
	    break;
	case 'e':
	    if (strncasecmp(text,"below",5)==0)
	    {
	    }
	    break;
	case 'o':
	    if (strncasecmp(text,"bottom",6)==0)
	    {
	    }
	    break;
	}
	break;
    case 'd':
	/* draw */
	if (strncasecmp(text,"draw",4)==0)
	{
	}
	break;
    case 'e':
	/* endtable */
	if (strncasecmp(text,"endtable",8)==0)
	{
	    page_status->table=0;
	    printf("</table>\n");
	}
	break;
    case 'f':
	/* f */
	text++;
	/*	printf("<!-- %s -->\n",text);*/
	html_fontprint(text,page_status);
	break;
    case 'i':
	/*
	 * indent
	 * include
	 */
	switch(tolower(text[2]))
	{
	case 'd':
	    if (strncasecmp(text,"indent",6)==0)
	    {
	    }
	    break;
	case 'c':
	    if (strncasecmp(text,"include",7)==0)
		html_include(&text[8],page_status);
	    break;
	}
	break;
    case 'l':
	/* line */
	if (strncasecmp(text,"line",4)==0)
	    printf("<hr>\n");
	break;
    case 'm':
	/* manuals */
	if (strncasecmp(text,"manuals",7)==0)
	{
	}
	break;
    case 'p':
	/*
	 * parent
	 * prefix
	 * postfix
	 */
	switch(tolower(text[1]))
	{
	case 'a':
	    if (strncasecmp(text,"parent",6)==0)
	    {
	    }
	    break;
	case 'r':
	    if (strncasecmp(text,"prefix",6)==0)
	    {
		strcpy(page_status->prefix,text+7);
	    }
	    break;
	case 'o':
	    if (strncasecmp(text,"postfix",7)==0)
	    {
		strcpy(page_status->postfix,text+8);
	    }
	    break;
	}
	break;
    case 't':
	/*
	 * tab
	 * table
	 * tag
	 */
	if (strncasecmp(text,"table",5)==0)
	{
	    page_status->table=1;
	    printf("<table>\n");
	    break;
	}

	if (strncasecmp(text,"tab",3)==0)
	{
	    break;
	}

	if (strncasecmp(text,"tag",3)==0)
	{
	    break;
	}

	break;
    case 's':
	/*
	 * sprite
	 * spritefile
	 * subpage
	 */
	if (strncasecmp(text,"spritefile",10)==0)
	{
	}
	if (strncasecmp(text,"sprite",6)==0)
	{
	}
	if (strncasecmp(text,"subpage",7)==0)
	{
	}
	break;
    case 'r':
	/* rgb */
	if (strncasecmp(text,"rgb",3)==0)
	{
	}
	break;
    case 'w':
	/* wrap */
	if (strncasecmp(text,"wrap",4)==0)
	{
	}
	break;
    case '/':
	/* emphasis */
	if (page_status->emph)
	{
	    printf("</"EMPHASIS_TAG">");
	    page_status->emph=0;
	}
	else
	{
	    printf("<"EMPHASIS_TAG">");
	    page_status->emph=1;
	}
	break;
    case '*':
	/* strong */
	if (page_status->bold)
	{
	    printf("</"STRONG_TAG">");
	    page_status->bold=0;
	}
	else
	{
	    printf("<"STRONG_TAG">");
	    page_status->bold=1;
	}
	break;
    case '_':
	/* underscore */

	if (page_status->underline)
	{
	    printf("</"UNDERLINE_TAG">");
	    page_status->underline=0;
	}
	else
	{
	    printf("<"UNDERLINE_TAG">");
	    page_status->underline=1;
	}
	break;
    }
    return 0;
}

char * skip_whitespace(char * in_str)
{
    while ((in_str[0]!=0) && (isspace(in_str[0])))
	in_str++;

    return in_str;
}

long html_process_link(char* link,shhtml_status* page_status)
{
    char * manual_name=page_status->manual_name;
    char * link_page;
    char * text;
    char * page_link;

    /* find the => if present */
    text=index(link,'>');
    while ((text!=NULL) && (text[-1]=='\\'))
	text=index(++text,'>');

    if(text==NULL)
    {
	/* link is page and name */
	link_page=malloc(strlen(page_status->prefix)+strlen(link)+strlen(page_status->postfix)+4);
	strcpy(link_page,page_status->prefix);
	strcat(link_page,link);
	strcat(link_page,page_status->postfix);
    }
    else
    {
	/* link has separate name and page */
	text[-1]=0;
	text++;
	link_page=strdup(text);
    }

    if (!strncasecmp(link_page,"#url",4))
    {
	/* real url*/
	if (strlen(link_page)==4)
	    put_href(link,link);/* the link name is the url! */
	else
	    put_href(&link_page[5],link);
    }
    else
    {
	/* parsing for stronghelp link */

	/* look for colon - manual:page type link */
	page_link=index(link_page,':');
	if (page_link!=NULL)
	{
	    page_link[0]=0;
	    page_link++;
	    manual_name=link_page;
	}
	else
	{
	    page_link=link_page;
	}

	/* strip preceading spaces from the page_link */
	page_link = skip_whitespace(page_link);

	put_sh_href(manual_name,page_link,link);

    }

    free(link_page);

    return 0;
}

long html_check_special(int cur_stat,char * text,shhtml_status * page_status)
{
    char buff[2];
    char * tmp_text;

    buff[0]=text[0];
    buff[1]=0;

    if (cur_stat)
    {
	if (isalpha((int)text[-1]) && (!isalpha((int)text[1])))
	    html_process_control_line(buff,page_status);
	else
	    html_put_entity(text[0]);
    }
    else
    {
	/* this lot checks the item conforms to
	 * preceded by newline or space
	 * alphanumeric follows
	 * there is another item on the line (items cannot have \n in)
	 * that the second item conforms to the close criteria
	 */
printf("<!-- %d -->",text[-1]);
	if ((text[-1]==0 || text[-1]==' ')
	    && isalpha((int)text[1])
	    && ((tmp_text=index(&text[1],text[0]))!=NULL)
	    && isalpha((int)tmp_text[-1])
	    && (!isalpha((int)tmp_text[1])))
	{
	    html_process_control_line(buff,page_status);
	}
	else
	    html_put_entity(text[0]);
    }
    return 0;
}


long html_process_line(char * line,shhtml_status * page_status)
{
    unsigned char * text=line;
    char * tmp_text=text;

    /* skip preceading spaces */
    for(;(text[0]!=0)&&(text[0]==32);text++);
    /* if its a # directive do summat with it */
    if (text[0]=='#')
    {
	/* there can be many commands separated by a ; on a command line
	 * this routine breaks the line up into separate commands to feed
	 * to the command process routine
	 */
	text++;
	tmp_text=text;
	while(tmp_text!=NULL)
	{
	    tmp_text=index(text,';');

	    if (tmp_text!=NULL)
	    {

		tmp_text[0]=0;
		html_process_control_line(text,page_status);
		text = ++tmp_text;/* skip null */
		while (!isalpha((int)text[0]))
		    text++;
	    }
	    else
		html_process_control_line(text,page_status);
	}
	return 0;
    }

    /* go through line byte by byte translating as we go */

    /* start a new paragraph per line? we will see */
    if(page_status->table)
	printf("<tr>\n");

    for(;(text[0]!=0);text++)
    {

	switch(text[0])
	{

	case '\\':
	    /* escaping a char */
	    text++;
	    html_put_entity(text[0]);
	    break;

	case '/': /* emphasis */
	    html_check_special(page_status->emph,text,page_status);
	    break;

	case '*': /* strong */
	    html_check_special(page_status->bold,text,page_status);
	    break;

	case '_': /* underscore */
	    html_check_special(page_status->underline,text,page_status);
	    break;

	case '{':
	    /* command in brackets */
	    text++;
	    tmp_text=index(text,'}');
	    if (tmp_text!=NULL)
	    {
		tmp_text[0]=0;
		html_process_control_line(text,page_status);
		text = tmp_text;/* skip null */
	    }
	    else
	    {
		/* fishy - no end of command brace?
		 * try using the whole rest of line anyway */
		html_process_control_line(text,page_status);
		return 0;
	    }
	    break;

	case '<':
	    /* possible url */
	    if (text[1]=='=' || text[1]=='-' || text[1]=='<')
	    {
		html_put_entity(text[0]);
		text++;
		html_put_entity(text[0]);
		break;
	    }
	    text++;
	    tmp_text=index(text,'>');

	    /* ensure we are not fooled by the => or escaped \> in the link */
	    while ((tmp_text!=NULL) && ((tmp_text[-1]=='=') | (tmp_text[-1]=='\\')))
		tmp_text=index(++tmp_text,'>');

	    if (tmp_text!=NULL)
	    {
		tmp_text[0]=0;
		html_process_link(text,page_status);
		text = tmp_text;/* skip null */
	    }
	    else
	    {
		/* fishy - no end of link use the whole rest of line anyway */
		html_process_link(text,page_status);
		return 0;
	    }
	    break;

	case 0x8f :
	case '&':
	case '>':
	case 0xdc : /* dunno yet! */
	case 0xde : /* dunno yet! */
	    /*	case 9:*/
	    /* need escaping */
	    html_put_entity(text[0]);
	    break;

	default:
	    putchar(text[0]);
	    break;
	}

    }

    printf("<br>\n");

    return 0;
}


int html_output(FILE * imagef,char * manual_name,char * sh_page,long sh_page_len)
{
    long cur_offset = 0;
    char * line;
    shhtml_status page_status;

    strncpy(page_status.manual_name,manual_name,1024);
    page_status.parent[0]=0;
    page_status.prefix[0]=0;
    page_status.postfix[0]=0;
    page_status.bold=0;
    page_status.emph=0;
    page_status.underline=0;
    page_status.align=0;
    page_status.table=0;
    page_status.imagef=imagef;
    page_status.ftag=NULL;

    printf("<head><title>%s</title></head>\n",html_get_line(sh_page,&cur_offset,sh_page_len));
    printf("<BODY BGColor=\"#DDDDDD\" TEXT=\"#000000\" BackGround=\"/"SH_CGI_ROOT"/."BACKGROUND_IMAGE"\">\n");

    while (cur_offset<sh_page_len)
    {
	line = html_get_line(sh_page,&cur_offset,sh_page_len);
	if (line==NULL) break;

	html_process_line(line,&page_status);

    }
    /* put our footer in as a comment */

    put_footer(1);

    printf("</body>\n");
    return 0;

}
