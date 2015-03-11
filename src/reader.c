#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/* delete in final release */

/* end of deletion */

#include "dazibao.h"
#include "reader.h"
#include "helpers.h"

#define BUFFER_SIZE 512

#define VIDEO_WIDTH 640
#define VIDEO_HEIGHT 480

#define LGENERAL -1 /* looping flag for general dazibao */
#define LDATED 1 /* looping flag for dated TLV */

void printHTML(char * file, int fd, off_t left, off_t right){
  printf("<!DOCTYPE html>\n");
  printf("<html>\n");
  printf("<head>\n");
  printf("<meta charset = \"utf-8\"/>\n");
  printf("<link rel = \"stylesheet\" href = \"dazibaostyle.css\" />\n");
  printf("<title>Dazibao %s</title>\n", file);
  printf("</head>\n");
  printf("<header>\n");
  printf("<h1>%s</h1>\n", file);
  printf("</header>\n");
  printf("<body>\n");
  printf("<section class=\"daz\">");
  loop(fd, left, right, 0, NULL, TRUE, file);
  printf("</section>");
  printf("</body>\n");
  printf("</html>\n");
}

/* 
   Prints a tlv on the standard output, html style
*/
void tlvtoHTML(tlv_t * t, char * dazname){
  char * buffer; /* for printing the content with printf , adding a '\0' */
  if(t->type == PAD1 || t->type == PADN)
    return;
  if(t->type == TEXT){
    buffer = malloc(sizeof(char)*(t->length+1) );
    memset(buffer, 0, t->length+1);
    memcpy(buffer, t->content, t->length);
    printf("<p>\n");
    printf("%s", buffer);
    printf("</p>\n");
    free(buffer);
    return;
  }
  if(t->type == JPEG || t->type == PNG){
    char * file;
    if(t->type == JPEG)
      file = writeFile(dazname, ".jpg", t->length, t->content);
    else
      file = writeFile(dazname, ".png", t->length, t->content);
    printf("<img src=\"%s\" alt=\"missing image\" />\n", file);  
    free(file);
    return;
  }
  if(t->type == MP3 || t->type == OGG){
    char * file;
    printf("<audio controls>\n");
    if(t->type == MP3){
      file = writeFile(dazname, ".mp3", t->length, t->content);
      printf("<source src=\"%s\" type=\"audio/mpeg\" >\n", file);
    }else{
      file = writeFile(dazname, ".ogg", t->length, t->content);
      printf("<source src=\"%s\" type=\"audio/ogg\" >\n", file);
    }
    free(file);
    printf("Your browser does not support audio\n</audio><br/>");
    return;
  }
  if(t->type == VIDEO || t->type == WEBM){ 
    char * file;
    printf("<video width=\"%d\" height=\"%d\" controls=\"controls\">\n", \
	   VIDEO_WIDTH, VIDEO_HEIGHT);
    if(t->type == VIDEO){
      file = writeFile(dazname, ".ogv", t->length, t->content);
      printf("<source src=\"%s\" type=\"video/ogg\" />\n", file);
    }else{
      file = writeFile(dazname, ".webm", t->length, t->content);
      printf("<source src=\"%s\" type=\"video/ogg\" />\n", file);
    }
    free(file);
    printf("Your browser does not support video\n</video><br/>");
    return;
  }
  if(t->type == DATED){
    char * strdate;
    unsigned int date;
    printf("<div>\n");
    date = ( (unsigned int *) (t->content))[0];
    strdate = UTCtoString(localendian(date) );
    printf("<em class=\"dated\">%s</em><br />\n", strdate);
    free(strdate);
    return;
  
  }
  if(t->type == COMPOUND){
    printf("<section class=\"compound\">\n");
  }
}


