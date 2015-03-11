#include "writer.h"
#include "dazibao.h"
#include "helpers.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define BUFFER_SIZE 512


/*
  Shows how to use the writer
*/
void showhelp(){
  printsn("==", 40);
  printf("\n");
  printf("./writer [options] <file> <expressions>\n");
  printsn("-", 40);
  printf("\n");
  printf("options : \nd -> adds the date to the \"general\" TLV\n");
  printf("-h -> show the help\n");
  printf("expression : \n");
  printf("<type>{<content>}\n");
  printf("type -> 1 (PAD1), n (PADN), t (TEXT), p (PNG), j (JPEG), m (MP3), o (OGG), v (OGV videos), w (WebM videos), d (DATED), c (COMPOUND)\n");
  printf("content can be surrounded by \" to espace characters.");
  printf("for pad1, nothing to add (no content).\n");
  printf("for padn, just enter the size.\n");
  printf("for texts, just type the text.\n");
  printf("for images, audio, video : put the name of the file.\n");
  printf("for dated, put the date, '-', and the sub-TLV (see \"examples\"). If the date is a number, it represents the UTC time. If it is any litteral, it will be computed by the program.\n");
  printf("for compound, insert the list of TLVs separated by '-' (see \"examples\").\n");
  printf("any unknown format will be treated like a padN.\n");
  printf("anything between the last expected data and the closing '}' will be ignored (the program accepts some errors).\n");
  printf("examples : \n");
  printf("c{t{\"family vacations\"}-d{12345-j{photo.jpg}}-t{yeah}}\n");
  printf("will result in a Compound : \n--text : \"family vacations\"\n--dated : 12345 UTC Time\n----Jpeg : content of photo.jpg\n--text : yeah\n");
  printsn("==", 40);
  printf("\n\n");
}

/*
  write the current version as a header
*/
int writeversion(int fd){
  byte nb, version, mbz[2];
  int w;
  byte buffer[4];
  nb = MAGICNUMBER;
  version = VERSION;
  mbz[0] = 0;
  mbz[1] = 42;
  buffer[0] = nb;
  buffer[1] = version;
  buffer[2] = mbz[0];
  buffer[3] = mbz[1];
  if( (w = write(fd, buffer, 4) ) != 4 ){
    perror("write version");
    return -1;
  }
  return 0;
}

/*
  Attempts to write the TLV at the end of the given file.
  Returns -1 if an error occured.
*/
int writetlv(tlv_t * tlv, int fd){
  byte buffer[BUFFER_SIZE];
  int remaining = 0; /* numbers of bytes left to write
		    (in case the content is > BUFFER_SIZE */
  int nb = 0; /* number of bytes written at each pass */
  byte * out_length;
  off_t offset = 0;

  /* if the file is empty, we can add a header, otherwise, we should check the header first */
  if( (offset = lseek(fd, 0, SEEK_CUR)) == 0){
    if(writeversion(fd) == -1)
      return -1;
  }else{
    if(checkversion(fd) == -1)
      return -1;
  }
  /* reaching the end of the file */
  
  memset(buffer, 0, BUFFER_SIZE);
  while( (nb = read(fd, buffer, BUFFER_SIZE) ) > 0){}
  if(nb == -1){
    errsys("Problem reaching the end of file");
  }


 memset(buffer, 0, BUFFER_SIZE);

  /* writing type */
  memcpy(buffer, &(tlv->type), 1);
  if( (nb = write(fd, &(tlv->type), 1) ) != 1){
    perror("Problem write");
    return -1;
  }
  
  if(tlv->type == PAD1){
    return 0;
  }
  
  /* not a PAD1, writing is length */
  out_length = (byte * )malloc(sizeof(byte)*3);
  if(out_length == NULL){
    perror("Malloc failed");
    return -1;
  }
  arrayedlength( bigendian(tlv->length), out_length);
  
  if( (nb = write(fd, out_length, 3) ) < 1){
    perror("Problem write");
    return -1;
  }
  free(out_length);

  remaining = tlv->length;
  while(remaining > 0){
    unsigned int towrite = 0;
    nb = 0;
    towrite = min(BUFFER_SIZE, remaining);
    memset(buffer, 0, BUFFER_SIZE);
    /*    memcpy(buffer, &(tlv)+(sizeof(tlv)-remaining), min(BUFFER_SIZE, remaining) ); */
    memcpy(buffer, (tlv->content)+(tlv->length-remaining), towrite );

    if( (unsigned int)(nb = write(fd, buffer, towrite ) ) <  (towrite) ){
      perror("writing tlv");
      return -1;
    }
    remaining -= nb;
  }
  return 0;
}


