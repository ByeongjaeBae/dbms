#ifndef __BPT_H__
#define __BPT_H__

#define _CRT_SECURE_NO_WARNINGS
#include "file.h"
#include "buffermanager.h"
#include <stdint.h> 
#include<string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdint.h> 
#include <string.h>
#include <assert.h>
#ifdef WINDOWS
#define int char
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

extern char table[11][20];
typedef struct node {
    pagenum_t page;
    struct node* next; // Used for queue.
} node;
extern int table_id;
int db_insert(int table_id, element key, char* ret_val);
int db_find(int table_id, element key, char* ret_val,int trx_id);
int db_update(int table_id,element key,char* values,int trx_id);
int open_table(char* pathname);
int db_delete(int table_id, element key);
void enqueue(pagenum_t page);
node* dequeue(void);
int path_to_root(int table_id, pagenum_t child);
void print_tree(int table_id);
pagenum_t find_leaf_trx(int table_id, element key,int trx_id);
record_t* find_tr(int table_id, element key,int trx_id,int lock_mode);
record_t* find_update_trx(int table_id, element key,char* values,int trx_id,int lock_mode);
pagenum_t find_leaf(int table_id, element key);
record_t* find(int table_id, element key);
int cut(int length);

pagenum_t root_number(page_t* header_page);
void set_root_number(int table_id, pagenum_t pg);
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

pagenum_t  make_page(int table_id);
pagenum_t  make_leaf(int table_id);
int get_left_index(int table_id, pagenum_t parent, pagenum_t left);

pagenum_t  insert_into_leaf(int table_id, pagenum_t leaf, record_t* pointer);
pagenum_t insert_into_leaf_after_splitting(int table_id, pagenum_t leaf, record_t* pointer);
pagenum_t  insert_into_node(int table_id, pagenum_t parent,
    int left_index, element key, pagenum_t right);
pagenum_t insert_into_node_after_splitting(int table_id, pagenum_t parent, int left_index, element key, pagenum_t right);
pagenum_t insert_into_parent(int table_id, pagenum_t left, element key, pagenum_t right);
pagenum_t insert_into_new_root(int table_id, pagenum_t left, element key, pagenum_t right);
pagenum_t start_new_tree(int table_id, record_t* pointer);
pagenum_t insert(int table_id, element key, char* value);

// Deletion.

pagenum_t remove_entry_from_leaf(int table_id, pagenum_t n, element key, record_t* pointer);
pagenum_t remove_entry_from_node(int table_id, pagenum_t n, element key, pagenum_t p);
int get_neighbor_index(int table_id, pagenum_t n);
pagenum_t adjust_root(int table_id, pagenum_t root);
pagenum_t coalesce_nodes(int table_id, pagenum_t n, pagenum_t neighbor, int neighbor_index, element k_prime);
pagenum_t redistribute_nodes(int table_id, pagenum_t n, pagenum_t neighbor, int neighbor_index, int k_prime_index, element k_prime);
pagenum_t delete_entry(int table_id, pagenum_t n, element key, record_t* pointer, pagenum_t p);
int buf_delete(int table_id, element key);
extern node* queue;
#endif
