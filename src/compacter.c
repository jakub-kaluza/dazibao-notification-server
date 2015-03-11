#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "dazibao.h"
#include "helpers.h"
#include "compacter.h"

unsigned long offset_write;
int modified=0;/*indicates if the file has to be modified*/

/*main loop for compacter*/
int loop_compacter(int fd, off_t left, off_t right, int lvl, tlv_t * parent){
  tlv_t * tlv;
  unsigned int new_length;
  unsigned int cumulated_length=0;
  unsigned long offset_read;
  byte * buffer=(byte *)malloc(3*sizeof(byte));
  unsigned long offset_length_tlv;/*will be useful for compound and dated*/
  while( left < right){
    if(lseek(fd,left,SEEK_SET)<0) errsys("Lseek problem");
    tlv = readtlv_compacter(fd,&offset_length_tlv);
    new_length=tlv->length;
    if(tlv == NULL){
      break;
    }
    
    if((tlv->type == DATED) && (tlv->length>4)){
      new_length=loop_compacter(fd, left+8, left+8+tlv->length-4, lvl+1, tlv);
      new_length+=4; /*length of 'date'*/
      offset_read=lseek(fd,0,SEEK_CUR);
      if(new_length!=(tlv->length)){
	if(lseek(fd,offset_length_tlv, SEEK_SET)<0) errsys("Lseek problem");
	arrayedlength(bigendian(new_length),buffer);
	/*change the length of the tlv*/
	if(write(fd,buffer,3)<3){
	  errsys("write problem (loop_compacter)");
	}
	if(lseek(fd,offset_read,SEEK_SET)<0)errsys("Lseek problem");
      }
    }
    if((tlv->type == COMPOUND) && (tlv->length>0)){
      new_length=loop_compacter(fd, left+4, left+4+tlv->length-4, lvl+1, tlv);
      offset_read=lseek(fd,0,SEEK_CUR);
      if(new_length!=(tlv->length)){
	if(lseek(fd,offset_length_tlv, SEEK_SET)<0) errsys("Lseek problem");
	arrayedlength(bigendian(new_length),buffer);
	/*change the length of the tlv*/
	if(write(fd,buffer,3)<3){
	  errsys("write problem (loop_compacter)");
	}
	if(lseek(fd,offset_read,SEEK_SET)<0)errsys("Lseek problem");
      }
    }
    
    left += 1;
    if(tlv->type != PAD1){ 
      left += 3;
      left += tlv->length;
    }
    tlv->length=new_length;
    if(  ((tlv->type)!=PADN) && (tlv->type)!=PAD1){
      cumulated_length+=4+(tlv->length);
    }
    freeTLV(tlv);
  }
  if((parent==NULL)){
    if(modified){
      if(ftruncate(fd,offset_write)<0){
	errsys("ftruncate problem");
      }
      printf("New size of the file:%d\n",(int)offset_write);
    }
    else printf("The file is already compacted.\n"); 
  }
  
  return cumulated_length;
}



