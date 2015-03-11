#ifndef DAZIBAO_H
#define DAZIBAO_H

#include <sys/types.h>

#include "helpers.h"

#define BUFFER_SIZE 512

#define MAGICNUMBER 53
#define VERSION 0

#define EBADMAGIC 22 /* if the Magic number is not 53 */
#define EBADVERSION 22 /* if the version is not 0 */
#define EINVALID 5 /* for invalid input/output */

#define TLVNOTYPE 22 /* if we cannot find a type for the TLV */
#define TLVNOLENGTH 42 /* if we cannot find a length for the TLV */
#define TLVBADSIZE 12 /* if the content's size is not the defined size (too short) */
#define TLVTOOBIG 27 /* when the TLV is larger than its defined size */



/* List of all the possible types of TLV */
typedef enum {
  PAD1 = 0,
  PADN,
  TEXT,
  PNG,
  JPEG,
  COMPOUND,
  DATED,
  /* official extensions */
  /* extensions */
  MP3 = 128,
  OGG,
  VIDEO, /* video format is theora (.ogv) */
  WEBM, /* video format is webm = vp8 + vorbis */
  TYPE_COUNT
} tlv_type; 

/* Object representing a TLV */
typedef struct tlv{
  tlv_type type; 
  unsigned int length; /* used to be a byte[3], but more convenient like this */ 
  byte * content; 
} tlv_t;

/*
  Checks if the version of the file matches the good version (magic = 53, version = 0)
  The argument is a file descriptor to the open dazibao.
  Returns 0 if the version is good, -1 if it not, errno is set. 
*/
int checkversion(int fd);

/*
  Opens a file with the given options and puts a flock on it with the given operation
*/
int lockopen(const char * fic, int flags, mode_t mode, int operation);

/*
  computes a length of a TLV from a 3-bytes array
*/
unsigned int computelength(byte * t);


/*
  puts in a 3-bytes arrays the given unsigned int, must be allocated
*/
byte * arrayedlength(unsigned int nb, byte * dest);

/*
  frees a TLV and its content
*/
void freeTLV(tlv_t * tlv);



void constrTLV(int length, char * content, tlv_t * t);



/*
  Adds to the child tlv to the parent's content.
  Doesn't change its size, the base content array has to be large enough.
  returns 0 if success, -1 if the container is full or if an error occured
*/
int appendTLV(tlv_t * parent, tlv_t * child);

/*
  returns an allocated copy of a TLV inside another, between left and right.
  content is also malloced
*/
tlv_t * extractTLV(tlv_t * parent, off_t left, size_t n);

/*
  Write the file with name ".name_num.type" where num is a static index.
  The size of the file and its content must be providen.
  Returns the name of the file.
*/
void printtlv(tlv_t * t, unsigned int lvl);


/* 
   reads a TLV from the file descriptor
*/
tlv_t * readTLV(int fd);

#endif 
