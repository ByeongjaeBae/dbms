#ifndef __BPT_H__
#define __BPT_H__

#include "file.h"
#include <stdint.h> 
#include<string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h> 
#ifdef WINDOWS
#define bool char
#define false 0
#define true 1
#endif
// Minimum order is necessarily 3.  We set the maximum
// order arbitrarily.  You may change the maximum order.
#define internal_order 249
#define leaf_order 32
// Constants for printing part or all of the GPL license.
#define LICENSE_FILE "LICENSE.txt"
#define LICENSE_WARRANTEE 0
#define LICENSE_WARRANTEE_START 592
#define LICENSE_WARRANTEE_END 624
#define LICENSE_CONDITIONS 1
#define LICENSE_CONDITIONS_START 70
#define LICENSE_CONDITIONS_END 625

char table[10000][10000];
extern int table_id;


int db_insert(element key, char* ret_val);
int db_find(element key, char* ret_val);
int open_table(char* pathname);
int db_delete(element key);
void license_notice(void);
void print_license(int licence_part);
void usage_1(void);
void usage_2(void);
void usage_3(void);
void enqueue(pagenum_t page);
node* dequeue(void);
int height();
int path_to_root(pagenum_t child);
void print_leaves();
void print_tree();
pagenum_t find_leaf(element key);
record_t* find(element key);
int cut(int length);

pagenum_t root_number();
void set_root_number(pagenum_t pg);
int get_num_keys(page_t* page);
void set_num_keys(page_t* page, int num);
element get_key(page_t* page, int i);
void set_key(page_t* page, int i, element key_);
record_t* make_record(element key, char* value);
void get_value(page_t* page, int i, char* buf);
void set_value(page_t* page, int i, char* buf);
pagenum_t get_parent_page(page_t* page);
void set_parent_page(page_t* page, pagenum_t pg);
void set_key(page_t* page, int i, element key_);
pagenum_t get_pagenum(page_t* page, int i);

// Insertion.

pagenum_t  make_page(void);
pagenum_t  make_leaf(void);
int get_left_index(pagenum_t parent, pagenum_t left);

pagenum_t  insert_into_leaf(pagenum_t leaf, record_t* pointer);
pagenum_t insert_into_leaf_after_splitting(pagenum_t leaf, record_t* pointer);
pagenum_t  insert_into_node(pagenum_t parent,
    int left_index, element key, pagenum_t right);
pagenum_t insert_into_node_after_splitting(pagenum_t parent, int left_index, element key, pagenum_t right);
pagenum_t insert_into_parent(pagenum_t left, element key, pagenum_t right);
pagenum_t insert_into_new_root(pagenum_t left, element key, pagenum_t right);
pagenum_t start_new_tree(record_t* pointer);
pagenum_t insert(element key, char* value);

// Deletion.

pagenum_t remove_entry_from_leaf(pagenum_t n, element key, record_t* pointer);
pagenum_t remove_entry_from_node(pagenum_t n, element key, pagenum_t p);
int get_neighbor_index(pagenum_t n);
pagenum_t adjust_root(pagenum_t root);
pagenum_t coalesce_nodes(pagenum_t n, pagenum_t neighbor, int neighbor_index, element k_prime);
pagenum_t delete_entry(pagenum_t n, element key, record_t* pointer, pagenum_t p);
int delete(element key);
pagenum_t redistribute_nodes(pagenum_t n,pagenum_t neighbor, int neighbor_index, int k_prime_index, element k_prime);
extern node* queue;
#endif
