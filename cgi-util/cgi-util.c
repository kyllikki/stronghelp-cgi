/*
  cgi-util.c
  
  version 2.0.4
  
  by Bill Kendrick <bill@newbreedsoftware.com>
  and Mike Simons <msimons@fsimons01.erols.com>
  
  New Breed Software
  http://www.newbreedsoftware.com/cgi-util/
  
  April 6, 1996 - August 24, 1999
*/


#include <stdio.h>
#include <ctype.h>
#include <errno.h>

#ifndef NO_STDLIB_H
#include <stdlib.h>
#else
char *getenv();
#endif

#include <string.h>
#include "cgi-util.h"


/* Globals: */

cgi_entry_type * cgi_entries = NULL;
int cgi_num_entries = 0;
int cgi_errno = CGIERR_NONE;
int cgi_request_method = CGIREQ_NONE;
char * query = NULL;


/* English error strings: */

char * cgi_error_strings[CGIERR_NUM_ERRS] = {
  "", "Not an integer", "Not a double", "Not a boolean",
  "Unknown method", "Incorrect Content Type",
  "NULL Query String", "Bad Content Length",
  "Content Length Discrepancy"
};


/* Converts hexadecimal to decimal (character): */

char x2c(char *what)
{
  register char digit;
  
  digit = (what[0] >= 'A' ? ((what[0] & 0xdf) - 'A')+10 : (what[0] - '0'));
  digit *= 16;
  digit += (what[1] >= 'A' ? ((what[1] & 0xdf) - 'A')+10 : (what[1] - '0'));
  return (digit);
}


/* Unescapes "%"-escaped characters in a query: */

void unescape_url(char *url)
{
  register int x,y,len;
  
  len = strlen(url);
  
  for (x=0, y=0; url[y]; ++x, ++y)
    {
      if ((url[x] = url[y]) == '%'
	  && y < len - 2)   /* 2.0.4 - MJ Pomraning (pilcrow@mailbag.com) */
	{
	  url[x] = x2c(&url[y+1]);
	  y+=2;
        }
    }
  url[x] = '\0';
}


/* Converts pluses back to spaces in a query: */

void plustospace(char *str)
{
  register int x;
  
  for (x=0; str[x]; x++)
    if (str[x] == '+')
      str[x] = ' ';
}


/* Initialize the CGI.  Grab data from the browser and prepare it for us. */

