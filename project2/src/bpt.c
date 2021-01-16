#include "bpt.h"
#include "page.h"
/*
 *  bpt.c
 */
#define Version "1.14"
 /*
  *
  *  bpt:  B+ Tree Implementation
  *  Copyright (C) 2010-2016  Amittai Aviram  http://www.amittai.com
  *  All rights reserved.
  *  Redistribution and use in source and binary forms, with or without
  *  modification, are permitted provided that the following conditions are met:
  *
  *  1. Redistributions of source code must retain the above copyright notice,
  *  this list of conditions and the following disclaimer.
  *
  *  2. Redistributions in binary form must reproduce the above copyright notice,
  *  this list of conditions and the following disclaimer in the documentation
  *  and/or other materials provided with the distribution.

  *  3. Neither the name of the copyright holder nor the names of its
  *  contributors may be used to endorse or promote products derived from this
  *  software without specific prior written permission.

  *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  *  POSSIBILITY OF SUCH DAMAGE.

  *  Author:  Amittai Aviram
  *    http://www.amittai.com
  *    amittai.aviram@gmail.edu or afa13@columbia.edu
  *  Original Date:  26 June 2010
  *  Last modified: 17 June 2016
  *
  *  This implementation demonstrates the B+ tree data structure
  *  for educational purposes, includin insertion, deletion, search, and display
  *  of the search path, the leaves, or the whole tree.
  *
  *  Must be compiled with a C99-compliant C compiler such as the latest GCC.
  *
  *  Usage:  bpt [order]
  *  where order is an optional argument
  *  (integer MIN_ORDER <= order <= MAX_ORDER)
  *  defined as the maximal number of pointers in any node.
  *
  */
int table_id = 1;

int db_insert(element key, char* value) {
    if (insert(key, value))return 0;
    else return -1;
}
int db_find(element key, char* ret_val) {
    record_t* record;
    file_read_page(header_page_num, header_page);
    if ((record = find(key)) == NULL) {
        return -1;
    }
    strncpy(ret_val, record->value, 120);
    return 0;
}
int db_delete(element key) {
    if (delete(key) >= 0)return 0;
    else return -1;
}

int open_table(char* pathname) {
    if (pathname == NULL) {
        printf("invalid path\n");
    }
    header_page = (page_t*)malloc(sizeof(page_t));
    int fp = file_open(pathname);
    if (fp == -1)return -1;
    for (int i = 0; i < table_id; i++) {
        if (strncmp(table[i], pathname, 120) == 0)
            return i;
    }
    strcpy(table[table_id], pathname);
    file_read_page(header_page_num, header_page);
    if (feof_file()) {
        initial_page(header_page);
	global=0;
	((header_page_t*)header_page)->Number_of_pages=1;
        file_write_page(0, header_page);
        return table_id++;
    }
    global = ((header_page_t*)header_page)->Number_of_pages;
    return table_id++;
}
// GLOBALS.

/* The order determines the maximum and minimum
 * number of entries (keys and pointers) in any
 * node.  Every node has at most order - 1 keys and
 * at least (roughly speaking) half that number.
 * Every leaf has as many pointers to data as keys,
 * and every internal node has one more pointer
 * to a subtree than the number of keys.
 * This global variable is initialized to the
 * default value.
 */

 /* The queue is used to print the tree in
  * level order, starting from the root
  * printing each entire rank on a separate
  * line, finishing with the leaves.
  */
node* queue = NULL;

/* The user can toggle on and off the "verbose"
 * property, which causes the pointer addresses
 * to be printed out in hexadecimal notation
 * next to their corresponding keys.
 */

 // FUNCTION DEFINITIONS.

 // OUTPUT AND UTILITIES

 /* Copyright and license notice to user at startup.
  */
void license_notice(void) {
    printf("bpt version %s -- Copyright (C) 2010  Amittai Aviram "
        "http://www.amittai.com\n", Version);
    printf("This program comes with ABSOLUTELY NO WARRANTY; for details "
        "type `show w'.\n"
        "This is free software, and you are welcome to redistribute it\n"
        "under certain conditions; type `show c' for details.\n\n");
}


/* Routine to print portion of GPL license to stdout.
 */
void print_license(int license_part) {
    int start, end, line;
    FILE* fp;
    char buffer[0x100];

    switch (license_part) {
    case LICENSE_WARRANTEE:
        start = LICENSE_WARRANTEE_START;
        end = LICENSE_WARRANTEE_END;
        break;
    case LICENSE_CONDITIONS:
        start = LICENSE_CONDITIONS_START;
        end = LICENSE_CONDITIONS_END;
        break;
    default:
        return;
    }

    fp = fopen(LICENSE_FILE, "r");
    if (fp == NULL) {
        perror("print_license: fopen");
        exit(EXIT_FAILURE);
    }
    for (line = 0; line < start; line++)
        fgets(buffer, sizeof(buffer), fp);
    for (; line < end; line++) {
        fgets(buffer, sizeof(buffer), fp);
        printf("%s", buffer);
    }
    fclose(fp);
}


/* First message to the user.
 */
void usage_1(void) {
    printf("B+ Tree of Order %d.\n", leaf_order);
    printf("Following Silberschatz, Korth, Sidarshan, Database Concepts, "
        "5th ed.\n\n"
        "To build a B+ tree of a different order, start again and enter "
        "the order\n"
        "as an integer argument:  bpt <order>  ");
    printf("(%d <= order <= %d).\n", leaf_order, internal_order);
    printf("To start with input from a file of newline-delimited integers, \n"
        "start again and enter the order followed by the filename:\n"
        "bpt <order> <inputfile> .\n");
}


