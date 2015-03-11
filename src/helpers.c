#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "helpers.h"

int BIGENDIAN = -1;

/*
  Exits the program in case of error, displaying hte given message and errno
*/
void errsys(const char * msg ){
  perror(msg);
  exit(EXIT_FAILURE);
}



/*
  The minimum, simply for positive integers
*/
unsigned int min(unsigned int x, unsigned int y){
  return (x>y)?y:x;
}

/*
  The maximum, simply for positive integers
*/
unsigned int max(unsigned int x, unsigned int y){
  return (x>y)?x:y;
}

/*
  checks if the system is in big or little endian.
  returns 1 for big endian, 0 for little endian
*/
int checkbigendian(){
  char * c;
  short s = 1;
  c = (char*)(&s);
  BIGENDIAN = ((*c) == 0)?1:0;
  return BIGENDIAN;
}

/*
  changes the endiannes of an unsigned int
*/
unsigned int otherendian(unsigned int nb){
  unsigned int res;
  res = 0;
  res += (nb & 0x0000ff00)<<8;
  res += (nb & 0x00ff0000)>>8;
  res += (nb & 0x000000ff)<<24;
  res += (nb & 0xff000000)>>24;
  return res;
}

/*
  converts a big-endian unsigned int to the local endianess
*/
unsigned int localendian(unsigned int nb){
  if(BIGENDIAN == -1)
    checkbigendian();
  if(BIGENDIAN == 1)
    return nb;
  else
    return otherendian(nb);
}

/*
  converts a local unsigned int to a big-endian
*/
unsigned int bigendian(unsigned int nb){
  if(BIGENDIAN == -1)
    checkbigendian();
  if(BIGENDIAN == 1)
    return nb;
  else
    return otherendian(nb);
}

/*
  converts a local unsigned int to a little-endian
*/
unsigned int littleendian(unsigned int nb){
  if(BIGENDIAN == -1)
    checkbigendian();
  if(BIGENDIAN == 0)
    return nb;
  else
    return otherendian(nb);
}


/*
  converts a time in UTC seconds to a string "yyyy/mm/dd day" null-terminated.
  String is allocated and must be freed.
*/
char * UTCtoString(int t){
  char * tmp = (char *)malloc(sizeof(char) * 100);
  char * res;
  time_t newt = (time_t)t;
  struct tm * time = localtime(&newt);
  memset(tmp, 0, 100);
  strftime(tmp, 100, "%F %A  %H:%M", time);
  res = malloc(sizeof(char) * (strlen(tmp)+1) );
  memcpy(res, tmp, strlen(tmp)+1);
  free(tmp);
  return res;
}


/*
  prints n times the given string, on the standard output
 */
void printsn(char * s, size_t n){
  unsigned int i;
  for(i = 0; i < n; i++){
    printf("%s", s);
  }
}


/*
  Separates te string with the token and returns the array of strings.
  Array and its content are newly allocated and has to be freed : that's why
  the last index contains NULL.
 */
char ** split(char * s, char token){
  char * copy; /* in case we don't want to modify the passed string */
  char * sub; /* will contain elements one by one */
  char ** res; /* will contain all sub elements */
  int size = 1; /* number of tokens found */
  for(copy = s; copy != NULL && *copy != '\0'; copy++){
    if( *copy == token)
      size++;
  }
  res = (char **)malloc(sizeof(char*)*(size+1) );
  if(res == NULL){
    perror("Malloc failed");
    return NULL;
  }
  copy = strdup(s);
  if(copy == NULL){
    perror("strdup");
    free(res);
    return NULL;
  }
  res[size] = NULL;
  size = 0; /* now size acts as an iterator */
  while( (sub = strsep(&copy, &token) ) != NULL){
    res[size] = strdup(sub);
    if(res[size] == NULL){
      int i;
      perror("strdup");
      for(i = 0; i < size; i++)
	free(res[i]);
      free(res);
      return NULL;
    }
    size++;
  }
  free(copy);
  free(sub);
  return res;
}


/*
  Delete all occurences of the token in the string (modify it)
 */
void deletechar(char * const s, char token){
  if( s && *s){
    char const * pr = s;
    char * pw = s;
    while( *pr){
      if(*pr != token){
	*pw = *pr;
	pw++;
      }
      pr++;
    }
    *pw = 0;
  }


}
