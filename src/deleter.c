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
#include "deleter.h"


/* loop for printing compound and dated */
void subloop(int fd, off_t left, off_t right, int lvl){
  tlv_t * tlv;
  while( left < right){
    tlv = readTLV(fd);
    
    if(tlv == NULL){
      break;
    }
    /* Here, we have read a TLV, so why not display it ? */
    printtlv(tlv, lvl);
    
    left += 1;
    if(tlv->type != PAD1){ 
      left += 3;
      if(tlv->type == DATED){
	left += 4;
	/*if dated is not empty, print its content*/
	if(tlv->length>4)subloop(fd, left, left+tlv->length-4-4, lvl+1);
      }
      
      if(tlv->type == COMPOUND){
	/*if compound is not empty, print its content*/
	if(tlv->length>0)subloop(fd, left, left+tlv->length-4, lvl+1);
      }
      left += tlv->length;
    }
    freeTLV(tlv);
  }
  
}



/*main loop for removal. We explore the file, in order to find TLVs we want to remove. Its functioning is similar to the function 'loop' used in reader, but now we can choose some actions, like switching TLVs, removing them or quit the program*/
void loop_deleter(int fd, off_t left, off_t right, int lvl, tlv_t * parent){
  tlv_t * tlv;
  int j=0;/*for the action 'jump' */
  while( left < right){
    if(j>0) j--;
    if(lseek(fd,left,SEEK_SET)<0)errsys("Lseek problem");
    tlv = readTLV(fd);
    
    if(tlv == NULL){
      break;
    }
    
    if((tlv->type)!=PADN && (tlv->type)!=PAD1){
      if(parent==NULL){
	printsn("\n", 40);
      }
      else{
	printf("\n\n");
	printtlv(parent,lvl-1);
      }
      printsn("*", 79);
      printf("\n");
      printtlv(tlv, lvl);
      if((tlv->type == DATED) && (tlv->length>4)){
	subloop(fd, left+8, left+8+tlv->length-4, lvl+1);
      }
      if((tlv->type == COMPOUND) && (tlv->length>0)){
	subloop(fd, left+4, left+4+tlv->length-4, lvl+1);
      }
      
      printsn("*", 79);
      printf("\n");
      if(lseek(fd,left,SEEK_SET)<0)errsys("Lseek problem");
      if(j==0) get_action(fd,left,lvl,tlv,&j);
    }
    
    
    left += 1;
    if(tlv->type != PAD1){ 
      left += 3;
      left += tlv->length;
    }
    freeTLV(tlv);
  }
  if(parent==NULL){
    printf("\n\n\nDone..\n\n\n");
  }
}

/*displays all possible actions, reads the input from user, checks if it's correct and then performs the chosen action.*/
void get_action(int fd, off_t left, int lvl, tlv_t * tlv,int * j){
  char padn=PADN;
  char buff[30];
  
  while(1){
    printf("Possible actions:\ndel-delete\nn-next TLV\njump <number>-jump of <number> TLVs\nexit-exit :)");
    if(tlv->type==DATED && (tlv->length>4)) printf("\ne-explore the 'dated'");
    if(tlv->type==COMPOUND && (tlv->length>0)) printf("\ne-explore the 'compound'");
    printf("\n\n");
    printf("Choose action:\n");
    if(read(STDIN_FILENO,buff,30)<0){
      errsys("read: get_option");
    }
    if(strncmp(buff,"del\n",4)==0){
      /*remove the TLV*/
      padn=PADN;
      lseek(fd,left,SEEK_SET);
      if(write(fd,(const void *)&padn,1)<1){
	perror("Write problem (function get_option)");
      }
      if(lseek(fd,(tlv->length)+3,SEEK_CUR)<0)errsys("Lseek problem");
      printf("\n\n\n\n\nTLV Deleted!\n");
      sleep(1);
      break;
    }
    
    if(strncmp(buff,"exit\n",5)==0){
      /*quit the program*/
      exit(0);
    }
    
    if(strncmp(buff,"n\n",2)==0){
      /*go to the next TLV*/
      break;
    }
    
    if((strncmp(buff,"e\n",2)==0) && (tlv->type==DATED) && (tlv->length>4)){
      /*explore/enter into a 'dated' TLV*/
      left += 8;
      loop_deleter(fd, left, left+tlv->length-4, lvl+1, tlv);
      break;
    }
    if((strncmp(buff,"e\n",2)==0) && (tlv->type==COMPOUND) && (tlv->length>0)){
      /*explore/enter into a 'compound' TLV*/
      left+=4;
      loop_deleter(fd, left, left+tlv->length-4, lvl+1, tlv);
      break;
    }
    
    if((strncmp(buff,"jump",4)==0)){
      /*make a jump, which means ignore some number of TLVs*/
      if((sscanf(buff+5,"%d",j)!=1)  || ((*j)<0)){
	*j=0;
      }
      else break;
    }
    
    /*if the chosen action is not correct*/
    printf("\n\nWrong action\n\n");
  }
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
  
  loop_deleter(fd, 4, st.st_size, 0, NULL);
  return(EXIT_SUCCESS);
}