/* Second message to the user.
 */
void usage_2(void) {
    printf("Enter any of the following commands after the prompt > :\n"
        "\ti <k>  -- Insert <k> (an integer) as both key and value).\n"
        "\tf <k>  -- Find the value under key <k>.\n"
        "\tp <k> -- Print the path from the root to key k and its associated "
        "value.\n"
        "\tr <k1> <k2> -- Print the keys and values found in the range "
        "[<k1>, <k2>\n"
        "\td <k>  -- Delete key <k> and its associated value.\n"
        "\tx -- Destroy the whole tree.  Start again with an empty tree of the "
        "same order.\n"
        "\tt -- Print the B+ tree.\n"
        "\tl -- Print the keys of the leaves (bottom row of the tree).\n"
        "\tv -- Toggle output of pointer addresses (\"verbose\") in tree and "
        "leaves.\n"
        "\tq -- Quit. (Or use Ctl-D.)\n"
        "\t? -- Print this help message.\n");
}


/* Brief usage note.
 */
void usage_3(void) {
    printf("Usage: ./bpt [<order>]\n");
    printf("\twhere %d <= order <= %d .\n", leaf_order, internal_order);
}


/* Helper function for printing the
 * tree out.  See print_tree.
 */
void enqueue(pagenum_t page) {
    node* c;
    node* new_node = (node*)malloc(sizeof(node));
    new_node->page = page;
    new_node->next = NULL;
    if (queue == NULL) {
        queue = new_node;
        queue->next = NULL;
    }
    else {
        c = queue;
        while (c->next != NULL) {
            c = c->next;
        }
        c->next = new_node;
        new_node->next = NULL;
    }
}


/* Helper function for printing the
 * tree out.  See print_tree.
 */
node* dequeue(void) {
    node* n = queue;
    queue = queue->next;
    n->next = NULL;
    return n;
}


/* Prints the bottom row of keys
 * of the tree (with their respective
 * pointers, if the verbose_output flag is set.
 */
void print_leaves() {
    pagenum_t root = root_number();
    int i;
    page_t* root_page = (page_t*)malloc(sizeof(page_t));
    if (root == 0) {
        printf("Empty tree.\n");
        return;
    }
    file_read_page(root, root_page);
    while (!((internal_page_t*)root_page)->is_Leaf) {
        root = ((internal_page_t*)root_page)->left_page_num;
        file_read_page(root, root_page);
    }
    while (true) {
        for (i = 0; i < get_num_keys(root_page); i++) {
            printf("%lu ", get_key(root_page, i));
        }
        if (((leaf_page_t*)root_page)->sibling_page_number != 0) {
            printf(" | ");
            root = ((leaf_page_t*)root_page)->sibling_page_number;
            file_read_page(root, root_page);
        }
        else
            break;
    }
    printf("\n");
}


/* Utility function to give the height
 * of the tree, which length in number of edges
 * of the path from the root to any leaf.
 */
int height() {
    pagenum_t root = root_number();
    int h = 0;
    page_t* root_page = (page_t*)malloc(sizeof(page_t));
    file_read_page(root, root_page);
    while (!((internal_page_t*)root_page)->is_Leaf) {
        root = ((internal_page_t*)root_page)->left_page_num;
        file_read_page(root, root_page);
        h++;
    }
    free(root_page);
    return h;
}


/* Utility function to give the length in edges
 * of the path from any node to the root.
 */
int path_to_root(pagenum_t child) {
    pagenum_t root = root_number();
    int length = 0;
    page_t* c_page = (page_t*)malloc(sizeof(page_t));
    file_read_page(child, c_page);
    pagenum_t c = child;
    while (c != root) {
        c = ((internal_page_t*)c_page)->parent_page_number;
        file_read_page(c, c_page);
        length++;
    }
    free(c_page);
    return length;
}


/* Prints the B+ tree in the command
 * line in level (rank) order, with the
 * keys in each node and the '|' symbol
 * to separate nodes.
 * With the verbose_output flag set.
 * the values of the pointers corresponding
 * to the keys also appear next to their respective
 * keys, in hexadecimal notation.
 */
void print_tree() {
    pagenum_t root = root_number();
    page_t* root_page = (page_t*)malloc(sizeof(page_t));
    page_t* parent_page = (page_t*)malloc(sizeof(page_t));
    page_t* n_page = (page_t*)malloc(sizeof(page_t));
    file_read_page(root, root_page);
    node* n;
    int i = 0;
    int rank = 0;
    int new_rank = 0;
    pagenum_t parent;
    if (root == 0) {
        printf("Empty tree.\n");
        return;
    }
    queue = NULL;
    enqueue(root);
    while (queue != NULL) {
        n = dequeue();
        file_read_page(n->page, n_page);
        file_read_page(get_parent_page(n_page), parent_page);
        if (((internal_page_t*)n_page)->parent_page_number != 0
            && n->page == ((internal_page_t*)parent_page)->left_page_num) {
            new_rank = path_to_root(n->page);
            if (new_rank != rank) {
                rank = new_rank;
                printf("\n");
            }
        }
        for (i = 0; i < get_num_keys(n_page); i++) {
            if (((internal_page_t*)n_page)->is_Leaf) {
                printf("%d ", get_key(n_page, i));
            }
            else {
                printf("%d ", ((internal_page_t*)n_page)->arr[i].key);
            }
        }
        printf("page:%u", n->page);
        if (!((internal_page_t*)n_page)->is_Leaf) {
            enqueue(((internal_page_t*)n_page)->left_page_num);
            for (i = 0; i < get_num_keys(n_page); i++) {
                enqueue(((internal_page_t*)n_page)->arr[i].page_number);
            }
        }
        printf("| ");
        free(n);
    }
    printf("\n");
}


