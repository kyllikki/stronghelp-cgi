/*
  test.cgi
  
  As simple example of a CGI written in C, using cgi-util 2.0
  
  by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/cgi-util/
  
  June 12, 1999 - July 12, 1999
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cgi-util.h"


/* --- MAIN --- */

int main(int argc, char * argv[])
{
  int res;
  
  
  /* Initialize the CGI and send out an HTTP header: */
  
  res = cgi_init();
  
  printf("Content-type: text/html\n\n");
  
  
  /* Was there an error initializing the CGI??? */
  
  if (res != CGIERR_NONE)
    {
      printf("Error # %d: %s<p>\n", res, cgi_strerror(res));
      exit(0);
    }
  
  
  /* Display some text: */
  
  printf("Hello.<p>\n");
  
  
  /* Grab some fields from an HTML form and display them: */
  
  if (cgi_getentrystr("name") != NULL)
    printf("name=%s<p>\n", cgi_getentrystr("name"));
  
  printf("age=%d<p>\n", cgi_getentryint("age"));
  
  if (cgi_getentrystr("sex") != NULL)
    printf("sex=%s<p>\n", cgi_getentrystr("sex"));
  
  
  /* Close up the CGI: */
  
  printf("Goodbye!<p>\n");
  
  cgi_quit();
  
  return(0);
}
