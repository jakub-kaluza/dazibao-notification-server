// Thread body thread which monitors adding and removing of readers of dazibao.
// Reader is removed from readersList when socket peer hungs up
//

#include "dzbNotify.h"

// readersList contains all file descriptors of stream sockets of clients
// reading dazibao files
extern node_t * readersList;

// readersCounter number of elements on redersList
extern volatile int readersCounter;
// listLock
extern pthread_mutex_t listLock;
// tableLock Synchronize access to readersList and readersCounter
extern pthread_mutex_t tableLock;

void*
addReader(void* arg)
{
	struct sockaddr_un addr;
	int sfd, cfd;
    struct pollfd *pollFd;

    int j, r, temp;


	sfd=socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0);

	if(sfd<0)
		errExit("socket");

	memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;

    strncpy(addr.sun_path, getenv("HOME"), 107);
    strncat(addr.sun_path, "/", 107);
    strncat(addr.sun_path, ".dazibao-notification-socket", 107);

    if(remove(addr.sun_path)<0 && errno!=ENOENT)
		errExit(" remove ");

	if(bind(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un))==-1)
		errExit("bind");

	if(listen(sfd, BACKLOG)<0)
		errExit("Listen");

	// Loop responsible for adding and removing elements from  the filedescrpitors list
	for(;;)
	{
		// p pointer used for traversing readersList
		node_t *p = readersList;

		cfd = accept(sfd, NULL, NULL);

		// sfd is SOCK_NONBLOCK so if error was different than EAGAIN causes error and exit
		if(cfd<0 && EAGAIN != errno)
			errExit("accept");
        // if accept returned correct file descriptor
        if(cfd>=0)
        {
            printf("New client is now connected\n");
            pthread_mutex_lock(&tableLock); // synchronize

            // add node to list with string "c" which is needed due to my list implemenatation
            // and cfd which is correct file descriptor
            addNodeToBeginning(&readersList, "c", cfd);
            readersCounter++;

            pthread_mutex_unlock(&tableLock); // endsynchronize
        }

        // if readersList is not empty check whether any of client programs was closed
        if(p!=NULL){

            pthread_mutex_lock(&tableLock); // synchronize

            pollFd = calloc(readersCounter, sizeof(struct pollfd));
            if (pollFd == NULL)
                errExit("malloc");

            // prepare table of poll events using file descriptos in readersList
            // p points at readersList head, then traverse whole list => p!=NULL
            for (j = 0; p!= NULL; j++) {

                pollFd[j].fd = p->nr;
                pollFd[j].events = POLLRDHUP;
                p=p->next;
            }

            // r number of file desciptors which upheld an event
            r = poll(pollFd, readersCounter,  30);

            if (r == -1)
                errExit("poll");


            temp = readersCounter;

            // for each file descriptor check if it was closed if so
            // decrement readersCounter
            for (j = 0; j < temp; j++)
                if (pollFd[j].revents & POLLRDHUP){
                    printf("a client was disconnected\n");
                    removeNode(&readersList,pollFd[j].fd);
                    readersCounter--;
                }

            pthread_mutex_unlock(&tableLock); // endsynchronize
        }
	}

    return NULL;
}