/* Finds the record under a given key and prints an
 * appropriate message to stdout.
 */
void find_and_print(element key) {
    record_t* r = find(key);
    if (r == NULL)
        printf("Record not found under key %d.\n", key);
    else
        printf("Record at %lx -- key %d, value %s.\n", r,
            (unsigned long)r->key, r->value);
}

int get_num_keys(page_t* page) {
    return ((leaf_page_t*)page)->Number_of_Keys;
}
void set_num_keys(page_t* page, int num) {
    ((leaf_page_t*)page)->Number_of_Keys = num;
}
pagenum_t root_number() {
    file_read_page(header_page_num, header_page);
    pagenum_t tmp = ((header_page_t*)header_page)->root_page_number;
    return tmp;
}
void set_root_number(pagenum_t pg) {
    file_read_page(header_page_num, header_page);
    ((header_page_t*)header_page)->root_page_number = pg;
    file_write_page(header_page_num, header_page);
}
pagenum_t find_leaf(element key) {
    int i = 0;
    pagenum_t root_num = root_number();
    pagenum_t leaf_num = root_num;
    if (root_num == 0) {
        return 0;
    }
    page_t* page = (page_t*)malloc(sizeof(page_t));
    file_read_page(root_num, page);
    while (!((leaf_page_t*)page)->is_Leaf) {
        i = 0;
        while (i < get_num_keys(page)) {
            if (key >= ((internal_page_t*)page)->arr[i].key) i++;
            else break;
        }
        if (i == 0)leaf_num = ((internal_page_t*)page)->left_page_num;
        else leaf_num = ((internal_page_t*)page)->arr[i - 1].page_number;
        file_read_page(leaf_num, page);
    }
    free(page);
    return leaf_num;
}
record_t* find(element key) {
    int i = 0;
    page_t* page = (page_t*)malloc(sizeof(page_t));
    pagenum_t leaf_num = find_leaf(key);
    file_read_page(leaf_num, page);
    if (leaf_num == 0) return NULL;
    record_t* ret = (record_t*)malloc(sizeof(record_t));
    for (i = 0; i < ((leaf_page_t*)page)->Number_of_Keys; i++)
        if (((leaf_page_t*)page)->record[i].key == key) break;
    if (i == ((leaf_page_t*)page)->Number_of_Keys) {
        free(page);
        return NULL;
    }
    else {
        ret->key = key;
        strncpy(ret->value, ((leaf_page_t*)page)->record[i].value, 120);
        free(page);
        return ret;
    }
}
pagenum_t get_parent_page(page_t* page) {
    return ((internal_page_t*)page)->parent_page_number;
}
void set_parent_page(page_t* page, pagenum_t pg) {
    ((internal_page_t*)page)->parent_page_number = pg;
}
element get_key(page_t* page, int i) {
    return (((leaf_page_t*)page)->record[i].key);
}
void set_key(page_t* page, int i, element key_) {
    ((leaf_page_t*)page)->record[i].key = key_;
}
pagenum_t get_pagenum(page_t* page, int i) {
    return ((internal_page_t*)page)->arr[i].page_number;
}

void get_value(page_t* page, int i, char* buf) {
    if (((leaf_page_t*)page)->is_Leaf == 1)
        strncpy(buf, ((leaf_page_t*)page)->record[i].value, 120);
    else {
        printf("invalid get value\n");
        return;
    }
}

void set_value(page_t* page, int i, char* buf) {
    if (((leaf_page_t*)page)->is_Leaf == 1)
        strncpy(((leaf_page_t*)page)->record[i].value, buf, 120);
    else {
        printf("invalid set value\n");
        return;
    }
}

/* Finds and returns the record to which
 * a key refers.
 */

 /* Finds the appropriate place to
  * split a node that is too big into two.
  */
int cut(int length) {
    if (length % 2 == 0)
        return length / 2;
    else
        return length / 2 + 1;
}


// INSERTION

/* Creates a new record to hold the value
 * to which a key refers.
 */
record_t* make_record(element key, char* value) {
    record_t* new_record = (record_t*)malloc(sizeof(record_t));
    if (new_record == NULL) {
        perror("Record creation.");
        exit(EXIT_FAILURE);
    }
    else {
        new_record->key = key;
        strncpy(new_record->value, value, 120);
    }
    return new_record;
}


/* Creates a new general node, which can be adapted
 * to serve as either a leaf or an internal node.
 */
pagenum_t make_page(void) {
    pagenum_t new_page_num = file_alloc_page();
    page_t* new_page = (page_t*)malloc(sizeof(page_t));
    file_read_page(new_page_num, new_page);
    ((internal_page_t*)new_page)->parent_page_number = 0;
    file_write_page(new_page_num, new_page);
    free(new_page);
    return new_page_num;
}

/* Creates a new leaf by creating a node
 * and then adapting it appropriately.
 */
pagenum_t make_leaf(void) {
    pagenum_t leaf_num = make_page();
    page_t* leaf = (page_t*)malloc(sizeof(page_t));
    file_read_page(leaf_num, leaf);
    ((leaf_page_t*)leaf)->is_Leaf = 1;
    file_write_page(leaf_num, leaf);
    free(leaf);
    return leaf_num;
}


/* Helper function used in insert_into_parent
 * to find the index of the parent's pointer to
 * the node to the left of the key to be inserted.
 */
 //