/*reads a TLV and write it at the position offset_write, if needed*/
tlv_t * readtlv_compacter(int fd,unsigned long * p_offset_length){
  
  
  int nb; /* number of bytes read from the file */
  unsigned int remaining; /* nb of bytes left to read (length of the TLV's content) */
  /*   void * contentaddr;*/
  byte buffer[BUFFER_SIZE];
  int offset_read;
  tlv_t * tlv;
  memset(buffer, 0, BUFFER_SIZE);
  tlv = (tlv_t *)(malloc(sizeof(tlv_t) ) );
  if(tlv == NULL){
    perror("Malloc a TLV failed");
    return NULL;
  }
  tlv->content=NULL;
  /* getting type */
  if( (nb = read(fd, buffer, 1)) < 1){
    if(nb < 0){
      perror("Read error (readtlv_compacter)");
      return NULL;
    }else{ 
      printf("\n\nEND\n");
      exit(1);
    }
  }
  tlv->type = buffer[0];
  if(tlv->type == PAD1){
    printf("We've got a PAD1 at offset %d. Let's clear it.\n",(int)lseek(fd,0,SEEK_CUR)-1);
    tlv->length = 0;
    if(!modified){
      modified=1;
      offset_write=lseek(fd,0,SEEK_CUR)-1;
    }
    return tlv;
  }
  
  memset(buffer, 0, BUFFER_SIZE);
  if( (nb = read(fd, buffer, 3 )) != 3 ){
    perror("Incorrect length for TLV");   
    return NULL;
  }
  tlv->length = computelength(buffer);
  if(tlv->type==PADN){
    printf("We've got a PADN of length %d at offset %d. Let's clear it.\n",(int)tlv->length,(int)lseek(fd,0,SEEK_CUR)-4);
    
    if(!modified){
      modified=1;
      offset_write=lseek(fd,0,SEEK_CUR)-4;
    }
    if(lseek(fd,tlv->length,SEEK_CUR)<0) errsys("Lseek problem");
    return tlv;
  }
  if(((tlv->type!=PAD1) && (tlv->type!=PADN))){
    if(modified){
      offset_read=lseek(fd,0,SEEK_CUR);
      if(lseek(fd,offset_write, SEEK_SET)<0)errsys("Lseek problem");
      /*write the type*/
      if(write(fd,(const void *)&(tlv->type),1)<1)errsys("Write problem");
      *p_offset_length=lseek(fd,0,SEEK_CUR);/*remember the offset of field 'length' of this TLV. Will be useful for changing length if needed*/
      write(fd,buffer,3);/*write the length*/
      offset_write=lseek(fd,0,SEEK_CUR);/*update offset_write*/
      if(lseek(fd,offset_read,SEEK_SET)<0)errsys("Lseek problem");/*come back to the read offset*/
    }
    else{
      *p_offset_length=lseek(fd,0,SEEK_CUR)-3;/*remember the offset of field 'length' of this TLV. Will be useful for changing length if needed*/
    }
  }
  
  /* In case of dated or compound, we must not read right now */
  if((tlv->type == DATED)){
    /*read only date*/
    if( (nb = read(fd, buffer, 4) ) != 4){
      perror("reading date");
      return NULL;
    }
    if(modified){
      offset_read=lseek(fd,0,SEEK_CUR);
      if(lseek(fd,offset_write, SEEK_SET)<0)errsys("Lseek problem");
      /*write the date*/
      if(write(fd,buffer,4)<4) errsys("Write problem");
      offset_write=lseek(fd,0,SEEK_CUR);/*update the offset for writing*/
      if(lseek(fd,offset_read,SEEK_SET)<0) errsys("Lseek problem");
    }
    return tlv;
  }
  
  if(tlv->type == COMPOUND){
    return tlv;
  }
  
  
  remaining = tlv->length;
  if(!modified){
    /*ignore the TLV if no modification is needed*/
    if(lseek(fd,tlv->length,SEEK_CUR)<0) errsys("Lseek problem");
  }
  else{/*if the file has to be modified*/
    while(remaining > 0){
      /* while we have not read as many bytes as the TLV wants */
      memset(buffer, 0, BUFFER_SIZE);
      if( (nb = read(fd, buffer, min(BUFFER_SIZE, remaining) ) ) > 0){
	offset_read=lseek(fd,0,SEEK_CUR);
	if(lseek(fd,offset_write, SEEK_SET)<0)errsys("Lseek problem");
	/*write the content*/
	if(write(fd,buffer,nb)<nb){
	  errsys("Write problem");
	};
	
	offset_write=lseek(fd,0,SEEK_CUR);
	if(lseek(fd,offset_read,SEEK_SET)<0)errsys("Lseek problem");
	remaining -= nb;
      }
      if(nb == -1){
	errsys("Read problem");
      } 
    }
  }
  return tlv;
}


int main(int argc, char ** argv){
  struct stat st;
  int fd;
  if(argc < 2){
    fprintf(stderr, "You must specify an input dazibao\n");
    return (EXIT_FAILURE);
  }
  fd=open(argv[1],O_RDWR);
  if(fd<0){
    errsys("open");
  }
  if(flock(fd, LOCK_EX) < 0 ){
    errsys("Could not flock the file");
  }
  if(checkversion(fd) == -1 ){
    close(fd);
    errsys("Incorrect dazibao");
  }
  if(fstat(fd, &st) == -1)
    errsys("fstat");
  offset_write=4;
  printf("Size of the file:%d\n",(int)st.st_size);
  loop_compacter(fd, 4, st.st_size, 0, NULL);
  close(fd);
  return(EXIT_SUCCESS);
}
