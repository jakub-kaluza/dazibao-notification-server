#include "list.h"


void initializeList(node_t** phead, char * s, int val)
{
	*phead = (node_t*)malloc(sizeof(node_t));
	if((*phead)==NULL)errExit("malloc");

	(*phead)->data = strdup(s);
    (*phead)->nr = val;
    (*phead)->flag = 0;

	if((*phead)->data==NULL)errExit("strdup");

	(*phead)->next = NULL;
}
void removeNode(node_t** phead, int val)
{
	node_t** p;

	p = searchNodeP(phead,val);

	if(*p==NULL)
		return;

        node_t* temp = *p;
        *p = (*p)->next;
		free(temp->data);
		free(temp);

}
void deinitializeList(node_t **phead)
{
	while((*phead)!=NULL)
		removeNode(phead,(*phead)->nr);
}
void addNodeToBeginning(node_t** phead, char * s, int val)
{

	node_t *p;
	// case 1: empty list

	if(*phead==NULL)
		initializeList(phead, s, val);
	// case 2: at least 1 node
	else {

		// search for identical nodes
//		p = *phead;
//		while(p!=NULL){
//			if(strcmp(s,p->data)==0)
//				return;
//            p=p->next;
//		}

		p = (node_t*)malloc(sizeof(node_t));
		if(p==NULL)errExit("malloc");

		p -> next = (*phead);
		(*phead) = p;

		p->data = strdup(s);
		p->nr = val;
		p->flag =0;
		if(p->data==NULL)errExit("strdup");
	}
}
void printList(node_t* head)
{
	while(head!=NULL){
		printf("# %s  #\n",head->data);
		head=head->next;
	}

}
node_t* searchNode(node_t *head, int val)
{
    while(head!=NULL){

        if(head->nr==val){
            return head;
        }
        head=head->next;

    }
    return head;
}

node_t ** searchNodeP(node_t ** phead, int val)
{
    node_t ** p = phead;
    while((*p)!=NULL){
        if((*p)->nr==val){
            return p;
        }
        p=&( (*p)->next);
    }
    return p;
}
