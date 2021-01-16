#include "file.h"
#include "buffermanager.h"
buffer* head;
pthread_mutex_t buffer_latch;
int init_db(int num_buf) {
    if (num_buf <= 0){
	printf("initialized faile\n");
	return -1;
}
    buffer* buf;
    for (int i = 0; i < num_buf; i++) {
        buf = (buffer*)malloc(sizeof(buffer));
        if (buf == NULL) {
            printf("max memory\n");
            exit(-1);
        }
        buf->page = (page_t*)malloc(sizeof(page_t));
        buf->page_latch=PTHREAD_MUTEX_INITIALIZER;
        buf->is_dirty = 0;
        buf->page_num = 0;
        buf->table_id = 0;
        if (head == NULL) {
            head = (buffer*)malloc(sizeof(buffer));
            head->next = buf;
            buf->next = head;
            buf->prev = head;
            head->prev = buf;
        }
        else {
            buf->next = head->next;
            head->next = buf;
            buf->prev = head;
            buf->next->prev = buf;
        }
    }
    buffer_latch=PTHREAD_MUTEX_INITIALIZER;
    return 0;
}
int close_table(int table_id) {
    pthread_mutex_lock(&buffer_latch);
    buffer* buf = head->next;
    while (buf != head) {
        if (buf->table_id == table_id) {
            if (buf == NULL)return -1;
            /*if (buf->is_pinned) {
                printf("close pinned error\n");
                printf("page:%d\n", buf->page_num);
            }*/
            write_buf(buf);
            buf->is_dirty = 0;
            buf->page_num = 0;
            buf->table_id = 0;
        }
        buf = buf->next;
    }
    if(close(fds[table_id])){
       fds[table_id]=-1;
       //printf("close error\n");
       pthread_mutex_unlock(&buffer_latch);
       return -1;
    }
    fds[table_id]=-1;
    pthread_mutex_unlock(&buffer_latch);
    return 0;
}
int shutdown_db() {
    pthread_mutex_lock(&buffer_latch);
    if(head==NULL){//printf("head is null\n"); 
	return -1;}
    buffer* buf = head->next;
    while (buf != head) {
        if (buf == NULL){
            pthread_mutex_unlock(&buffer_latch);
            return -1;
            }
        //if (buf->is_pinned)printf("close pinned error");
        if (buf->is_dirty)write_buf(buf);
        buf = buf->next;
        if (buf->prev != NULL)free(buf->prev);
    }
    free(head);
    head = NULL;
    for (int i = 1; i <= 10; i++) {
        if (fds[i] != -1) {
            close(fds[i]);
	    fds[i]=-1;
        }
    }
    pthread_mutex_unlock(&buffer_latch);
    return 0;
}
buffer* get_frame(int table_id, pagenum_t page) {
   // printf("get_frame1 %d\n",page);
    pthread_mutex_lock(&buffer_latch);
   // printf("get_frame2 %d\n",page);
    buffer* search = head->next;
    while (search != head) {
        if (search->table_id == table_id && search->page_num == page) {
            //printf("get_frame_2_ %d\n",page);
            pthread_mutex_lock(&search->page_latch);
                search->prev->next = search->next;
                search->next->prev = search->prev;
                search->prev = head;
                search->next = head->next;
                search->prev->next = search;
                search->next->prev = search;
           // printf("get_frame_a2_ %d\n",page);
            pthread_mutex_unlock(&buffer_latch);
           // printf("get_frame_b2_ %d\n",page);
            return search;
        }
        search = search->next;
    }
    //printf("get_frame3\n");
    buffer* unpinned = head->prev;
    while (unpinned != head) {
        if (pthread_mutex_trylock(&unpinned->page_latch)==0) {
            pthread_mutex_unlock(&unpinned->page_latch);
            break;
        }
        unpinned = unpinned->prev;
    }
   // printf("get_frame4\n");
    if (unpinned == head) {
        printf("frame is all pinned\n");
        pthread_mutex_unlock(&buffer_latch);
        return NULL;
    }
    //printf("get_frame5\n");
    write_buf(unpinned);
    unpinned->table_id = table_id;
    unpinned->page_num = page;
    file_read_page(table_id, page, unpinned->page);
    //printf("get_frame6 %d\n",page);
    pthread_mutex_lock(&unpinned->page_latch);
    unpinned->prev->next =unpinned->next;
    unpinned->next->prev = unpinned->prev;
    unpinned->prev = head;
    unpinned->next = head->next;
    unpinned->prev->next = unpinned;
    unpinned->next->prev = unpinned;
    pthread_mutex_unlock(&buffer_latch);
    //printf("get_frame7 %d\n",page);
    return unpinned;
}
void write_buf(buffer* frame) {
    //printf("write_buf1 %d\n",frame->page_num);
    if (frame == NULL)return;
   // pthread_mutex_lock(&frame->page_latch);
   // printf("write_buf2 %d\n",frame->page_num);
    if (frame->is_dirty) {
        page_t* page = frame->page;
        file_write_page(frame->table_id, frame->page_num, page);
        frame->is_dirty = 0;
    }
    pthread_mutex_unlock(&frame->page_latch);
    //printf("write_bu3 %d\n",frame->page_num);
}
void put_frame(buffer* frame, int is_dirty) {
    //printf("put_frame1 %d\n",frame->page_num);
    if (frame->is_dirty != 1)frame->is_dirty = is_dirty;
    if (head->next == frame) {
        pthread_mutex_unlock(&frame->page_latch);
        return;
    }
    pthread_mutex_unlock(&frame->page_latch);
    /*if (frame->is_pinned)frame->is_pinned = 0;
    else printf("unpinned error\n");*/
}
