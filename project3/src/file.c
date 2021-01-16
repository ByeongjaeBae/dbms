#include "file.h"
#include"buffermanager.h"
pagenum_t global = 0;
int fd;
void initial_page(page_t* page) {
    memset(page, 0, page_size);
}
void add_free_page(int table_id) {
    buffer* head_buf;
    page_t* free_page = (page_t*)malloc(sizeof(page_t));
    head_buf=get_frame(table_id,header_page_num);
    page_t* header_page = head_buf->page;
    global = ((header_page_t*)header_page)->Number_of_pages-1;
    put_frame(head_buf,0);
    for (int i = 0; i < 10; i++) {
        head_buf = get_frame(table_id, header_page_num);
        header_page = head_buf->page;
        initial_page(free_page);
        ((free_page_t*)free_page)->next_free_page_num = ((header_page_t*)header_page)->free_page_number;
        global++;
        file_write_page(table_id, global, free_page);
        ((header_page_t*)header_page)->free_page_number = global;
        ((header_page_t*)header_page)->Number_of_pages++;
        put_frame(head_buf, 1);
    }
    free(free_page);
}
pagenum_t file_alloc_page(int table_id) {
    buffer* free_buf;
    buffer* head_buf = get_frame(table_id, header_page_num);
    page_t* header_page = head_buf->page;
    page_t* free_page;
    int num = ((header_page_t*)header_page)->free_page_number;
    if (num == 0) {
        add_free_page(table_id);
    }
    pagenum_t pagenum = ((header_page_t*)header_page)->free_page_number;
    free_buf = get_frame(table_id, pagenum);
    free_page = free_buf->page;
    ((header_page_t*)header_page)->free_page_number = ((free_page_t*)free_page)->next_free_page_num;
    put_frame(head_buf, 1);
    put_frame(free_buf, 1);
    return pagenum;
}
int file_open(char* pathname){
	fd=open(pathname,O_CREAT|O_RDWR,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	if(fd==-1)
		return -1;
	else return fd;	
}
void file_free_page(int table_id, pagenum_t pagenum) {
    pagenum_t free_num = file_alloc_page(table_id);
    buffer* head_buf, * buf;
    page_t* page;
    buf = get_frame(table_id, pagenum);
    page = buf->page;
    initial_page(page);
    ((free_page_t*)page)->next_free_page_num = free_num;
    head_buf = get_frame(table_id, header_page_num);
    page_t* header_page = head_buf->page;
    ((header_page_t*)header_page)->free_page_number = pagenum;
    put_frame(head_buf, 1);
    put_frame(buf, 1);
}

void file_read_page(int table_id, pagenum_t pagenum, page_t* dest) {
   if( lseek(fds[table_id], (pagenum)*page_size, SEEK_SET)==-1){
	   //printf("lseek error\n");
   	   return;
   }
    read(fds[table_id],dest, page_size);
}

void file_write_page(int table_id, pagenum_t pagenum, const page_t* src) {
    lseek(fds[table_id], (pagenum)*page_size, SEEK_SET);
    write(fds[table_id],src, page_size);
    if(fsync(fds[table_id]))printf("sync fail\n");
}
int feof_file(int table_id) {
	char a[10];
	lseek(fds[table_id],0,SEEK_SET);
	if(read(fds[table_id],a,1)==0)
		return 1;
	else return 0;
}
