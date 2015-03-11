// file contains impementation of all minor functions used in dzbNotify program
#include "dzbNotify.h"

extern node_t * readersList;
extern volatile int readersCounter;
extern node_t * dazibaolist;
extern pthread_mutex_t listLock;
extern pthread_mutex_t tableLock;


// proper usage of dzbNotify from console
void
usage()
{
    fprintf(stderr,"Specify first argument as a dazibao files paths' list\n");
}
// function sends messages to all file descriptors on readersList
void
sendInotifyEvent(struct inotify_event *i)
{
    int j;
    char msg[100];

    pthread_mutex_lock(&listLock); // synchronize

    node_t *p = searchNode(dazibaolist, i->wd);

    pthread_mutex_unlock(&listLock); // endsynchronize
    // create message
    sprintf(msg,"C%s", p->data);
    strcat(msg,"\n");

    pthread_mutex_lock(&tableLock); // synchronize
    // loop sends message to all file descriptors on the readersList
    p = readersList;
    for(j =readersCounter-1 ; j>-1; --j){

        if(write(p->nr,msg, strlen(msg))==-1)
            errExit("write");
        p = p->next;
    }
    pthread_mutex_unlock(&tableLock); // endsynchronize
}
// function initializes mutexes and dzbPathListHead
void
InitializeServer(node_t **dzbPathListHead, char * path)
{

    char s[PATH_MAX];
    FILE* dzbPaths = fopen(path, "r+");
    int i=0;
    if(dzbPaths==NULL)
        errExit("fopen");

    while(1){
        if(fscanf(dzbPaths,"%s",s)<0)
            break;
        addNodeToBeginning(dzbPathListHead, s, i);
        i++;
    }

    if(i==0)
        errExit("Dazibao startup file contains no paths\n");

    fclose(dzbPaths);
    readersCounter = 0;

    if (pthread_mutex_init(&listLock, NULL) != 0)
        errExit("pthread_mutex_init");

    if (pthread_mutex_init(&tableLock, NULL) != 0)
        errExit("pthread_mutex_init");

}