/* Master insertion function.
 * Inserts a key and an associated value into
 * the B+ tree, causing the tree to be adjusted
 * however necessary to maintain the B+ tree
 * properties.
 */
pagenum_t insert(element key, char* value) {
    file_read_page(header_page_num, header_page);
    pagenum_t root = root_number();
    /* The current implementation ignores
     * duplicates.
     */
    if (find(key) != NULL)
        return 0;

    /* Create a new record for the
     * value.
     */
    record_t* pointer = make_record(key, value);

    /* Case: the tree does not exist yet.
     * Start a new tree.
     */

    if (root == 0)
        return start_new_tree(pointer);


    /* Case: the tree already exists.
     * (Rest of function body.)
     */
    pagenum_t leaf = find_leaf(key);//
    page_t* leaf_page = (page_t*)malloc(sizeof(page_t));
    file_read_page(leaf, leaf_page);
    /* Case: leaf has room for key and pointer.
     */
    if (((leaf_page_t*)leaf_page)->Number_of_Keys < leaf_order - 1) {
        free(leaf_page);
        return insert_into_leaf(leaf, pointer);
    }


    /* Case:  leaf must be split.
     */
    free(leaf_page);

    return insert_into_leaf_after_splitting(leaf, pointer);
}

pagenum_t insert_into_leaf_after_splitting(pagenum_t leaf, record_t* pointer) {
    pagenum_t new_leaf;
    page_t* new_leaf_page = (page_t*)malloc(sizeof(page_t));
    page_t* leaf_page = (page_t*)malloc(sizeof(page_t));
    record_t* temp_record;
    element new_key;
    int insertion_index, split, i, j;

    new_leaf = make_leaf();
    file_read_page(leaf, leaf_page);
    file_read_page(new_leaf, new_leaf_page);
    temp_record = (record_t*)malloc(leaf_order * sizeof(record_t));
    if (temp_record == NULL) {
        perror("Temporary record array.");
        exit(EXIT_FAILURE);
    }

    insertion_index = 0;
    while (insertion_index < leaf_order - 1 && get_key(leaf_page, insertion_index) < pointer->key)
        insertion_index++;

    for (i = 0, j = 0; i < get_num_keys(leaf_page); i++, j++) {
        if (j == insertion_index) j++;
        temp_record[j].key = get_key(leaf_page, i);
        get_value(leaf_page, i, temp_record[j].value);
    }

    temp_record[insertion_index].key = pointer->key;
    strncpy(temp_record[insertion_index].value, pointer->value, 120);

    set_num_keys(leaf_page, 0);

    split = cut(leaf_order - 1);

    for (i = 0; i < split; i++) {
        set_value(leaf_page, i, temp_record[i].value);
        set_key(leaf_page, i, temp_record[i].key);
        set_num_keys(leaf_page, get_num_keys(leaf_page) + 1);
    }

    for (i = split, j = 0; i < leaf_order; i++, j++) {
        set_value(new_leaf_page, j, temp_record[i].value);
        set_key(new_leaf_page, j, temp_record[i].key);
        set_num_keys(new_leaf_page, get_num_keys(new_leaf_page) + 1);
    }

    free(temp_record);

    ((leaf_page_t*)new_leaf_page)->sibling_page_number = ((leaf_page_t*)leaf_page)->sibling_page_number;
    ((leaf_page_t*)leaf_page)->sibling_page_number = new_leaf;


    ((leaf_page_t*)new_leaf_page)->Parent_page_number = ((leaf_page_t*)leaf_page)->Parent_page_number;
    new_key = ((leaf_page_t*)new_leaf_page)->record[0].key;
    file_write_page(leaf, leaf_page);
    file_write_page(new_leaf, new_leaf_page);
    free(leaf_page);
    free(new_leaf_page);
    return insert_into_parent(leaf, new_key, new_leaf);
}
int get_left_index(pagenum_t parent, pagenum_t left) {
    page_t* parent_page = (page_t*)malloc(sizeof(page_t));
    file_read_page(parent, parent_page);
    int left_index = 0;
    if (((internal_page_t*)parent_page)->left_page_num == left) {
        free(parent_page);
        return -1;
    }
    while (left_index <= get_num_keys(parent_page) &&
        (((internal_page_t*)parent_page)->arr[left_index].page_number != left))
        left_index++;
    free(parent_page);
    return left_index;
}
/* Inserts a new pointer to a record and its corresponding
 * key into a leaf.
 * Returns the altered leaf.
 */
pagenum_t insert_into_leaf(pagenum_t leaf, record_t* pointer) {
    page_t* leaf_page = (page_t*)malloc(sizeof(page_t));
    char buf[120];
    file_read_page(leaf, leaf_page);
    int i, insertion_point;
    insertion_point = 0;
    int num_keys = get_num_keys(leaf_page);
    while (insertion_point < num_keys && get_key(leaf_page, insertion_point) < pointer->key)
        insertion_point++;

    for (i = num_keys; i > insertion_point; i--) {
        set_key(leaf_page, i, ((leaf_page_t*)leaf_page)->record[i - 1].key);
        set_value(leaf_page, i, ((leaf_page_t*)leaf_page)->record[i - 1].value);
    }
    set_key(leaf_page, insertion_point, pointer->key);
    set_value(leaf_page, insertion_point, pointer->value);
    set_num_keys(leaf_page, ++num_keys);
    file_write_page(leaf, leaf_page);
    free(leaf_page);
    return leaf;
}


/* Inserts a new key and pointer
 * to a new record into a leaf so as to exceed
 * the tree's order, causing the leaf to be split
 * in half.
 */


 /* Inserts a new key and pointer to a node
  * into a node into which these can fit
  * without violating the B+ tree properties.
  */