/*
  copies files
*/
int cp(char * original, char * dest){
  int pid;
  switch(pid = fork() ){

  case -1:
    errsys("Fork for cp");
    break;
    
  case 0:
    if(execl("/bin/cp", "cp", "-p", original, dest, NULL) == -1){
      errsys("Could not cp");
    }
    break;

  default:
    if( wait(&pid) == -1)
      errsys("Wait cp");
    break;
  }
  return 0;
}


/*
  Tries to create a TLV matching the given expression and returns it.
  Returns NULL in case of error or if the TLV is incorrect.
  If a TLV has been created, it must be freed.
*/
tlv_t * exprtoTLV(char * expr){
  tlv_t * tlv; /* the TLV to be returned */
  char type; /* the type of the TLV */
  char * substring; /* the substring between the first '{' and the last '}' */
  int sublength; /* the length of this substring */
  fprintf(stderr, "extracting a TLV from \"%s\"\n", expr);
  tlv = (tlv_t *)malloc(sizeof(tlv_t) );
  type = expr[0];
  if(tlv == NULL){
    perror("Malloc failed");
    return NULL;
  }
  switch(type){
  case '1':
    tlv->type = PAD1;
    tlv->length = 0;
    tlv->content = NULL;
    return tlv;
    break;

  case 'n':
    tlv->type = PADN;
    break;

  case 't':
    tlv->type = TEXT;
    break;

  case 'p':
    tlv->type = PNG;
    break;

  case 'j':
    tlv->type = JPEG;
    break;

  case 'c':
    tlv->type = COMPOUND;
    break;
    
  case 'd':
    tlv->type = DATED;
    break;

  case 'm':
    tlv->type = MP3;
    break;

  case 'o':
    tlv->type = OGG;
    break;

  case 'v':
    tlv->type = VIDEO;
    break;

  case 'w':
    tlv->type = WEBM;
    break;

  default:
    tlv->type = PADN;
    fprintf(stderr, "Warning : The TLV type \"%c\"(%u) is unknown, will be replaced by a PADN\n", type, type);
    break;
  }
  /* parsing the expression between '{' and '}' */
  if(expr[1] != '{'){
    printf("Incorrect format");
    free(tlv);
    return NULL;
  }
  /* extracting the substring :
     between (expr+2) ( after the first '{' and the last '}'*/
  sublength = strrchr(expr, '}') - (expr+2);
  substring = (char * )malloc(sizeof(char) * (sublength+1) );
  memset(substring, 0, sublength+1);
  strncpy(substring, expr+2, sublength);
  substring[sublength] = '\0';
  /*
  printf("---> \"%s\"", substring);
  */
  if(tlv->type == PADN){
    if(sscanf(substring, "%u", &(tlv->length) ) == 0){
      errno = EINVALID;
      perror("Bad format");
      free(tlv);
      free(substring);
      return NULL;
    }
    tlv->content = (byte *)malloc(sizeof(byte)*tlv->length);
    memset(tlv->content, 0, tlv->length);
    free(substring);
    return tlv;
  }else if(tlv->type == TEXT){
    tlv->content = (byte *)malloc(sizeof(byte)*sublength);
    sublength = strlen(substring);
    memcpy(tlv->content, substring, sublength);
    tlv->length = sublength;
  }
  else if(tlv->type == JPEG || tlv->type == PNG 
	  || tlv->type == MP3 || tlv->type == OGG
	  || tlv->type == VIDEO || tlv->type == WEBM){
    exprtofile(substring, tlv);
  }else if(tlv->type == DATED){
    exprtodated(substring, tlv);    
  }else if(tlv->type == COMPOUND){
    exprtocompound(substring, tlv);
  }
  free(substring);
  return tlv;
}

