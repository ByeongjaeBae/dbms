#ifndef __file_H__
#define __file_H__
#include <stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include <stdint.h> 
#include<stdlib.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>
typedef uint64_t pagenum_t;
typedef uint64_t element;
#define page_size 4096
#define header_page_num 0

extern pagenum_t global;
extern int fds[11];
extern int fd;
typedef struct record {
    element key;
    char value[120];
}record_t;

typedef struct nums {
    element key;
    pagenum_t page_number;
}nums;

typedef struct page_t1 {
    pagenum_t free_page_number;
    pagenum_t root_page_number;
    element Number_of_pages;
    char page[4072];
}header_page_t;
typedef struct page_t2 {
    pagenum_t Parent_page_number;
    int is_Leaf;
    int Number_of_Keys;
    char page[104];
    pagenum_t sibling_page_number;
    record_t record[31];
}leaf_page_t;
typedef struct page_t3 {
    pagenum_t next_free_page_num;
    char page[4088];
}free_page_t;
typedef struct page_t4 {
    pagenum_t parent_page_number;
    int is_Leaf;
    int number_of_Keys;
    char page[104];
    pagenum_t left_page_num;
    nums arr[248];
}internal_page_t;

typedef struct page_t {
    char bytes[4096];
}page_t;

int file_open(char* pathname);
void initial_page(page_t* page);

void add_free_page(int table_id);

pagenum_t file_alloc_page(int table_id);

void file_free_page(int table_id,pagenum_t pagenum);

void file_read_page(int table_id,pagenum_t pagenum, page_t* dest);
int feof_file(int table_id);
void file_write_page(int table_id,pagenum_t pagenum, const page_t* src);


#endif