pagenum_t insert_into_node(pagenum_t parent,
    int left_index, element key, pagenum_t right) {
    int i;
    page_t* parent_page = (page_t*)malloc(sizeof(page_t));
    file_read_page(parent, parent_page);
    for (i = get_num_keys(parent_page); i > left_index + 1; i--) {
        ((internal_page_t*)parent_page)->arr[i].page_number =
            ((internal_page_t*)parent_page)->arr[i - 1].page_number;
        ((internal_page_t*)parent_page)->arr[i].key =
            ((internal_page_t*)parent_page)->arr[i - 1].key;
    }
    ((internal_page_t*)parent_page)->arr[left_index + 1].page_number = right;
    ((internal_page_t*)parent_page)->arr[left_index + 1].key = key;
    set_num_keys(parent_page, get_num_keys(parent_page) + 1);
    file_write_page(parent, parent_page);
    pagenum_t root = root_number();
    free(parent_page);
    return root;
}


/* Inserts a new key and pointer to a node
 * into a node, causing the node's size to exceed
 * the order, and causing the node to split into two.
 */
pagenum_t insert_into_node_after_splitting(pagenum_t parent, int left_index,
    element key, pagenum_t right) {

    int i, j, split;
    element k_prime;
    page_t* parent_page = (page_t*)malloc(sizeof(page_t));
    file_read_page(parent, parent_page);
    nums* temp_nums = (nums*)malloc(sizeof(nums) * (internal_order));
    page_t* new_page = (page_t*)malloc(sizeof(page_t));
    pagenum_t new = make_page();

    if (temp_nums == NULL) {
        perror("Temporary pointers array for splitting nodes.");
        exit(EXIT_FAILURE);
    }

    for (i = 0, j = 0; i < get_num_keys(parent_page); i++, j++) {
        if (j == left_index + 1) j++;
        temp_nums[j].page_number = get_pagenum(parent_page, i);
        temp_nums[j].key = ((internal_page_t*)parent_page)->arr[i].key;
    }
    temp_nums[left_index + 1].page_number = right;
    temp_nums[left_index + 1].key = key;

    /* Create the new node and copy
     * half the keys and pointers to the
     * old and half to the new.
     */
    split = cut(internal_order);
    file_read_page(new, new_page);
    set_num_keys(parent_page, 0);
    for (i = 0; i < split - 1; i++) {
        ((internal_page_t*)parent_page)->arr[i].page_number = temp_nums[i].page_number;
        ((internal_page_t*)parent_page)->arr[i].key = temp_nums[i].key;
        set_num_keys(parent_page, get_num_keys(parent_page) + 1);
    }
    ((internal_page_t*)new_page)->left_page_num = temp_nums[i].page_number;
    k_prime = temp_nums[split - 1].key;
    for (++i, j = 0; i < internal_order; i++, j++) {
        ((internal_page_t*)new_page)->arr[j].page_number = temp_nums[i].page_number;
        ((internal_page_t*)new_page)->arr[j].key = temp_nums[i].key;
        set_num_keys(new_page, get_num_keys(new_page) + 1);
    }
    free(temp_nums);
    set_parent_page(new_page, get_parent_page(parent_page));
    page_t* child_page = (page_t*)malloc(sizeof(page_t));
    pagenum_t child;
    child = ((internal_page_t*)new_page)->left_page_num;
    file_read_page(child, child_page);
    ((internal_page_t*)child_page)->parent_page_number = new;
    file_write_page(child, child_page);
    for (i = 0; i < get_num_keys(new_page); i++) {
        child = get_pagenum(new_page, i);
        file_read_page(child, child_page);
        ((internal_page_t*)child_page)->parent_page_number = new;
        file_write_page(child, child_page);
    }
    /* Insert a new key into the parent of the two
     * nodes resulting from the split, with
     * the old node to the left and the new to the right.
     */
    file_write_page(parent, parent_page);
    file_write_page(new, new_page);
    free(child_page);
    free(parent_page);
    free(new_page);
    return insert_into_parent(parent, k_prime, new);
}



/* Inserts a new node (leaf or internal node) into the B+ tree.
 * Returns the root of the tree after insertion.
 */
pagenum_t insert_into_parent(pagenum_t left, element key, pagenum_t right) {

    int left_index;
    pagenum_t parent;
    page_t* parent_page = (page_t*)malloc(sizeof(page_t));
    page_t* left_page = (page_t*)malloc(sizeof(page_t));
    file_read_page(left, left_page);
    parent = get_parent_page(left_page);
    file_read_page(parent, parent_page);
    int parent_num_key = get_num_keys(parent_page);
    free(parent_page);
    free(left_page);
    /* Case: new root. */

    if (parent == 0)
        return insert_into_new_root(left, key, right);

    /* Case: leaf or node. (Remainder of
     * function body.)
     */

     /* Find the parent's pointer to the left
      * node.
      */

    left_index = get_left_index(parent, left);


    /* Simple case: the new key fits into the node.
     */
    if (parent_num_key < internal_order - 1)
        return insert_into_node(parent, left_index, key, right);

    /* Harder case:  split a node in order
     * to preserve the B+ tree properties.
     */

    return insert_into_node_after_splitting(parent, left_index, key, right);
}


/* Creates a new root for two subtrees
 * and inserts the appropriate key into
 * the new root.
 */
