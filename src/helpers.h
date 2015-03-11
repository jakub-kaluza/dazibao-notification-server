#ifndef HELPERS_H
#define HELPERS_H

#include <unistd.h>

extern int BIGENDIAN;

typedef enum{ FALSE = 0, TRUE = 1 } bool;

/* We define a byte as an unsigned char to keep in mind that we are not really using characters */
typedef unsigned char byte;

/*
  Exits the program in case of error, displaying hte given message and errno
 */
void errsys(const char * msg );

/*
  displays all the errno table
 */
void errnotable();

/*
  The minimum, simply for positive integers
 */
unsigned int min(unsigned int x, unsigned int y);

/*
  The maximum, simply for positive integers
*/
unsigned int max(unsigned int x, unsigned int y);

/*
  checks if the system is in big or little endian.
  returns 1 for big endian, 0 for little endian
*/
  int checkbigendian();

/*
  changes the endiannes of an unsigned int
*/
unsigned int otherendian(unsigned int nb);

/*
  converts a big-endian unsigned int to the local endianess
 */
unsigned int localendian(unsigned int nb);

/*
  converts a local unsigned int to a big-endian
 */
unsigned int bigendian(unsigned int nb);

/*
  converts a local unsigned int to a little-endian
*/
unsigned int littleendian(unsigned int nb);

/*
  converts a time in UTC seconds to a string "yyyy/mm/dd day".
  String is allocated and must be freed.
 */
char * UTCtoString(int time);

/*
  prints n times the given string, on the standard output
 */
void printsn(char * s, size_t n);

/*
  Separates te string with the token and returns the array of strings newly allocated
 */
char ** split(char * s, char token);

/*
  Delete all occurences of the token in the string (modify it)
 */
void deletechar(char * s, char token);
#endif
