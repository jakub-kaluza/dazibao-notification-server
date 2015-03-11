// pthread responsible for monitoring dazibao files
// checks if elements on dazibaolist were changed
// inotify linux mechanism is used

#include "dzbNotify.h"

// dazibaolist list containg paths to dazibao files with respecting inotify descriptor
extern node_t * dazibaolist;

// listLock synchronize acces to dazibaolist
extern pthread_mutex_t listLock;
// tableLock Synchronize access to readersList and readersCounter
extern pthread_mutex_t tableLock;

// uncomment to debug

//static void
//displayInotifyEvent(struct inotify_event *i)
//{
//  printf("wd =%2d; ", i->wd);
//  if (i->cookie > 0)
//    printf("cookie =%4d; ", i->cookie);
//  printf("mask = ");
//  if (i->mask & IN_ACCESS) printf("IN_ACCESS\n");
//  if (i->mask & IN_ATTRIB) printf("IN_ATTRIB\n");
//  if (i->mask & IN_CLOSE_NOWRITE)printf("IN_CLOSE_NOWRITE\n");
//  if (i->mask & IN_CLOSE_WRITE)printf("IN_CLOSE_WRITE\n");
//  if (i->mask & IN_CREATE)printf("IN_CREATE\n");
//  if (i->mask & IN_DELETE)printf("IN_DELETE\n");
//  if (i->mask & IN_DELETE_SELF)printf("IN_DELETE_SELF\n");
//  if (i->mask & IN_IGNORED)printf("IN_IGNORED\n");
//  if (i->mask & IN_ISDIR)printf("IN_ISDIR\n");
//  if (i->mask & IN_MODIFY)printf("IN_MODIFY\n");
//  if (i->mask & IN_MOVE_SELF)printf("IN_MOVE_SELF\n");
//  if (i->mask & IN_MOVED_FROM)printf("IN_MOVED_FROM\n");
//  if (i->mask & IN_MOVED_TO)printf("IN_MOVED_TO\n");
//  if (i->mask & IN_OPEN)printf("IN_OPEN\n");
//  if (i->mask & IN_Q_OVERFLOW)printf("IN_Q_OVERFLOW\n");
//  if (i->mask & IN_UNMOUNT)printf("IN_UNMOUNT\n");
//  printf("\n");
//  if (i->len > 0)
//    printf("name = %s\n", i->name);
//
//}


void*
monitorDazibao(void* arg)
{
    int inotifyFd, wd, j;
    char buf[BUF_LEN];
    ssize_t numRead;
    char *p;
    struct inotify_event *event;


    // initialize inotify mechanism
    inotifyFd = inotify_init();
    if (inotifyFd == -1)
        errExit("inotify_init");

    pthread_mutex_lock(&listLock); // synchronize

    // lp pointer used to traverse dazibao list
    node_t * lp = dazibaolist;

    for (j = 1; lp!=NULL; j++) {
        wd = inotify_add_watch(inotifyFd, lp->data,  EVENTS_MASK);
        if (wd == -1)
            errExit("inotify_add_watch");
        lp->nr = wd;
        lp=lp->next;
    }

    pthread_mutex_unlock(&listLock); // endsynchronize

    // loop which monitors events on inotifyFd
    for (;;) {
        numRead = read(inotifyFd, buf, BUF_LEN);
        if (numRead == 0)
            errExit("read() from inotify fd returned 0!");

        if (numRead == -1)
            errExit("read");

        printf("\n###############\n");
        for (p = buf; p < buf + numRead; ) {
            event = (struct inotify_event *) p;

            // sends event to all subscribed clients
            if(event->mask & IN_MODIFY)
                 (searchNode(dazibaolist, event->wd))->flag = 1;

            if(event->mask & IN_CLOSE_WRITE && (searchNode(dazibaolist, event->wd))->flag == 1){
                (searchNode(dazibaolist, event->wd))->flag  = 0;
                sendInotifyEvent(event);
                // uncomment to debug
                // displayInotifyEvent(event);
            } else if(event->mask & IN_CLOSE_WRITE)
                (searchNode(dazibaolist, event->wd))->flag = 0;

            p += sizeof(struct inotify_event) + event->len;
        }
    }
    return NULL;
}
