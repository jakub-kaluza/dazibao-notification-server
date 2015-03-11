#ifndef DELETER_H
#define DELETER_H


void subloop(int fd, off_t left, off_t right, int lvl);

/*main loop for removal. We explore the file, in order to find TLVs we want to remove. Its functioning is similar to the function 'loop' used in reader, but now we can choose some actions, like switching TLVs, removing them or quit the program*/
void loop_deleter(int fd, off_t left, off_t right, int lvl, tlv_t * parent);

/*displays all possible actions, reads the input from user, checks if it's correct and then performs the chosen action.*/
void get_action(int fd, off_t left, int lvl, tlv_t * tlv, int * j);

#endif
