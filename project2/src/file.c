#include "file.h"
#include "page.h"
pagenum_t global = 0;
int fd;
void initial_page(page_t* page) {
    memset(page, 0, page_size);
}
void add_free_page() {
    file_read_page(header_page_num, header_page);
    page_t* free_page = (page_t*)malloc(sizeof(page_t));
    for (int i = 0; i < 10; i++) {
        initial_page(free_page);
        ((free_page_t*)free_page)->next_free_page_num = ((header_page_t*)header_page)->free_page_number;
        global++;
        file_write_page(global, free_page);
        ((header_page_t*)header_page)->free_page_number = global;
        ((header_page_t*)header_page)->Number_of_pages++;
        file_write_page(header_page_num, header_page);
    }
    free(free_page);
}
pagenum_t file_alloc_page() {
    page_t* free_page = (page_t*)malloc(sizeof(page_t));
    file_read_page(header_page_num, header_page);
    int num = ((header_page_t*)header_page)->free_page_number;
    if (num == 0) {
        add_free_page();
    }
    pagenum_t pagenum = ((header_page_t*)header_page)->free_page_number;
    file_read_page(pagenum, free_page);
    ((header_page_t*)header_page)->free_page_number = ((free_page_t*)free_page)->next_free_page_num;
    file_write_page(header_page_num, header_page);
    return pagenum;
}
int file_open(char* pathname){
	fd=open(pathname,O_CREAT|O_RDWR,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	if(fd==-1)
		return -1;
	else return fd;	
}
void file_free_page(pagenum_t pagenum) {
    pagenum_t free_num = file_alloc_page();
    page_t* page = (page_t*)malloc(sizeof(page_t));
    file_read_page(pagenum, page);
    initial_page(page);
    ((free_page_t*)page)->next_free_page_num = free_num;
    file_read_page(header_page_num, header_page);
    ((header_page_t*)header_page)->free_page_number = pagenum;
    file_write_page(header_page_num, header_page);
    file_write_page(pagenum, page);
    free(page);
}

void file_read_page(pagenum_t pagenum, page_t* dest) {
   if( lseek(fd, (pagenum)*page_size, SEEK_SET)==-1){
	   printf("lseek error\n");
   	   return;
   }
    read(fd,dest, page_size);
}

void file_write_page(pagenum_t pagenum, const page_t* src) {
    lseek(fd, (pagenum)*page_size, SEEK_SET);
    write(fd, src, page_size);
    if(fsync(fd))printf("sync fail\n");
}
int feof_file(){
	char a[10];
	lseek(fd,0,SEEK_SET);
	if(read(fd,a,1)==0)
		return 1;
	else return 0;
}