/*
  Write the file with name ".name_num.type" where num is a static index.
  The size of the file and its content must be providen.
  Returns the name of the file.
*/
char * writeFile(char * name, char * ext, unsigned int size, byte * content){ 
  unsigned int remaining, nb;
  int fd;
  byte buffer[BUFFER_SIZE];
  char * file = (char *)malloc(sizeof(char)*256 );
  char * nbstr = (char *)malloc(sizeof(char)*4); 
  /* this limits the number of files to 10000... is that enough ? */
  static unsigned int nbres = 0;
  memset(file, 0, 256);
  memset(nbstr, 0, 4);
  strcat(file, "."); /* we want the file to be hidden */
  strcat(file, name);
  sprintf(nbstr, "%d", nbres++);
  strcat(file, nbstr);
  strcat(file, ext); 
  fd = open(file, O_CREAT|O_TRUNC|O_RDWR, 0611);
  if(fd == -1){
    fprintf(stderr, "Problem when reconstructing a %s file\n", ext);
    return NULL;
  }
  free(nbstr);
  remaining = size;
 
 
  while(remaining > 0){
    unsigned int towrite = min(BUFFER_SIZE, remaining);
    memset(buffer, 0, BUFFER_SIZE);
    memcpy(buffer, content+(size-remaining), towrite);
    if( (nb = write(fd, buffer, towrite)) != towrite){
      fprintf(stderr, "Problem creating a %s file\n", ext);
      return NULL;
    }
    remaining -= nb;
  }
  close(fd);
  return file;
  
}


/*
  The main loop for reading a dazibao :
  parsing the file, analyses the bytes in order to extract a type and a length.
  if it could get a type and a length, then it analyses "length" bytes to product
  the content.
  Then, displays the tlv and loops for a following tlv.
  Limits left and right indicate the offset we must not overflow (for dated and compound recursion). 
  We are actually searching for TLV in the area [ left ; right ],
  which is normally [current_offset; current_offset+length-4] for nested TLV
  or [current_offset ; EOF]
  Lvl indicated the depth of displaying, a higher level has a parent TLV
*/
void loop(int fd, off_t left, off_t right, int lvl, tlv_t * parent, bool htmlflag, char * file){
  tlv_t * tlv;
  /* while there a byte left to read, there must be a TLV to read */
  while( left < right){
    tlv = readTLV(fd);
    if(lvl > 0 && parent != NULL){
      appendTLV(parent, tlv);
    }
    
    if(tlv == NULL){
      break;
    }
    /* Here, we have read a TLV, so why not display it ? */
    if(htmlflag)
      tlvtoHTML(tlv, file);
    else
      printtlv(tlv, lvl);

    left += 1;
    if(tlv->type != PAD1){ 
      left += 3;
      if(tlv->type == DATED){
	left += 4;
	if(tlv->length>4)loop(fd, left, left+tlv->length-4, lvl+1, tlv, htmlflag, file);
	if(htmlflag)
	  printf("</div>\n");
      }
      if(tlv->type == COMPOUND){
	if(tlv->length!=0)loop(fd, left, left+tlv->length-4, lvl+1, tlv, htmlflag, file);
	if(htmlflag)
	  printf("</section>\n");
      }
      left += tlv->length;
    }
	
    freeTLV(tlv);
  }

}





/*
  The "real" program : opens a file, locking it, and parses it, then closes it
*/
void readdazibao(char * f, bool htmlflag){
  struct stat st;
  int fd = lockopen(f, O_RDONLY, 0, LOCK_SH);
  if(checkversion(fd) == -1 ){
    close(fd);
    errno = 5;
    errsys("Incorrect dazibao");
  }
  if(fstat(fd, &st) == -1)
    errsys("fstat");
  if(htmlflag)
    printHTML(f, fd, 4, st.st_size);
  else
    loop(fd, 4, st.st_size, 0, NULL, FALSE, NULL);
  close(fd);
}


int main(int argc, char ** argv){
  if(argc < 2){
    fprintf(stderr, "You must specify an input dazibao\n");
    return (EXIT_FAILURE);
  }else{
    int opt;
    extern int opterr;
    char format [] = "h";
    bool htmlflag = FALSE;
    opterr = 1;
    while((opt = getopt(argc, argv, format)) != -1)
      switch(opt){
      case 'h':
	htmlflag = TRUE;
	break;
      }
    readdazibao(argv[optind], htmlflag);
  }
  return(EXIT_SUCCESS);
}

