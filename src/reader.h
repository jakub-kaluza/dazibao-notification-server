#ifndef READER_H
#define READER_H

#include <sys/types.h>



void printHTML(char * file, int fd, off_t left, off_t right);


/* 
  Prints a tlv on the standard output, html style
*/
void tlvtoHTML(tlv_t * t, char * s);


/*
  Write the file with name ".file_num.type" where num is a static index
 */
char * writeFile(char * file, char * ext, unsigned int size, byte * content);



/*
  The main loop for reading a dazibao :
  parsing the file, analyses the bytes in order to extract a type and a length.
  if it could get a type and a length, then it analyses "length" bytes to product
  the content.
  Then, displays the tlv and loops for a following tlv
 */
void loop(int fd, off_t left, off_t right, int lvl, tlv_t * parent, bool htmlflag, char * file);


/* 
 The "real" program : opens a file, locking it, and parses it, then closes it.
 the flag htmlflag indicate the mode
*/
void readdazibao(char * f, bool htmlflag);

#endif
