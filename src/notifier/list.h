#ifndef LIST_H
#define LIST_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define errExit(msg) { perror(msg); exit(EXIT_FAILURE); }

typedef struct node node_t;
struct node
{
	node_t * next;
	char * data;
	int nr;
	int flag;
};

void initializeList(node_t** phead, char * s, int val);
void deinitializeList(node_t **phead);
void addNodeToBeginning(node_t** phead, char * s, int val);
void removeNode(node_t** phead, int val);
node_t* searchNode(node_t *head, int val);
node_t ** searchNodeP(node_t ** phead, int val);

void printList(node_t* head);


#endif
