#ifndef _STRINGS_H_
#define _STRINGS_H_

/* Standard #defines */

#ifdef __cplusplus
extern "C" {
#endif


#ifndef CNULL
#define CNULL '\0'
#endif

#ifndef MAX_VARGS
#define MAX_VARGS 100                                             
#endif 

#ifndef n_free
#define n_free(x) if((x) != NULL){free((void *) x);x=NULL;}
#endif

char * xmlEncode(char * src);

/*******************************************************************************
* vstrdupcat...
*
* Variatic string concatination function.
*******************************************************************************/
void vstrdupcat(char **str, ...);

#ifdef __cplusplus
}
#endif

#endif // _STRINGS_H_
