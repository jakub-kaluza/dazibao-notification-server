#ifndef WRITER_H
#define WRITER_H

#include "dazibao.h"

/*
Shows how to use the writer
*/
void showhelp();

/*
  write the current version as a header
 */
int writeversion(int fd);

/*
  Attempts to write the TLV at the end of the given file.
  Returns -1 if an error occured.
 */
int writetlv(tlv_t * tlv, int fd);

/*
  Reads the nb first bytes of a file and returns them in an array.
  dest must be large enough
 */
byte * readFile(int fd, unsigned int nb, byte * dest);


/*
  Starts the process to write the tlv, 
  creating a backup of the file,
  adding the TLV inducted from the expression,
  and deleting the backup or the new dazibao in case of error.
  dateflag tells if the TLV must be wrapped to a dated TLV whose date is "right now"
 */
int addTLV(char * file, char * expr, bool dateflag);


/*
  Tries to create a TLV matching the given expression and returns it.
  Returns NULL in case of error or if the TLV is incorrect.
  If a TLV has been created, it must be freed.
 */
tlv_t * exprtoTLV(char * expr);

/* 
   extracts a DATED from a string and injects it in the given TLV
 */
tlv_t * exprtodated(char * substring, tlv_t * tlv);   

/* 
   extracts a COMPOUND from a string and injects it in the given TLV
 */
tlv_t * exprtocompound(char * substring, tlv_t * tlv);

/*
  extracts a JPEG/PNG, an audio file or a video  from the file from a string and
  injects it in the given TLV
*/
tlv_t * exprtofile(char * substring, tlv_t * tlv);

/*
  creates a backup of the file int he second file
 */
int cp(char * original, char * backup);


#endif