/* 
   extracts a DATED from a string and injects it in the given TLV
*/
tlv_t * exprtodated(char * substring, tlv_t * tlv){
  char * first, * second; /* the two components of the TLV, once split */
  time_t date;
  byte * array;
  tlv_t * subtlv;

  /* splitting the {date;content }*/ 
  second = substring;
  if(second == NULL){
    printf("Bad format for Dated TLV\n");
    free(tlv);
    free(substring);
    return NULL;
  }
  if( (first = strsep(&second, "-")) == NULL){
    printf("Bad format for Dated TLV\n");
    free(tlv);
    free(second);
    free(substring);
    return NULL;
  }

  /* checking that the first part is a number. if not, we will compute the date */
  if(sscanf(first, "%ld", &date) == 0){
    date = time(NULL);
    if(date == -1){
      printf("Warning : couldn't get the current time\n");
    }
  }
  subtlv = exprtoTLV(second);

  /*  free(second); */
  date = bigendian(date);
  array = (byte *) (&date);
  tlv->length = subtlv->length+8;
  tlv->content = (byte *)malloc(sizeof(byte)*(tlv->length) );
  memset(tlv->content, 0, tlv->length);
  /* copying the current date*/
  memcpy(tlv->content, array, 4);

  array = malloc(sizeof(byte) * 3); /* array now contains the length of the sub tlv */
  array = arrayedlength(bigendian(subtlv->length), array);
  /* copying the subTLV */
  memcpy(tlv->content+4, &(subtlv->type), 1);
  memcpy(tlv->content+5, array, 3);
  memcpy(tlv->content+8, subtlv->content, subtlv->length);
  free(array);
  freeTLV(subtlv);
  return tlv;
}


/* 
   extracts a COMPOUND from a string and injects it in the given TLV
*/
tlv_t * exprtocompound(char * substring, tlv_t * tlv){
  int nb = 1, * indexes; /* number of subtlvs, and the indexes of the '-' */
  int i;
  bool counting = TRUE; /* we must not count the '-' inside sub tlvs */
  /* we are sure that there are not more TLVs as the length of the inside string 
     (the higher number is reached when we fill with PAD1 */
  int sublength = strlen(substring);
  tlv_t ** subtlvs; /* will store the tlvs of the compound */
  tlv->length = 0;
  indexes = (int *)malloc(sizeof(int)*sublength);
  /* as we will take indexes +1 & -1, we need the first index to be -1 */
  memset(indexes, -1, sublength);
  /* counting the number of '-' inside the substring which are not inside an other tlv */
  for(i = 0; i < sublength; i++){
    /* printf("[%u] %c\n", i, substring[i]); */
    if(counting && substring[i] == '-'){
      indexes[nb] = i;
      nb++;
      /* printf("found a '-' at %u\n", i); */
    }
    else if(counting && i > 0 && substring[i] == '{' && i > 0 && substring[i-1] != '\\'){
      counting = FALSE;
    }
    else if(!counting && i > 0 && substring[i] == '}' && substring[i-1] != '\\'){
      counting = TRUE;
    }
  }

  indexes[nb] = sublength;
  subtlvs = (tlv_t **)malloc(sizeof(tlv_t *)*nb);
  /*
  for(i = 0;  i < nb; i++){    
    printf("index[%u] = %u\n", i, indexes[i]);
    }*/

  for(i = 0;  i < nb; i++){    
    /* retireving the substrings */
    unsigned int length = indexes[i+1]-indexes[i]-1;
    char * subsubstring; 
    tlv_t * subtlv;
    subsubstring = strndup(substring+indexes[i]+1, length);
    /* subsubstring = malloc(sizeof(char)*(length+1));
       memset(subsubstring, 0, length+1);
       subsubstring = strncat(subsubstring, substring+indexes[i]+1, length );*/
    if(strcmp(subsubstring, "") == 0)
      continue;
    /* creating subtlvs, and adding their length to the container */
    subtlv = exprtoTLV(subsubstring);
    /*    printf("substring \"%s\" (%u-%u = %u) leads to a %u of size %u\n", subsubstring, indexes[i+1]-1, indexes[i]+1, length, subtlv->type, subtlv->length); */
    if(subtlv->type == PAD1){
      tlv->length += 1;
    }else{
      tlv->length += subtlv->length+4;
    }
    subtlvs[i] = subtlv;
    free(subsubstring);
  }

  /* copying the subtlvs to the container */
  tlv->content = (byte *)malloc(sizeof(byte)*tlv->length);
  if(tlv->content == NULL){
    perror("Malloc failed");
    return NULL;
  }else{
    memset(tlv->content, 0, tlv->length);
  }

  free(indexes);
  for(i = 0; i < nb; i++){
    static int insertindex = 0;
    tlv_t * subtlv = subtlvs[i];
    byte * sublength;
    sublength = (byte * )malloc(sizeof(byte)*3);
    arrayedlength( bigendian(subtlv->length), sublength);
    
    if(subtlv->type == PAD1){
      memcpy(tlv->content+insertindex, subtlv, 1);
      insertindex += 1;
    }else{
      memcpy(tlv->content+insertindex, &(subtlv->type), 1);
      memcpy(tlv->content+insertindex+1, sublength, 3);
      memcpy(tlv->content+insertindex+4, subtlv->content, subtlv->length);
      insertindex += subtlv->length+4;
    }
    free(sublength);
    freeTLV(subtlvs[i]);
  }
  free(subtlvs);
  return tlv;
}

