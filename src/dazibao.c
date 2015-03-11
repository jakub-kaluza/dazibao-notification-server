#include "dazibao.h"
#include "helpers.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


/*
  Checks if the version of the file matches the good version (magic = 53, version = 0)
  The argument is a file descriptor to the open dazibao.
  Returns MBZ if the magic/version is good, -1 if not <and errno is set>.
*/
int checkversion(int fd){
  char buffer[4];
  int nb;
  int mbz = 0;
  if( (nb = read(fd, buffer, 4) ) <= 0){
    errno = EINVALID;
    errsys("Read header error");
  }
  if(buffer[0] != MAGICNUMBER){
    errno = EBADMAGIC;
    perror("Bad magic number");
    return -1; /* has to quit */
  }
  if(buffer[1] != VERSION){
    errno = EBADVERSION;
    perror("Bad version");
    return -1; /* has to quit */
  }
  mbz += buffer[2];
  mbz <<= 8;
  mbz += buffer[3];
  return mbz;
}


/*
  Opens a file with the given options and puts a flock on it with the given operation
*/
int lockopen(const char * fic, int flags, mode_t mode, int operation){
  int fd = open(fic, flags, mode);
  if( fd == -1 ){
    errsys("Could not open the file");
  }
  if( flock(fd, operation) < 0 ){
    errsys("Could not flock the file");
  }
  return fd;
}

/*
  computes the size of a tlv
*/
unsigned int computelength(byte * t){
  unsigned int res = 0;
  byte t2[3];
  t2[0] = (byte)t[0];
  t2[1] = (byte)t[1];
  t2[2] = (byte)t[2];
  res += t2[0];
  res <<= 8;
  res += t2[1];
  res <<= 8;
  res += t2[2];
  return res;
}


/*
  puts in a 3-bytes arrays the given unsigned int, must be allocated
*/
byte * arrayedlength(unsigned int nb, byte * dest){
  byte * tmp = (byte * ) (&nb);
  tmp++;
  memset(dest, 0, 3);
  memcpy(dest, tmp, 3);
  return dest;
}


/*
  frees a TLV and its content
*/
void freeTLV(tlv_t * tlv){
  if(tlv == NULL)
    return;
  if(tlv->type != PAD1 && tlv->content != NULL){
    free(tlv->content);
    tlv->content = NULL;
  }
  free(tlv);
  tlv = NULL;
}


/*
  Adds to the child tlv to the parent's content.
  Doesn't change its size, the base content array has to be large enough.
  returns 0 if success, -1 if the container is full or if an error occured
*/
int appendTLV(tlv_t * parent, tlv_t * child){
  unsigned int index;
  byte * outlength;
  /* retrieving the byte where we should insert */
  for(index = 0; index < parent->length; index++){
    if( parent->content[index] == '\0') /* normally, it has been filled with \0 */
      break; 
  }
  if(index >= parent->length)
    return -1;
  memcpy(parent->content+index, &(child->type), 1);
  if( child->type != PAD1){
    outlength = (byte *)malloc(sizeof(byte)*3);
    outlength = arrayedlength(bigendian(child->length), outlength);
    memcpy(parent->content+index+1, outlength, 3);
    free(outlength);
    memcpy(parent->content+index+4, child->content, child->length);
  }
  return 0;
}


/*
  returns an allocated copy of a TLV inside another.
  content is also malloced
*/
tlv_t * extractTLV(tlv_t * parent, off_t index, size_t n){
  tlv_t * target = (tlv_t *)malloc(sizeof(tlv_t) );

  if(target == NULL){
    perror("Extraction Malloc failed ");
    return NULL;
  }
  memcpy(target, parent->content+index, 1);  

  if(target->type == PAD1){
    target->length = 0;
    target->content = NULL;
    return target;
  }else{
    memcpy(target+1, parent->content+index+1, 3);
    target->content = (byte *)malloc(sizeof(byte)*target->length);
    if(target->content == NULL){
      perror("Extraction Malloc failed");
      free(target);
      return NULL;
    }
    memcpy(target+4, parent->content+index+4, n-4);
    return target;
  }
}


