#define _GNU_SOURCE

#include <sys/un.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/sendfile.h>
#include <ctype.h>
#include <pthread.h>
#include <linux/limits.h>
#include <sys/stat.h>
#include <sys/inotify.h>
#include <limits.h>
#include <poll.h>
#include <errno.h>

#include "list.h"


#define SV_SOCK_PATH "/home/jsk/.dazibao-notification-socket"
#define BUF_SIZE 10
#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))
#define BACKLOG 5
#define MAX_READERS 10
#define EVENTS_MASK (IN_MODIFY | IN_CLOSE_WRITE)

#define errExit(msg) { perror(msg); exit(EXIT_FAILURE); }

void* removeReader(void* arg);
void* addReader(void* arg);
void* monitorDazibao(void* arg);
void sendInotifyEvent(struct inotify_event *i);
void InitializeServer(node_t **dzbPathListHead, char *path);
void usage();

node_t * dazibaolist;
int readersTable[MAX_READERS];
node_t * readersList;

volatile int readersCounter;
pthread_mutex_t listLock;
pthread_mutex_t tableLock;
