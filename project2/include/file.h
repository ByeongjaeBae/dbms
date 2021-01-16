#ifndef __file_H__
#define __file_H__
#include <stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include <stdint.h> 
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>
typedef uint64_t pagenum_t;
typedef uint64_t element;
#define page_size 4096
#define header_page_num 0

extern pagenum_t global;
extern int fd;
typedef struct record {
    element key;
    char value[120];
}record_t;

typedef struct nums {
    element key;
    pagenum_t page_number;
}nums;


typedef struct page_t {
	char bytes[4096];
}page_t;
typedef struct node {
    pagenum_t page;
    struct node* next; // Used for queue.
} node;

page_t* header_page;

void initial_page(page_t* page);

void add_free_page();

pagenum_t file_alloc_page();

void file_free_page(pagenum_t pagenum);

void file_read_page(pagenum_t pagenum, page_t* dest);
int check_id(int tid);
void file_write_page(pagenum_t pagenum, const page_t* src);
#endif
