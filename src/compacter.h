/*reads a TLV and write it at the position offset_write, if needed*/
tlv_t * readtlv_compacter(int fd,unsigned long * p_offset_length);

/*main loop for compacter*/
int loop_compacter(int fd, off_t left, off_t right, int lvl, tlv_t * parent);