int cgi_init(void)
{
  int cl, i;
  
  
  /* Default, no errors, no name/value pairs ("entries"): */
  
  cgi_errno = CGIERR_NONE;
  cgi_num_entries = 0;
  
  
  /* Check for REQUEST_METHOD (set by HTTP server): */
  
  if (getenv("REQUEST_METHOD") == NULL)
    {
      /* None set?  Assume the user is invoking the CGI from a shell prompt
	 (for debugging): */
      
      cgi_request_method = CGIREQ_NONE;
    }
  else
    {
      /* Determine the exact request method, and grab the data (if any)
	 in the appropriate manner: */
      
      if (strcmp(getenv("REQUEST_METHOD"),"POST") == 0)
	{
	  /* Post method (data is sent to us via "stdin"): */
	  
	  cgi_request_method = CGIREQ_POST;
	  
	  
	  if (getenv("CONTENT_TYPE") == NULL ||
	      strcmp(getenv("CONTENT_TYPE"),
		     "application/x-www-form-urlencoded") != 0)
	    {
	      /* Is the content type incorrect (or not set at all?) */
	      
	      cgi_errno = CGIERR_INCORRECT_TYPE;
	      return(cgi_errno);
	    }
	  else
	    {
	      /* How much data do we expect? */
	      
	      if (getenv("CONTENT_LENGTH") == NULL ||
		  sscanf(getenv("CONTENT_LENGTH"), "%d", &cl) != 1)
		{
		  cgi_errno = CGIERR_BAD_CONTENT_LENGTH;
		  return(cgi_errno);
		}
	      
	      
	      /* Create space for it: */
	      
	      query = malloc(cl + 1);
	      /* 2.0.1 - Tadek Orlowski (orlowski@epnet.com) ... "+1" */
	      
	      if (query == NULL)
		{
		  cgi_errno = CGIERR_OUT_OF_MEMORY;
		  return(cgi_errno);
		}
	      
	      
	      /* Read it in: */
	      
	      fgets(query, cl + 1, stdin);
	      
	      
	      /* Verify that we got as much data as we expected: */
	      
	      if (strlen(query) != cl)
		cgi_errno = CGIERR_CONTENT_LENGTH_DISCREPANCY;
	    }
	}
      else if (strcmp(getenv("REQUEST_METHOD"),"GET") == 0)
	{
	  /* GET method (data sent via "QUERY_STRING" env. variable): */
	  
	  cgi_request_method = CGIREQ_GET;
	  
	  
	  /* Get a pointer to the data: */
	  
	  query = getenv("QUERY_STRING");
	  
	  if (query == NULL)
	    {
	      /* Does the "QUERY_STRING" env. variable not exist!? */
	      
	      cgi_errno = CGIERR_NULL_QUERY_STRING;
	      
	      return(cgi_errno);
	    }
	  else
	    {
	      /* Determine the content length by seeing how big the
		 string is: */
	      
	      cl = strlen(query);
	    }
	}
      else
	{
	  /* Something else? We can't handle it! */
	  
	  cgi_request_method = CGIREQ_UNKNOWN;
	  cgi_errno = CGIERR_UNKNOWN_METHOD;
	  cgi_num_entries = 0;
	  
	  return(cgi_errno);
	}      
      
      
      /* How many entries (name/value pairs) do we need to
	 allocate space for? (They should be separated by "&"'s) */
      
      cgi_num_entries = 0;
      
      for (i = 0; i <= cl; i++)
	if (query[i] == '&' || query[i] == '\0')
	  cgi_num_entries++;
      
      
      /* Allocate the space for that many structures: */
      
      cgi_entries = malloc(sizeof(cgi_entry_type) * cgi_num_entries);

      if (cgi_entries == NULL)
	{
	  cgi_errno = CGIERR_OUT_OF_MEMORY;
	  return(cgi_errno);
	}
      
      
      /* Grab each name/value pair: */
      
      cgi_num_entries = 0;
      
      
      /* (Begin with the first half of the first pair): */
      
      if (query[0] != '\0' && query[0] != '&')
	cgi_entries[0].name = query;
      
      
      /* Go through the entire string of characters: */
      
      for (i = 0; i <= cl; i++)
	{
	  if (query[i] == '&')
	    {
	      /* "&" represents the end of a name/value pair: */
	      
	      cgi_entries[cgi_num_entries].name = query + i + 1;
	      query[i] = '\0';
	    }
	  else if (query[i] == '=')
	    {
	      /* "=" is the end of the name half of a name/value pair: */
	      
	      cgi_entries[cgi_num_entries].val = query + i + 1;
	      
	      /*  plustospace(cgi_entries[cgi_num_entries].val);
		  unescape_url(cgi_entries[cgi_num_entries].val); */
	      
	      cgi_num_entries++;
	      
	      query[i] = '\0';
	    }
	}

      for (i = 0; i < cgi_num_entries; i++)
	{
	  plustospace(cgi_entries[i].val);
	  unescape_url(cgi_entries[i].val);
	}

    }
  
  
  /* Fix any NULL strings to be empty strings */
  /* 2.0.4 - MJ Pomraning (pilcrow@mailbag.com) */
  
  for (i = 0; i < cgi_num_entries; i++)
    {
      if (cgi_entries[i].name == NULL)
	cgi_entries[i].name = "";
      if (cgi_entries[i].val == NULL)
	cgi_entries[i].val = "";
    }
  
  return(CGIERR_NONE);
}


/* Free up memory that was allocated when we called "cgi_init()": */

void cgi_quit(void)
{
  if (cgi_request_method == CGIREQ_NONE ||
      cgi_request_method == CGIREQ_UNKNOWN)
    {
      /* Nothing to do! */
    }
  else
    {
      if (cgi_request_method == CGIREQ_POST)
	{
	  /* Was it POST method?  Free the data we had read from "stdin" */
	  
	  free(query);
	}


      /* Free the entry structures themselves: */
      
      free(cgi_entries);
    }
  
  cgi_entries = NULL;
  cgi_num_entries = 0;
  cgi_errno = CGIERR_NONE;
  cgi_request_method = CGIREQ_NONE;
  query = NULL;
}


/* Grab a value and return it as a string: */

const char * cgi_getentrystr(const char *field_name)
{
  int x;
  
  if (cgi_request_method != CGIREQ_NONE)
    {
      /* Look for the name: */
      
      for (x = 0; x < cgi_num_entries; x++)
	{
	  if (strcmp(cgi_entries[x].name, field_name) == 0)
	    {
	      return (cgi_entries[x].val);
	    }
	}
      
      return(NULL);
    }
  else
    {
      /* printf("CGI-UTIL: \"%s\" ? ", field_name);
      fgets(buf, 512, stdin);
      buf[strlen(buf) - 1] = '\0'; */
      
      return("x");
    }
}