pagenum_t insert_into_new_root(pagenum_t left, element key, pagenum_t right) {
    pagenum_t root = make_page();
    page_t* root_page = (page_t*)malloc(sizeof(page_t));
    page_t* left_page = (page_t*)malloc(sizeof(page_t));
    page_t* right_page = (page_t*)malloc(sizeof(page_t));
    file_read_page(left, left_page);
    file_read_page(right, right_page);
    file_read_page(root, root_page);
    ((internal_page_t*)root_page)->arr[0].key = key;
    ((internal_page_t*)root_page)->left_page_num = left;
    ((internal_page_t*)root_page)->arr[0].page_number = right;
    set_num_keys(root_page, get_num_keys(root_page) + 1);
    set_parent_page(left_page, root);
    set_parent_page(right_page, root);
    file_write_page(left, left_page);
    file_write_page(right, right_page);
    file_write_page(root, root_page);
    set_root_number(root);
    free(left_page);
    free(right_page);
    free(root_page);
    return root;
}



/* First insertion:
 * start a new tree.
 */
pagenum_t start_new_tree(record_t* pointer) {
    pagenum_t leaf = make_leaf();
    page_t* root = (page_t*)malloc(sizeof(page_t));
    file_read_page(leaf, root);
    ((leaf_page_t*)root)->record[0].key = pointer->key;
    strncpy(((leaf_page_t*)root)->record[0].value, pointer->value, 120);
    ((leaf_page_t*)root)->Number_of_Keys++;
    ((leaf_page_t*)root)->Parent_page_number = 0;
    set_root_number(leaf);
    file_write_page(leaf, root);
    free(root);
    return leaf;
}


// DELETION.

/* Utility function for deletion.  Retrieves
 * the index of a node's nearest neighbor (sibling)
 * to the left if one exists.  If not (the node
 * is the leftmost child), returns -1 to signify
 * this special case.
 */

int get_neighbor_index(pagenum_t n) {
    page_t* n_page = (page_t*)malloc(sizeof(page_t));
    int i;
    file_read_page(n, n_page);
    pagenum_t parent = get_parent_page(n_page);
    free(n_page);
    page_t* parent_page = (page_t*)malloc(sizeof(page_t));
    file_read_page(parent, parent_page);
    /* Return the index of the key to the left
     * of the pointer in the parent pointing
     * to n.
     * If n is the leftmost child, this means
     * return -1.
     */
    if (n == ((internal_page_t*)parent_page)->left_page_num) {
        free(parent_page);
        return -1;
    }
    for (i = 0; i < get_num_keys(parent_page); i++)//1
        if (((internal_page_t*)parent_page)->arr[i].page_number == n) {
            free(parent_page);
            return i; 
        }

    // Error state.
    printf("Search for nonexistent pointer to node in parent.\n");
    printf("Node:  %#lx\n", (unsigned long)n);
    exit(EXIT_FAILURE);
}
pagenum_t remove_entry_from_node(pagenum_t n, element key, pagenum_t p) {

    int i, num_values;

    // Remove the key and shift other keys accordingly.
    i = 0;
    page_t* n_page = (page_t*)malloc(sizeof(page_t));
    file_read_page(n, n_page);
    while (((internal_page_t*)n_page)->arr[i].key != key)
        i++;
    for (++i; i < get_num_keys(n_page); i++)
        ((internal_page_t*)n_page)->arr[i - 1].key = ((internal_page_t*)n_page)->arr[i].key;

    num_values = get_num_keys(n_page);
    i = 0;
    while (get_pagenum(n_page, i) != p) {
        i++;
        if (((internal_page_t*)n_page)->left_page_num == p) {
            i = -1;
            break;
        }
    }
    for (i; i < num_values; i++) {
        if (i == -1) {
            ((internal_page_t*)n_page)->left_page_num
                = ((internal_page_t*)n_page)->arr[i + 1].page_number;
        }
        else {
            ((internal_page_t*)n_page)->arr[i].page_number
                = ((internal_page_t*)n_page)->arr[i + 1].page_number;
        }
    }

    // One key fewer.
    set_num_keys(n_page, get_num_keys(n_page) - 1);
    file_write_page(n, n_page);
    free(n_page);
    return n;
}

pagenum_t remove_entry_from_leaf(pagenum_t n, element key, record_t* pointer) {

    int i, num_values;

    // Remove the key and shift other keys accordingly.
    i = 0;
    page_t* n_page = (page_t*)malloc(sizeof(page_t));
    file_read_page(n, n_page);
    while (get_key(n_page, i) != key)
        i++;
    for (++i; i < get_num_keys(n_page); i++){
        set_key(n_page, i - 1, get_key(n_page, i));
	        strncpy(((leaf_page_t*)n_page)->record[i - 1].value,
            ((leaf_page_t*)n_page)->record[i].value, 120);
}


    // One key fewer.
    set_num_keys(n_page, get_num_keys(n_page) - 1);
    file_write_page(n, n_page);
    free(n_page);
    return n;
}


pagenum_t adjust_root(pagenum_t root) {

    pagenum_t new_root;
    page_t* new_root_page = (page_t*)malloc(sizeof(page_t));
    page_t* root_page = (page_t*)malloc(sizeof(page_t));
    file_read_page(root, root_page);

    /* Case: nonempty root.
     * Key and pointer have already been deleted,
     * so nothing to be done.
     */

    if (get_num_keys(root_page) > 0) {
        free(new_root_page);
        free(root_page);
        return root;
    }

    /* Case: empty root.
     */

     // If it has a child, promote 
     // the first (only) child
     // as the new root.

    if (!((leaf_page_t*)root_page)->is_Leaf) {
        new_root = ((internal_page_t*)root_page)->left_page_num;
        file_read_page(new_root, new_root_page);
        ((internal_page_t*)new_root_page)->parent_page_number = 0;
        file_write_page(new_root, new_root_page);
        set_root_number(new_root);
    }

    // If it is a leaf (has no children),
    // then the whole tree is empty.

    else
        new_root = 0;
    file_free_page(root);
    set_root_number(new_root);
    free(root_page);
    free(new_root_page);
    return new_root;
}


