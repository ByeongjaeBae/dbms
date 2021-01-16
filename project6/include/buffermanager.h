#ifndef __buffer_H__
#define __buffer_H__
#include<pthread.h>
typedef struct buffer {
    page_t* page;
    int table_id;
    pagenum_t page_num;
    int is_dirty;
    pthread_mutex_t page_latch;
    struct buffer* next;
    struct buffer* prev;
}buffer;
extern buffer* head;
extern pthread_mutex_t buffer_latch;
int init_db(int num_buf);
int close_table(int table_id);
int shutdown_db();
buffer* get_frame(int table_id, pagenum_t page);
void put_frame(buffer* frame, int is_dirty);
void write_buf(buffer* frame);
#endif