/* Grab a value and return it as an integer: */

int cgi_getentryint(const char *field_name)
{
  int v;
  
  v = 0;
  
  if (cgi_getentrystr(field_name) != NULL)
    {
      if (sscanf(cgi_getentrystr(field_name), "%d", &v) != 1)
	cgi_errno = CGIERR_NOT_INTEGER;
    }
  else
    cgi_errno = CGIERR_NOT_INTEGER;
  
  return(v);
}


/* Grab a value and return it as a double: */

double cgi_getentrydouble(const char *field_name)
{
  double v;
  
  v = 0;
  
  if (cgi_getentrystr(field_name) != NULL)
    {
      if (sscanf(cgi_getentrystr(field_name), "%lf", &v) != 1)
	cgi_errno = CGIERR_NOT_DOUBLE;
    }
  else
    cgi_errno = CGIERR_NOT_DOUBLE;
  
  return(v);
}


/* Grab a value and return it as a boolean (depending on if the
   value was "yes" or "on", or "no" or "off"): */

int cgi_getentrybool(const char *field_name, int def)
{
  const char * temp;
  int v;
  
  
  /* Assume the default: */
  
  v = def;
  
  
  /* Get the value (if any): */
  
  temp = cgi_getentrystr(field_name);
  
  if (temp != NULL)
    {
      if (strcasecmp(temp, "yes") == 0 ||
	  strcasecmp(temp, "on") == 0)
	{
	  /* A "yes" or "on" is a 1: */
	  
	  v = 1;
	}
      else if (strcasecmp(temp, "no") == 0 ||
	       strcasecmp(temp, "off") == 0)
	{
	  /* A "no" or "off" is a 0: */
	  
	  v = 0;
	}
      else if (temp[0] != 0)
	{
	  /* We got something, but not "yes", "on", "no" or "off": */
	  
	  cgi_errno = CGIERR_NOT_BOOL;
	}
    }
  else
    cgi_errno = CGIERR_NOT_BOOL;
  
  
  return(v);
}


/* Open a file and send it to "stdout" (the browser): */
/* (Returns an error if we can't open the file) */

int cgi_dump_no_abort(const char * filename)
{
  FILE * fi;
  int c;
  
  
  cgi_errno = CGIERR_NONE;
  
  
  /* Open the file: */
  
  fi = fopen(filename, "r");
  if (fi == NULL)
    cgi_errno = CGIERR_CANT_OPEN;
  else
    {  
      /* Read data and push it to "stdout": */
      
      do
	{
	  c = fgetc(fi);
	  if (c != EOF)
	    fputc(c, stdout);
	}
      while (c != EOF);
      
      fclose(fi);
    }
  
  return(CGIERR_NONE);
}


/* Open a file and send it to "stdout" (the browser): */
/* (Displays an error message and quits the CGI if we can't open the file) */

void cgi_dump(const char * filename)
{
  if (cgi_dump_no_abort(filename) != CGIERR_NONE)
    {
      printf("Can't open %s - %s\n", filename, strerror(errno));
      exit(0);
    }
}


/* Display a simple error message and quit the CGI: */

void cgi_error(const char * reason)
{
  printf("<h1>Error</h1>\n");
  printf("%s\n", reason);
  
  exit(0);
}


/* Returns whether or not an e-mail address appears to be in the correct
   syntax ("username@host.domain"): */

int cgi_goodemailaddress(const char * addr)
{
  int i;
  
  
  /* No "@".. what? */
  
  if (strchr(addr, '@') == NULL)
    return 0;
  
  
  /* "@" or "." at the end or beginning? */
  
  if (addr[strlen(addr - 1)] == '@' ||
      addr[strlen(addr - 1)] == '.' ||
      addr[0] == '@' || addr[0] == '.')
    return 0;
  
  
  /* No "." after the "@"?  More than one "@"? */
  
  if (strchr(strchr(addr, '@'), '.') == NULL ||
      strchr(strchr(addr, '@') + 1, '@') != NULL)
    return 0;
  
  
  /* Any illegal characters within the string? */
  
  for (i = 0; i < strlen(addr); i++)
    {
      if (isalnum(addr[i]) == 0 &&
	  addr[i] != '.' && addr[i] != '@' && addr[i] != '_' &&
	  addr[i] != '-')
	return(0);
    }
  
  
  /* Must be ok... */
  
  return 1;
}


/* Returns the English string description for a particular cgi-util error
   value: */

const char * cgi_strerror(int err)
{
  if (err < 0 || err > CGIERR_NUM_ERRS)
    return("");
  else
    return(cgi_error_strings[err]);
}
