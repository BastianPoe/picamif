#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#ifndef MAX_VARGS
#define MAX_VARGS 256
#endif

char * xmlEncode(char *src) {
    char * result = (char *) malloc(sizeof(char) * strlen(src) * 6 + 1);
    int i = 0, j = 0;

    result[0] = '\0';
    for (i = 0; i < strlen(src); i++) {
        if (src[i] == '&') {
            result[j++] = '&';
            result[j++] = 'a';
            result[j++] = 'm';
            result[j++] = 'p';
            result[j++] = ';';
        } else  if (src[i] == '<') {
            result[j++] = '&';
            result[j++] = 'l';
            result[j++] = 't';
            result[j++] = ';';
        } else  if (src[i] == '>') {
            result[j++] = '&';
            result[j++] = 'g';
            result[j++] = 't';
            result[j++] = ';';
        } else  if (src[i] == '"') {
            result[j++] = '&';
            result[j++] = 'q';
            result[j++] = 'u';
            result[j++] = 'o';
            result[j++] = 't';
            result[j++] = ';';
        } else  if (src[i] == '\'') {
            result[j++] = '&';
            result[j++] = 'a';
            result[j++] = 'p';
            result[j++] = 'o';
            result[j++] = 's';
            result[j++] = ';';
        } else {
            result[j++] = src[i];
        }
    }
    result[j++] = '\0';



    return result;
}

/*******************************************************************************
* vstrdupcat...
*
* Variatic string concatination function.
*******************************************************************************/
void vstrdupcat(char **str, ...) {
  va_list args;
  char   *argv[MAX_VARGS], *ptr = *str;
  long long int i = 0, len = 0,
      extralen = 0,
      increment = 8192,
      nalloc = 0;  
  int lastarg = 0;
                                                                  
  va_start(args, str);
  while (!lastarg) { 
    argv[i] = va_arg(args, char *);                               
    if ((argv[i]) && (i < (MAX_VARGS -1))) {                      
      extralen += strlen(argv[i]);
    } else {
      lastarg = 1;
    }
    i++;
  }
  
  if (ptr) {
    len = strlen(ptr);
    if ((len / increment) < ((len + extralen) / increment)) {
      // we need to realloc
      nalloc = (((len + extralen) / increment) + 1)*increment;
      ptr = (char *) realloc(ptr, sizeof(char) * (nalloc));
      if (ptr == NULL)
        return;
    }
  } else {
    nalloc = ((extralen / increment) + 1)*increment;
    ptr = (char *) malloc(sizeof(char) * (nalloc));
    if (ptr == NULL)
      return;
    *ptr = '\0';
  }

  i = 0;
  while (argv[i] != NULL) {
    strcat(ptr, argv[i]);
    i++;
  }

  va_end(args);
  *str = ptr;
}

