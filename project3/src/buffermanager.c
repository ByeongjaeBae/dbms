#include "file.h"
#include "buffermanager.h"
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
        buf->is_pinned = 0;
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
    return 0;
}
int close_table(int table_id) {
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
            buf->is_pinned = 0;
            buf->page_num = 0;
            buf->table_id = 0;
        }
        buf = buf->next;
    }
    if(close(fds[table_id])){
       fds[table_id]=-1;
       //printf("close error\n");
       return -1;
    }
    fds[table_id]=-1;
    return 0;
}
int shutdown_db() {
    if(head==NULL){//printf("head is null\n"); 
	return -1;}
    buffer* buf = head->next;
    while (buf != head) {
        if (buf == NULL)return -1;
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
    return 0;
}
buffer* get_frame(int table_id, pagenum_t page) {
    buffer* search = head->next;
    while (search != head) {
        if (search->table_id == table_id && search->page_num == page) {
            search->is_pinned = 1;
            return search;
        }
        search = search->next;
    }
    buffer* unpinned = head->prev;
    while (unpinned != head) {
        if (unpinned->is_pinned == 0) {
            break;
        }
        unpinned = unpinned->prev;
    }
    if (unpinned == head) {
        printf("frame is all pinned\n");
        return NULL;
    }
    write_buf(unpinned);
    unpinned->table_id = table_id;
    unpinned->page_num = page;
    file_read_page(table_id, page, unpinned->page);
    unpinned->is_pinned = 1;
    return unpinned;
}
void write_buf(buffer* frame) {
    //printf("write_buf\n");
    if (frame == NULL)return;
    frame->is_pinned = 1;
    if (frame->is_dirty) {
        page_t* page = frame->page;
        file_write_page(frame->table_id, frame->page_num, page);
        frame->is_dirty = 0;
    }
    frame->is_pinned = 0;
}
void put_frame(buffer* frame, int is_dirty) {
    //printf("put_frame\n");
    if (frame->is_dirty != 1)frame->is_dirty = is_dirty;
    if (head->next == frame) {
        frame->is_pinned = 0;
        return;
    }
    frame->prev->next = frame->next;
    frame->next->prev = frame->prev;
    frame->prev = head;
    frame->next = head->next;
    frame->prev->next = frame;
    frame->next->prev = frame;
    if (frame->is_pinned)frame->is_pinned = 0;
    else printf("unpinned error\n");
}