/*
  Prints a tlv on the standard output, the lvl indicates the depth
*/
void printtlv(tlv_t * t, unsigned int lvl){
  char * separator = "----";
  char textbuffer[BUFFER_SIZE+1];
  int date;
  unsigned int j = 0;
  if(t->type == PAD1 || t->type == PADN)
    return;
  
  printf("_\n");
  printsn(separator, lvl);
  if(t->type==TEXT){
        printf("type = texte | length = %d\n",t->length);
    printsn(separator, lvl);
    while( j < t->length){
      memset(textbuffer, 0, BUFFER_SIZE+1);
      memcpy(textbuffer, t->content+j, min(t->length, BUFFER_SIZE) );
      printf("%s\n", textbuffer);
      j += strlen(textbuffer);
    }
    return;
  }
  
  if(t->type==JPEG){
    printf("type = image JPEG | length = %d\n",t->length);
    printsn(separator, lvl);
    printf("(image JPEG)\n");
    return;
  }
  
  if(t->type == PNG){
    printf("type = image PNG | length = %d\n",t->length);
    printsn(separator, lvl);
    printf("(image PNG)\n");
    return;
  }

  if(t->type == MP3){
    printf("type = musique MP3 | length = %d\n",t->length);
    printsn(separator, lvl);
    printf("(musique MP3)\n");
    return;
  }

  if(t->type == OGG){
    printf("type = musique OGG | length = %d\n",t->length);
    printsn(separator, lvl);
    printf("(musique OGGVORBIS)\n");
    return;
  }

  if(t->type == VIDEO){
    printf("type = video Theora | length = %d\n",t->length);
    printsn(separator, lvl);
    printf("(video OGV)\n");
    return;
  }

  if(t->type == WEBM){
    printf("type = video WebM | length = %d\n",t->length);
    printsn(separator, lvl);
    printf("(video WebM)\n");
    return;
  }
  
  if(t->type == DATED){
    char * strdate;
    printf("type = dated | length = %d\n",t->length);
    date = ( (unsigned int *) (t->content))[0];
    strdate = UTCtoString(localendian(date) );
    printsn(separator, lvl);
    printf("date: %s (%u)\n", strdate, date);
    free(strdate);
    return;
  }
  
  if(t->type==COMPOUND){
    printf("type = compound | length = %d\n", t->length);
    printsn(separator, lvl);
    return ;
  }
  /*if we are here, it means that the type is unknown*/
  printf("type = unknown (%d) | legth = %d\n", t->type, t->length);
  
}


/*
  Reads a TLV from the file descriptor and injects it in the pointer,
  modifies the offset.
  returns NULL in case of error.
*/
tlv_t * readTLV(int fd){
  
  tlv_t * tlv; /* result */
  int nb; /* number of bytes read from the file */
  unsigned int remaining; /* nb of bytes left to read (length of the TLV's content) */
  /*   void * contentaddr;*/
  byte buffer[BUFFER_SIZE];
  memset(buffer, 0, BUFFER_SIZE);
  

  tlv = (tlv_t *)(malloc(sizeof(tlv_t) ) );
  if(tlv == NULL){
    perror("Malloc a TLV failed");
    return NULL;
  }
  
  /* getting type */
  if( (nb = read(fd, buffer, 1)) < 1){
    if(nb < 0){
      perror("Read error");
      free(tlv);
      return NULL;
    }else{ 
      printf("\n\nEND\n");
    }
  }
  tlv->type = buffer[0];
  fprintf(stderr, "read a TLV type %d\n", tlv->type);
  if(tlv->type == PAD1){
    tlv->length = 0;
    tlv->content = NULL;
    return tlv;
  }

  memset(buffer, 0, BUFFER_SIZE);
  if( (nb = read(fd, buffer, 3 )) != 3 ){
    perror("Incorrect length for TLV");
    free(tlv);    
    return NULL;
  }
  tlv->length = computelength(buffer);
  
  tlv->content = (byte *) malloc( sizeof(byte)*tlv->length);
  if(tlv->content == NULL){
    errsys("Malloc content failed : ");
    return NULL;
  }
  memset(tlv->content, 0, tlv->length); /* it will be easier to add content */

  /* In case of dated or compound, we must not read right now */
  if(tlv->type == DATED){
    if( (nb = read(fd, buffer, 4) ) != 4){
      perror("reading date");
      return NULL;
    }
    memcpy(tlv->content, buffer, 4);
    return tlv;
  }

  if(tlv->type == COMPOUND){
    return tlv;
  }


  remaining = tlv->length;
  while(remaining > 0){
    /* while we have not read as many bytes as the TLV wants */
    memset(buffer, 0, BUFFER_SIZE);
    if( (nb = read(fd, buffer, min(BUFFER_SIZE, remaining) ) ) > 0){
      memcpy(tlv->content+(tlv->length-remaining), buffer, nb);
      remaining -= nb;
    }
    if(nb == -1){
      errsys("Read problem");
    } 
  }
  return tlv;
}