/* Coalesces a node that has become
 * too small after deletion
 * with a neighboring node that
 * can accept the additional entries
 * without exceeding the maximum.
 */



 /* Deletes an entry from the B+ tree.
  * Removes the record and its key and pointer
  * from the leaf, and then makes all appropriate
  * changes to preserve the B+ tree properties.
  */
pagenum_t delete_entry(pagenum_t n, element key, record_t* pointer, pagenum_t pg) {
    page_t* n_page = (page_t*)malloc(sizeof(page_t));
    int neighbor_index;
    element k_prime;
    pagenum_t neighbor;
    pagenum_t root = root_number();
    file_read_page(n, n_page);
    // Remove key and pointer from node.
    if (((leaf_page_t*)n_page)->is_Leaf)
        n = remove_entry_from_leaf(n, key, pointer);
    else
        n = remove_entry_from_node(n, key, pg);

    /* Case:  deletion from the root.
     */

    if (n == root) {
        free(n_page);
        return adjust_root(root);
    }

    file_read_page(n, n_page);
    int num_keys = get_num_keys(n_page);

    /* Case:  node falls below minimum.
     * Either coalescence or redistribution
     * is needed.
     */

     /* Find the appropriate neighbor node with which
      * to coalesce.
      * Also find the key (k_prime) in the parent
      * between the pointer to node n and the pointer
      * to the neighbor.
      */
    pagenum_t parent = get_parent_page(n_page);
    page_t* parent_page = (page_t*)malloc(sizeof(page_t));
    file_read_page(parent, parent_page);
    neighbor_index = get_neighbor_index(n);
    if (neighbor_index == -1)k_prime = ((internal_page_t*)parent_page)->arr[0].key;
    else k_prime = ((internal_page_t*)parent_page)->arr[neighbor_index].key;
    int k_prime_index = neighbor_index == -1 ? 0 : neighbor_index;
    if (neighbor_index == 0) {
        neighbor = ((internal_page_t*)parent_page)->left_page_num;
    }
    else neighbor = neighbor_index == -1 ? ((internal_page_t*)parent_page)->arr[0].page_number :
        ((internal_page_t*)parent_page)->arr[neighbor_index - 1].page_number;
    page_t* neighbor_page = (page_t*)malloc(sizeof(page_t));
    file_read_page(neighbor, neighbor_page);
    if (num_keys <= 0 && (!((internal_page_t*)n_page)->is_Leaf) && get_num_keys(neighbor_page) == (internal_order - 1))
        redistribute_nodes(n, neighbor, neighbor_index, k_prime_index, k_prime);
    else if (num_keys <= 0) {
        coalesce_nodes(n, neighbor, neighbor_index, k_prime);
        free(n_page);
        free(parent_page);
        return root;
    }
    free(n_page);
    free(parent_page);
    return root;
}

