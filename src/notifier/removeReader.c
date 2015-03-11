#include "dzbNotify.h"

extern node_t * readersList;

extern volatile int readersCounter;
extern node_t * dazibaolist;
extern pthread_mutex_t listLock;
extern pthread_mutex_t tableLock;

void*
removeReader(void* arg)
{
    struct pollfd *pollFd;
    node_t *p = readersList;
    int j, r, temp;


    while(1){

        pthread_mutex_lock(&tableLock);

        pollFd = calloc(readersCounter, sizeof(struct pollfd));
        if (pollFd == NULL)
            errExit("malloc");

        fprintf(stderr, "tu4 rc= %d",readersCounter);

        for (j = 0; j < readersCounter; j++) {
            fprintf(stderr, "tu2 %d rc= %d",j,readersCounter);
            pollFd[j].fd = p->nr;
            pollFd[j].events = POLLRDHUP;
            fprintf(stderr, "tu3%d",j);
        }


        r = poll(pollFd, readersCounter,  30);

        if (r == -1)
            errExit("poll");


        temp = readersCounter;
        for (j = 0; j < temp; j++)
            if (pollFd[j].revents & POLLRDHUP){
            removeNode(&readersList,pollFd[j].fd);
            readersCounter--;
            }
        pthread_mutex_unlock(&tableLock);

    }
    return NULL;
}