/*
  extracts a JPEG/PNG, an audio file or a video  from the file from a string and
  injects it in the given TLV
*/
tlv_t * exprtofile(char * substring, tlv_t * tlv){
  byte * filecontent;
  struct stat * stat = malloc(sizeof(struct stat) );
  int fd;
  fprintf(stdout, "opening file \"%s\"\n", substring);
  fd = lockopen(substring, O_RDONLY, 0, LOCK_SH);

  fstat(fd, stat);
  tlv->length = stat->st_size;
  filecontent = malloc(sizeof(byte)*tlv->length);
  if(filecontent == NULL){
    perror("Malloc failed");
    free(tlv);
    free(substring);
    return NULL;
  }
  readFile(fd, stat->st_size, filecontent);
  free(stat);
  tlv->content = malloc(sizeof(byte) * tlv->length);
  memcpy(tlv->content, filecontent, tlv->length);
  free(filecontent);
  return tlv;
}


/*
  Reads the nb first bytes of a file and returns them in an array.
  dest must be large enough
*/
byte * readFile(int fd, unsigned int nb, byte * dest){
  int i = 0, n = 0;
  /* i is the index where we insert the content,
     it is shifted every step by n, the number of bytes read */
  memset(dest, 0, nb);
  if(dest == NULL){
    perror("Malloc failed");
    return NULL;
  }
  while( (n = read(fd, dest+i, min(BUFFER_SIZE, nb-i) ) ) > 0){
    i += n;
  }
  if(n == -1){
    perror("read problem");
    free(dest);
    return NULL;
  }
  return dest;
}


/*
  Starts the process to write the tlv,
  creating a backup of the file,
  adding the TLV inducted from the expression,
  and deleting the backup or the new dazibao in case of error.
  dateflag tells if the TLV must be wrapped to a dated TLV whose date is "right now"
*/
int addTLV(char * file, char * expr, bool dateflag){
  tlv_t * tlv;
  if(dateflag == TRUE){
    char * expr2;
    expr2 = (char * )malloc(sizeof(char)*(strlen(expr)+6) );
    if(expr2 == NULL){
      perror("Malloc failed");
      return -1;
    }
    memset(expr2, 0, strlen(expr)+6);
    strcat(expr2, "d{d-");
    strcat(expr2, expr);
    strcat(expr2, "}\0");
    fprintf(stdout, "date mode enabled : replacing by \"%s\"\n", expr2);
    tlv = exprtoTLV(expr2);
    free(expr2);
  }else{
    tlv = exprtoTLV(expr);
  }
  
  if(tlv == NULL){
    return -1;
  }
  /* Writing the TLV*/
  if(tlv->length > 0x00ffffff){
    errno = TLVTOOBIG;
    perror("TLV is too big");
    return -1;
  }else{
    int fd = lockopen(file, O_RDWR|O_CREAT, 0664, LOCK_EX);
    int lenf = strlen(file);
    char * bak = (char *) malloc(lenf+5);
    if(bak == NULL)
      errsys("Malloc failed");
    strcpy(bak, file);
    bak[lenf] = '.';
    bak[lenf+1] = 'b';
    bak[lenf+2] = 'a';
    bak[lenf+3] = 'k';
    bak[lenf+4] = '\0';
    cp(file, bak);
    if( writetlv(tlv, fd) == -1){
      free(tlv);
      cp(bak, file);
      close(fd);
      free(bak);
      errsys("Error writing tlv");
    }
    freeTLV(tlv);
    free(bak);
    close(fd);
  }
  return 0;
  
}



int main(int argc, char ** argv){
  int opt;
  extern int opterr;
  char format [] = "dh"; 
  char * file;
  bool dateflag = FALSE; /* dateflag wraps the TLV to a dated or the current instant (except for a Dated TLV) */
  int i;
  while( (opt = getopt(argc, argv, format) ) != -1){
    switch(opt){
    case 'd' :
      dateflag = TRUE;
      break;

    case 'h':
      showhelp();
      break;

    default :
      break;
    }
  }
  if(optind+2 > argc){
    errno = 22;
    errsys("You must specify a file and an expression");
  }
  file = argv[optind];
  for(i = optind+1; i < argc; i++)
    addTLV(file, argv[i], dateflag);

  return EXIT_SUCCESS;
}