pagenum_t coalesce_nodes(pagenum_t n, pagenum_t neighbor, int neighbor_index, element k_prime) {

    int i, j, neighbor_insertion_index;
    pagenum_t tmp, root = root_number();
    page_t* neighbor_page = (page_t*)malloc(sizeof(page_t));
    page_t* n_page = (page_t*)malloc(sizeof(page_t));
    page_t* tmp_page = (page_t*)malloc(sizeof(page_t));
    file_read_page(n, n_page);
    file_read_page(neighbor, neighbor_page);
    pagenum_t parent = get_parent_page(n_page);
    if (neighbor_index != -1) {
        neighbor_insertion_index = get_num_keys(neighbor_page);

        if (!((internal_page_t*)neighbor_page)->is_Leaf) {
            ((internal_page_t*)neighbor_page)->arr[neighbor_insertion_index].key = k_prime;
            ((internal_page_t*)neighbor_page)->arr[neighbor_insertion_index].page_number
                = ((internal_page_t*)n_page)->left_page_num;
            set_num_keys(neighbor_page, get_num_keys(neighbor_page) + 1);
            for (i = ++neighbor_insertion_index, j = 0; j < get_num_keys(n_page); i++, j++) {
                ((internal_page_t*)neighbor_page)->arr[i] = ((internal_page_t*)n_page)->arr[j];
                set_num_keys(neighbor_page, get_num_keys(neighbor_page) + 1);
            }
            tmp = ((internal_page_t*)neighbor_page)->left_page_num;
            file_read_page(tmp, tmp_page);
            ((internal_page_t*)tmp_page)->parent_page_number = neighbor;
            file_write_page(tmp, tmp_page);
            for (i = 0; i < get_num_keys(neighbor_page); i++) {
                tmp = ((internal_page_t*)neighbor_page)->arr[i].page_number;
                file_read_page(tmp, tmp_page);
                ((internal_page_t*)tmp_page)->parent_page_number = neighbor;
                file_write_page(tmp, tmp_page);
            }
            file_write_page(neighbor, neighbor_page);
        }



        else {
            ((leaf_page_t*)neighbor_page)->sibling_page_number = ((leaf_page_t*)n_page)->sibling_page_number;
            file_write_page(neighbor, neighbor_page);
        }
        delete_entry(parent, k_prime, NULL, n);
        free(tmp_page);
        file_free_page(n);
        free(n_page);
        free(neighbor_page);
    }
    else {
        neighbor_insertion_index = 0;

        if (!((internal_page_t*)n_page)->is_Leaf) {
            ((internal_page_t*)n_page)->arr[neighbor_insertion_index].key = k_prime;
            ((internal_page_t*)n_page)->arr[neighbor_insertion_index].page_number =
                ((internal_page_t*)neighbor_page)->left_page_num;
            set_num_keys(n_page, get_num_keys(n_page) + 1);
            for (i = ++neighbor_insertion_index, j = 0; j < get_num_keys(neighbor_page); i++, j++) {
                ((internal_page_t*)n_page)->arr[i] = ((internal_page_t*)neighbor_page)->arr[j];
                set_num_keys(n_page, get_num_keys(n_page) + 1);
            }
            tmp = ((internal_page_t*)n_page)->left_page_num;
            file_read_page(tmp, tmp_page);
            ((internal_page_t*)tmp_page)->parent_page_number = n;
            file_write_page(tmp, tmp_page);
            for (i = 0; i < get_num_keys(n_page); i++) {
                tmp = ((internal_page_t*)n_page)->arr[i].page_number;
                file_read_page(tmp, tmp_page);
                ((internal_page_t*)tmp_page)->parent_page_number = n;
                file_write_page(tmp, tmp_page);
            }
            file_write_page(n, n_page);
        }



        else {
            for (i = neighbor_insertion_index, j = 0; j < get_num_keys(neighbor_page); i++, j++) {
                char buf[120];
                set_key(n_page, i, get_key(neighbor_page, i));
                set_num_keys(n_page, get_num_keys(n_page) + 1);
                get_value(neighbor_page, i, buf);
                set_value(n_page, i, buf);
            }
            ((leaf_page_t*)n_page)->sibling_page_number = ((leaf_page_t*)neighbor_page)->sibling_page_number;
            file_write_page(n, n_page);
        }
        delete_entry(parent, k_prime, NULL, neighbor);
        file_free_page(neighbor);
        free(tmp_page);
        free(n_page);
        free(neighbor_page);
    }
    return root;
}
pagenum_t redistribute_nodes(pagenum_t n,pagenum_t neighbor, int neighbor_index, int k_prime_index, element k_prime) {
    page_t* neighbor_page = (page_t*)malloc(sizeof(page_t));
    page_t* n_page = (page_t*)malloc(sizeof(page_t));
    file_read_page(n, n_page);
    file_read_page(neighbor, neighbor_page);
    pagenum_t tmp,parent;
    page_t* tmp_page= (page_t*)malloc(sizeof(page_t));
    int i;
    int n_key = ((internal_page_t*)n_page)->number_of_Keys;
    if (neighbor_index != -1) {
        ((internal_page_t*)n_page)->arr[0].page_number=((internal_page_t*)n_page)->left_page_num;
        ((internal_page_t*)n_page)->left_page_num = 
            ((internal_page_t*)neighbor_page)->arr[get_num_keys(neighbor_page) - 1].page_number;
        tmp = ((internal_page_t*)n_page)->left_page_num;
        file_read_page(tmp, tmp_page);
        ((internal_page_t*)tmp_page)->parent_page_number = n;
        file_write_page(tmp, tmp_page);
        ((internal_page_t*)n_page)->arr[0].key = k_prime;
        parent = ((internal_page_t*)n_page)->parent_page_number;
        file_read_page(parent, tmp_page);
        ((internal_page_t*)tmp_page)->arr[k_prime_index].key = 
            ((internal_page_t*)neighbor_page)->arr[get_num_keys(neighbor_page)-1].key;
        file_write_page(parent, tmp_page);
    }
    else {
        ((internal_page_t*)n_page)->arr[n_key].key = k_prime;
        ((internal_page_t*)n_page)->arr[n_key].page_number = ((internal_page_t*)neighbor_page)->left_page_num;
        tmp = ((internal_page_t*)n_page)->arr[n_key].page_number;
        file_read_page(tmp, tmp_page);
        ((internal_page_t*)tmp_page)->parent_page_number = n;
        file_write_page(tmp, tmp_page);
        parent = ((internal_page_t*)n_page)->parent_page_number;
        file_read_page(parent, tmp_page);
        ((internal_page_t*)tmp_page)->arr[k_prime_index].key = ((internal_page_t*)neighbor_page)->arr[0].key;
        file_write_page(parent, tmp_page);
        ((internal_page_t*)neighbor_page)->left_page_num = ((internal_page_t*)neighbor_page)->arr[0].page_number;
        for (i = 0; i < ((internal_page_t*)neighbor_page)->number_of_Keys-1; i++) {
            ((internal_page_t*)neighbor_page)->arr[i].key = ((internal_page_t*)neighbor_page)->arr[i + 1].key;
            ((internal_page_t*)neighbor_page)->arr[i].page_number = ((internal_page_t*)neighbor_page)->arr[i + 1].page_number;
        }
    }
    set_num_keys(n_page, get_num_keys(n_page) + 1);
    set_num_keys(neighbor_page, get_num_keys(neighbor_page) -1);
    file_write_page(n, n_page);
    file_write_page(neighbor, neighbor_page);
    free(tmp_page);
    free(n_page);
    free(neighbor_page);
    return 1;
}
/* Master deletion function.
 */
int delete(element key) {
    pagenum_t leaf, root = root_number();
    record_t* record = (record_t*)malloc(sizeof(record_t));
    record = find(key);
    leaf = find_leaf(key);
    if (record != NULL && leaf != 0) {
        root = delete_entry(leaf, key, record, 0);
        free(record);
        return root;
    }
    return -1;
}
