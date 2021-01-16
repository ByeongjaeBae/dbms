#include "bpt.h"
int table_id = 1;
int db_insert(int table_id, element key, char* value) {
    if(table_id>10||table_id==-1)return -1;
    if (insert(table_id, key, value))return 0;
    else return -1;
}
int db_find(int table_id, element key, char* ret_val) {
    if(table_id>10||table_id==-1)return -1;
    record_t* record;
    if ((record = find(table_id, key)) == NULL) {
        return -1;
    }
    strncpy(ret_val, record->value, 120);
    return 0;
}
int db_delete(int table_id, element key) {
    if(table_id>10||table_id==-1)return -1;
    if (delete(table_id, key) >= 0)return 0;
    else return -1;
}

int open_table(char* pathname) {
    int flag=0;
    if(strlen(pathname)>20)return -1;
    if(table_id==1){
	for(int i=0;i<11;i++){
		fds[i]=-1;
	}
	}
    if (pathname == NULL) {
        printf("invalid pathname\n");
    }
    for (int i = 1; i <= table_id; i++) {
        if (strncmp(table[i], pathname, 120) == 0){
		flag=1;
	}
    }
        if (!flag&&table_id > 10) {
        printf("Maximum table\n");
        return -1;
    } 
    int fp=file_open(pathname);
    if (fp == -1)return -1;
    for (int i = 1; i <= table_id; i++) {
        if (strncmp(table[i], pathname, 120) == 0){
	    fds[i]=fp;
            return i;
	}
    }
    strcpy(table[table_id], pathname);
    fds[table_id] = fp;
    buffer* head_buf = get_frame(table_id, header_page_num);
    page_t* header_page = head_buf->page;
    if (feof_file(table_id)) {
        initial_page(header_page);
        global = 0;
        ((header_page_t*)header_page)->Number_of_pages = 1;
        put_frame(head_buf, 1);
        return table_id++;
    }
    put_frame(head_buf, 1);
    return table_id++;
}
node* queue = NULL;


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

node* dequeue(void) {
    node* n = queue;
    queue = queue->next;
    n->next = NULL;
    return n;
}


int path_to_root(int table_id, pagenum_t child) {
    page_t* header_page = (page_t*)malloc(sizeof(page_t));
    file_read_page(table_id, header_page_num, header_page);
    pagenum_t root = root_number(header_page);
    int length = 0;
    page_t* c_page = (page_t*)malloc(sizeof(page_t));
    file_read_page(table_id, child, c_page);
    pagenum_t c = child;
    while (c != root) {
        c = ((internal_page_t*)c_page)->parent_page_number;
        file_read_page(table_id, c, c_page);
        length++;
    }
    free(c_page);
    return length;
}

void print_tree(int table_id) {
    page_t* header_page = (page_t*)malloc(sizeof(page_t));
    file_read_page(table_id, header_page_num, header_page);
    pagenum_t root = root_number(header_page);
    page_t* root_page = (page_t*)malloc(sizeof(page_t));
    page_t* parent_page = (page_t*)malloc(sizeof(page_t));
    page_t* n_page = (page_t*)malloc(sizeof(page_t));
    file_read_page(table_id, root, root_page);
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
        file_read_page(table_id, n->page, n_page);
        file_read_page(table_id, get_parent_page(n_page), parent_page);
        if (((internal_page_t*)n_page)->parent_page_number != 0
            && n->page == ((internal_page_t*)parent_page)->left_page_num) {
            new_rank = path_to_root(table_id, n->page);
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
        printf("(page:%u) ", n->page);
        //printf("(parent:%u)", get_parent_page(n_page));
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

int get_num_keys(page_t* page) {
    return ((leaf_page_t*)page)->Number_of_Keys;
}
void set_num_keys(page_t* page, int num) {
    ((leaf_page_t*)page)->Number_of_Keys = num;
}
pagenum_t root_number(page_t* header_page) {
    pagenum_t tmp = ((header_page_t*)header_page)->root_page_number;
    return tmp;
}
void set_root_number(int table_id, pagenum_t pg) {
    buffer* head_buf = get_frame(table_id, header_page_num);
    page_t* header_page = head_buf->page;
    ((header_page_t*)header_page)->root_page_number = pg;
    put_frame(head_buf, 1);
}
pagenum_t find_leaf(int table_id, element key) {
    int i = 0;
    buffer* root_buf, * leaf_buf;
    page_t* header_page;
    buffer* head_buf = get_frame(table_id, header_page_num);
    header_page = head_buf->page;
    pagenum_t root_num = root_number(header_page);
    put_frame(head_buf, 0);
    pagenum_t leaf_num = root_num;
    if (root_num == 0) {
        return 0;
    }
    page_t* page;
    root_buf = get_frame(table_id, root_num);
    page = root_buf->page;
    put_frame(root_buf, 0);
    while (!((leaf_page_t*)page)->is_Leaf) {
        i = 0;
        while (i < get_num_keys(page)) {
            if (key >= ((internal_page_t*)page)->arr[i].key) i++;
            else break;
        }
        if (i == 0)leaf_num = ((internal_page_t*)page)->left_page_num;
        else leaf_num = ((internal_page_t*)page)->arr[i - 1].page_number;
        leaf_buf = get_frame(table_id, leaf_num);
        page = leaf_buf->page;
        put_frame(leaf_buf, 0);
    }
    return leaf_num;
}
record_t* find(int table_id, element key) {
    int i = 0;
    buffer* buf;
    page_t* page;
    pagenum_t leaf_num = find_leaf(table_id, key);
    if (leaf_num == 0) return NULL;
    buf = get_frame(table_id, leaf_num);
    page = buf->page;
    record_t* ret = (record_t*)malloc(sizeof(record_t));
    for (i = 0; i < ((leaf_page_t*)page)->Number_of_Keys; i++)
        if (((leaf_page_t*)page)->record[i].key == key) break;
    if (i == ((leaf_page_t*)page)->Number_of_Keys) {
        put_frame(buf, 0);
        return NULL;
    }
    else {
        ret->key = key;
        strncpy(ret->value, ((leaf_page_t*)page)->record[i].value, 120);
        put_frame(buf, 0);
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
pagenum_t make_page(int table_id) {
    pagenum_t new_page_num = file_alloc_page(table_id);
    buffer* new_buf = get_frame(table_id, new_page_num);
    page_t* new_page = new_buf->page;
    ((internal_page_t*)new_page)->parent_page_number = 0;
    put_frame(new_buf, 1);
    return new_page_num;
}

/* Creates a new leaf by creating a node
 * and then adapting it appropriately.
 */
pagenum_t make_leaf(int table_id) {
    pagenum_t leaf_num = make_page(table_id);
    buffer* leaf_buf = get_frame(table_id, leaf_num);
    page_t* leaf = leaf_buf->page;
    ((leaf_page_t*)leaf)->is_Leaf = 1;
    put_frame(leaf_buf, 1);
    return leaf_num;
}


pagenum_t insert(int table_id, element key, char* value) {
    buffer* head_buf = get_frame(table_id, header_page_num);
    page_t* header_page = head_buf->page;
    pagenum_t root = root_number(header_page);
    put_frame(head_buf, 0);
    if (find(table_id, key) != NULL)
        return 0;
    record_t* pointer = make_record(key, value);

    if (root == 0)
        return start_new_tree(table_id, pointer);


    /* Case: the tree already exists.
     * (Rest of function body.)
     */
    pagenum_t leaf = find_leaf(table_id, key);
    page_t* leaf_page;
    buffer* leaf_buf = get_frame(table_id, leaf);
    leaf_page = leaf_buf->page;
    /* Case: leaf has room for key and pointer.
     */
    put_frame(leaf_buf, 0);
    if (((leaf_page_t*)leaf_page)->Number_of_Keys < leaf_order - 1) {
        return insert_into_leaf(table_id, leaf, pointer);
    }


    return insert_into_leaf_after_splitting(table_id, leaf, pointer);
}

pagenum_t insert_into_leaf_after_splitting(int table_id, pagenum_t leaf, record_t* pointer) {
    pagenum_t new_leaf;
    page_t* new_leaf_page, * leaf_page;
    record_t* temp_record;
    element new_key;
    int insertion_index, split, i, j;
    buffer* leaf_buf, * new_buf;
    new_leaf = make_leaf(table_id);
    new_buf = get_frame(table_id, new_leaf);
    new_leaf_page = new_buf->page;
    leaf_buf = get_frame(table_id, leaf);
    leaf_page = leaf_buf->page;
    temp_record = (record_t*)malloc(leaf_order * sizeof(record_t));
    if (temp_record == NULL) {
        leaf_buf->is_pinned = 0;
        new_buf->is_pinned = 0;
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
    put_frame(leaf_buf, 1);
    put_frame(new_buf, 1);
    return insert_into_parent(table_id, leaf, new_key, new_leaf);
}
int get_left_index(int table_id, pagenum_t parent, pagenum_t left) {
    page_t* parent_page;
    buffer* parent_buf = get_frame(table_id, parent);
    parent_page = parent_buf->page;
    int left_index = 0;
    if (((internal_page_t*)parent_page)->left_page_num == left) {
        put_frame(parent_buf, 0);
        return -1;
    }
    while (left_index <= get_num_keys(parent_page) &&
        (((internal_page_t*)parent_page)->arr[left_index].page_number != left))
        left_index++;
    put_frame(parent_buf, 0);
    return left_index;
}
/* Inserts a new pointer to a record and its corresponding
 * key into a leaf.
 * Returns the altered leaf.
 */
pagenum_t insert_into_leaf(int table_id, pagenum_t leaf, record_t* pointer) {
    buffer* leaf_buf;
    page_t* leaf_page;
    char buf[120];
    leaf_buf = get_frame(table_id, leaf);
    leaf_page = leaf_buf->page;
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
    put_frame(leaf_buf, 1);
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
pagenum_t insert_into_node(int table_id, pagenum_t parent,
    int left_index, element key, pagenum_t right) {
    int i;
    page_t* parent_page, * header_page;
    buffer* head_buf = get_frame(table_id, header_page_num);
    header_page = head_buf->page;
    buffer* parent_buf = get_frame(table_id, parent);
    parent_page = parent_buf->page;
    for (i = get_num_keys(parent_page); i > left_index + 1; i--) {
        ((internal_page_t*)parent_page)->arr[i].page_number =
            ((internal_page_t*)parent_page)->arr[i - 1].page_number;
        ((internal_page_t*)parent_page)->arr[i].key =
            ((internal_page_t*)parent_page)->arr[i - 1].key;
    }
    ((internal_page_t*)parent_page)->arr[left_index + 1].page_number = right;
    ((internal_page_t*)parent_page)->arr[left_index + 1].key = key;
    set_num_keys(parent_page, get_num_keys(parent_page) + 1);
    put_frame(parent_buf, 1);
    pagenum_t root = root_number(header_page);
    put_frame(head_buf, 0);
    return root;
}


/* Inserts a new key and pointer to a node
 * into a node, causing the node's size to exceed
 * the order, and causing the node to split into two.
 */
pagenum_t insert_into_node_after_splitting(int table_id, pagenum_t parent, int left_index,
    element key, pagenum_t right) {
    buffer* parent_buf, * new_buf, * child_buf;
    int i, j, split;
    element k_prime;
    parent_buf = get_frame(table_id, parent);
    page_t* parent_page = parent_buf->page;
    nums* temp_nums = (nums*)malloc(sizeof(nums) * (internal_order));
    pagenum_t new = make_page(table_id);
    new_buf = get_frame(table_id, new);
    page_t* new_page = new_buf->page;
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
    page_t* child_page;
    pagenum_t child;
    child = ((internal_page_t*)new_page)->left_page_num;
    child_buf = get_frame(table_id, child);
    child_page = child_buf->page;
    ((internal_page_t*)child_page)->parent_page_number = new;
    put_frame(child_buf, 1);
    for (i = 0; i < get_num_keys(new_page); i++) {
        child = get_pagenum(new_page, i);
        child_buf = get_frame(table_id, child);
        child_page = child_buf->page;
        ((internal_page_t*)child_page)->parent_page_number = new;
        put_frame(child_buf, 1);
    }
    /* Insert a new key into the parent of the two
     * nodes resulting from the split, with
     * the old node to the left and the new to the right.
     */
    put_frame(parent_buf, 1);
    put_frame(new_buf, 1);
    return insert_into_parent(table_id, parent, k_prime, new);
}



/* Inserts a new node (leaf or internal node) into the B+ tree.
 * Returns the root of the tree after insertion.
 */
pagenum_t insert_into_parent(int table_id, pagenum_t left, element key, pagenum_t right) {
    int left_index;
    pagenum_t parent;
    page_t* parent_page, * left_page;
    buffer* parent_buf, * left_buf;
    left_buf = get_frame(table_id, left);
    left_page = left_buf->page;
    parent = get_parent_page(left_page);
    parent_buf = get_frame(table_id, parent);
    parent_page = parent_buf->page;
    int parent_num_key = get_num_keys(parent_page);
    /* Case: new root. */
    put_frame(parent_buf, 0);
    put_frame(left_buf, 0);
    if (parent == 0)
        return insert_into_new_root(table_id, left, key, right);

    /* Case: leaf or node. (Remainder of
     * function body.)
     */

     /* Find the parent's pointer to the left
      * node.
      */

    left_index = get_left_index(table_id, parent, left);


    /* Simple case: the new key fits into the node.
     */
    if (parent_num_key < internal_order - 1)
        return insert_into_node(table_id, parent, left_index, key, right);

    /* Harder case:  split a node in order
     * to preserve the B+ tree properties.
     */

    return insert_into_node_after_splitting(table_id, parent, left_index, key, right);
}


/* Creates a new root for two subtrees
 * and inserts the appropriate key into
 * the new root.
 */
pagenum_t insert_into_new_root(int table_id, pagenum_t left, element key, pagenum_t right) {
    pagenum_t root = make_page(table_id);
    page_t* root_page, * left_page, * right_page;
    buffer* root_buf = get_frame(table_id, root);
    buffer* left_buf = get_frame(table_id, left);
    buffer* right_buf = get_frame(table_id, right);
    root_page = root_buf->page;
    left_page = left_buf->page;
    right_page = right_buf->page;
    ((internal_page_t*)root_page)->arr[0].key = key;
    ((internal_page_t*)root_page)->left_page_num = left;
    ((internal_page_t*)root_page)->arr[0].page_number = right;
    set_num_keys(root_page, get_num_keys(root_page) + 1);
    set_parent_page(left_page, root);
    set_parent_page(right_page, root);
    put_frame(root_buf, 1);
    put_frame(left_buf, 1);
    put_frame(right_buf, 1);
    set_root_number(table_id, root);
    return root;
}



/* First insertion:
 * start a new tree.
 */
pagenum_t start_new_tree(int table_id, record_t* pointer) {
    pagenum_t leaf = make_leaf(table_id);
    page_t* root;
    buffer* buf = get_frame(table_id, leaf);
    root = buf->page;
    ((leaf_page_t*)root)->record[0].key = pointer->key;
    strncpy(((leaf_page_t*)root)->record[0].value, pointer->value, 120);
    ((leaf_page_t*)root)->Number_of_Keys++;
    ((leaf_page_t*)root)->Parent_page_number = 0;
    set_root_number(table_id, leaf);
    put_frame(buf, 1);
    return leaf;
}



int get_neighbor_index(int table_id, pagenum_t n) {
    buffer* r_buf;
    page_t* r_page;
    buffer* n_buf = get_frame(table_id, n);
    page_t* n_page = n_buf->page;
    int i;
    pagenum_t parent = get_parent_page(n_page);
    put_frame(n_buf, 0);
    buffer* parent_buf = get_frame(table_id, parent);
    page_t* parent_page = parent_buf->page;
    put_frame(parent_buf, 0);
    if (n == ((internal_page_t*)parent_page)->left_page_num) {
        return -1;
    }
    for (i = 0; i < get_num_keys(parent_page); i++)//1
        if (((internal_page_t*)parent_page)->arr[i].page_number == n) {
            return i;
        }

}
pagenum_t remove_entry_from_node(int table_id, pagenum_t n, element key, pagenum_t p) {
    int i, num_values;

    // Remove the key and shift other keys accordingly.
    i = 0;
    buffer* n_buf = get_frame(table_id, n);
    page_t* n_page = n_buf->page;
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
    put_frame(n_buf, 1);
    return n;
}

pagenum_t remove_entry_from_leaf(int table_id, pagenum_t n, element key, record_t* pointer) {
    int i, num_values;

    // Remove the key and shift other keys accordingly.
    i = 0;
    buffer* n_buf = get_frame(table_id, n);
    page_t* n_page = n_buf->page;
    while (get_key(n_page, i) != key)
        i++;
    for (++i; i < get_num_keys(n_page); i++){
        set_key(n_page, i - 1, get_key(n_page, i));
        strncpy(((leaf_page_t*)n_page)->record[i - 1].value,
            ((leaf_page_t*)n_page)->record[i].value, 120);
}



    // One key fewer.
    set_num_keys(n_page, get_num_keys(n_page) - 1);
    put_frame(n_buf, 1);
    return n;
}


pagenum_t adjust_root(int table_id, pagenum_t root) {
    buffer* root_buf = get_frame(table_id, root);
    buffer* new_root_buf;
    pagenum_t new_root;
    page_t* new_root_page;
    page_t* root_page = root_buf->page;
    put_frame(root_buf, 0);


    if (get_num_keys(root_page) > 0) {
        return root;
    }


    if (!((leaf_page_t*)root_page)->is_Leaf) {
        new_root = ((internal_page_t*)root_page)->left_page_num;
        new_root_buf = get_frame(table_id, new_root);
        new_root_page = new_root_buf->page;
        ((internal_page_t*)new_root_page)->parent_page_number = 0;
        put_frame(new_root_buf, 1);
        set_root_number(table_id, new_root);
    }

    else
        new_root = 0;
    file_free_page(table_id, root);
    set_root_number(table_id, new_root);
    return new_root;
}

pagenum_t delete_entry(int table_id, pagenum_t n, element key, record_t* pointer, pagenum_t pg) {
    page_t* n_page;
    int neighbor_index;
    element k_prime;
    pagenum_t neighbor;
    buffer* head_buf = get_frame(table_id, header_page_num);
    page_t* header_page = head_buf->page;
    put_frame(head_buf, 0);
    pagenum_t root = root_number(header_page);
    buffer* n_buf = get_frame(table_id, n);
    n_page = n_buf->page;
    put_frame(n_buf, 0);
    // Remove key and pointer from node.
    if (((leaf_page_t*)n_page)->is_Leaf)
        n = remove_entry_from_leaf(table_id, n, key, pointer);
    else
        n = remove_entry_from_node(table_id, n, key, pg);



    if (n == root) {
        return adjust_root(table_id, root);
    }

    n_buf = get_frame(table_id, n);
    n_page = n_buf->page;
    put_frame(n_buf, 0);
    int num_keys = get_num_keys(n_page);

    pagenum_t parent = get_parent_page(n_page);
    buffer* parent_buf = get_frame(table_id, parent);
    page_t* parent_page = parent_buf->page;
    put_frame(parent_buf, 0);
    neighbor_index = get_neighbor_index(table_id, n);
    if (neighbor_index == -1)k_prime = ((internal_page_t*)parent_page)->arr[0].key;
    else k_prime = ((internal_page_t*)parent_page)->arr[neighbor_index].key;
    int k_prime_index = neighbor_index == -1 ? 0 : neighbor_index;
    if (neighbor_index == 0) {
        neighbor = ((internal_page_t*)parent_page)->left_page_num;
    }
    else neighbor = neighbor_index == -1 ? ((internal_page_t*)parent_page)->arr[0].page_number :
        ((internal_page_t*)parent_page)->arr[neighbor_index - 1].page_number;
    buffer* neighbor_buf = get_frame(table_id, neighbor);
    page_t* neighbor_page = neighbor_buf->page;
    put_frame(neighbor_buf, 0);
    if (num_keys <= 0 && (!((internal_page_t*)n_page)->is_Leaf) && get_num_keys(neighbor_page) == (internal_order - 1))
        redistribute_nodes(table_id, n, neighbor, neighbor_index, k_prime_index, k_prime);
    else if (num_keys <= 0) {
        coalesce_nodes(table_id, n, neighbor, neighbor_index, k_prime);
        return root;
    }
    return root;
}

pagenum_t coalesce_nodes(int table_id, pagenum_t n, pagenum_t neighbor, int neighbor_index, element k_prime) {
    buffer* head_buf = get_frame(table_id, header_page_num);
    page_t* header_page = head_buf->page;
    put_frame(head_buf, 0);
    int i, j, neighbor_insertion_index;
    pagenum_t tmp, root = root_number(header_page);
    page_t* neighbor_page, * n_page, * tmp_page;
    buffer* n_buf = get_frame(table_id, n);
    buffer* neighbor_buf = get_frame(table_id, neighbor);
    buffer* tmp_buf;
    n_page = n_buf->page;
    neighbor_page = neighbor_buf->page;
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
            tmp_buf = get_frame(table_id, tmp);
            tmp_page = tmp_buf->page;
            ((internal_page_t*)tmp_page)->parent_page_number = neighbor;
            put_frame(tmp_buf, 1);
            for (i = 0; i < get_num_keys(neighbor_page); i++) {
                tmp = ((internal_page_t*)neighbor_page)->arr[i].page_number;
                tmp_buf = get_frame(table_id, tmp);
                tmp_page = tmp_buf->page;
                ((internal_page_t*)tmp_page)->parent_page_number = neighbor;
                put_frame(tmp_buf, 1);
            }
            put_frame(neighbor_buf, 1);
            put_frame(n_buf, 0);
        }
        else {
            ((leaf_page_t*)neighbor_page)->sibling_page_number = ((leaf_page_t*)n_page)->sibling_page_number;
            put_frame(neighbor_buf, 1);
            put_frame(n_buf, 0);
        }
        delete_entry(table_id, parent, k_prime, NULL, n);
        file_free_page(table_id, n);
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
            tmp_buf = get_frame(table_id, tmp);
            tmp_page = tmp_buf->page;
            ((internal_page_t*)tmp_page)->parent_page_number = n;
            put_frame(tmp_buf, 1);
            for (i = 0; i < get_num_keys(n_page); i++) {
                tmp = ((internal_page_t*)n_page)->arr[i].page_number;
                tmp_buf = get_frame(table_id, tmp);
                tmp_page = tmp_buf->page;
                ((internal_page_t*)tmp_page)->parent_page_number = n;
                put_frame(tmp_buf, 1);
            }
            put_frame(n_buf, 1);
            put_frame(neighbor_buf, 0);
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
            put_frame(n_buf, 1);
            put_frame(neighbor_buf, 0);
        }
        delete_entry(table_id, parent, k_prime, NULL, neighbor);
        file_free_page(table_id, neighbor);
    }
    return root;
}
pagenum_t redistribute_nodes(int table_id, pagenum_t n, pagenum_t neighbor, int neighbor_index, int k_prime_index, element k_prime) {
    page_t* neighbor_page, * n_page;
    buffer* neighbor_buf = get_frame(table_id, neighbor);
    buffer* tmp_buf, * parent_buf;
    buffer* n_buf = get_frame(table_id, n);
    n_page = n_buf->page;
    neighbor_page = neighbor_buf->page;
    pagenum_t tmp, parent;
    page_t* tmp_page;
    int i;
    int n_key = ((internal_page_t*)n_page)->number_of_Keys;
    if (neighbor_index != -1) {
        ((internal_page_t*)n_page)->arr[0].page_number = ((internal_page_t*)n_page)->left_page_num;
        ((internal_page_t*)n_page)->left_page_num =
            ((internal_page_t*)neighbor_page)->arr[get_num_keys(neighbor_page) - 1].page_number;
        tmp = ((internal_page_t*)n_page)->left_page_num;
        tmp_buf = get_frame(table_id, tmp);
        tmp_page = tmp_buf->page;
        ((internal_page_t*)tmp_page)->parent_page_number = n;
        put_frame(tmp_buf, 1);
        ((internal_page_t*)n_page)->arr[0].key = k_prime;
        parent = ((internal_page_t*)n_page)->parent_page_number;
        parent_buf = get_frame(table_id, parent);
        tmp_page = parent_buf->page;
        ((internal_page_t*)tmp_page)->arr[k_prime_index].key =
            ((internal_page_t*)neighbor_page)->arr[get_num_keys(neighbor_page) - 1].key;
        put_frame(parent_buf, 1);
    }
    else {
        ((internal_page_t*)n_page)->arr[n_key].key = k_prime;
        ((internal_page_t*)n_page)->arr[n_key].page_number = ((internal_page_t*)neighbor_page)->left_page_num;
        tmp = ((internal_page_t*)n_page)->arr[n_key].page_number;
        tmp_buf = get_frame(table_id, tmp);
        tmp_page = tmp_buf->page;
        ((internal_page_t*)tmp_page)->parent_page_number = n;
        put_frame(tmp_buf, 1);
        parent = ((internal_page_t*)n_page)->parent_page_number;
        parent_buf = get_frame(table_id, parent);
        tmp_page = parent_buf->page;
        ((internal_page_t*)tmp_page)->arr[k_prime_index].key = ((internal_page_t*)neighbor_page)->arr[0].key;
        put_frame(parent_buf, 1);
        ((internal_page_t*)neighbor_page)->left_page_num = ((internal_page_t*)neighbor_page)->arr[0].page_number;
        for (i = 0; i < ((internal_page_t*)neighbor_page)->number_of_Keys - 1; i++) {
            ((internal_page_t*)neighbor_page)->arr[i].key = ((internal_page_t*)neighbor_page)->arr[i + 1].key;
            ((internal_page_t*)neighbor_page)->arr[i].page_number = ((internal_page_t*)neighbor_page)->arr[i + 1].page_number;
        }
    }
    set_num_keys(n_page, get_num_keys(n_page) + 1);
    set_num_keys(neighbor_page, get_num_keys(neighbor_page) - 1);
    put_frame(n_buf, 1);
    put_frame(neighbor_buf, 1);
    return 1;
}

int delete(int table_id, element key) {
    buffer* head_buf = get_frame(table_id, header_page_num);
    page_t* header_page = head_buf->page;
    pagenum_t leaf, root = root_number(header_page);
    put_frame(head_buf, 0);
    record_t* record = (record_t*)malloc(sizeof(record_t));
    record = find(table_id, key);
    leaf = find_leaf(table_id, key);
    if (record != NULL && leaf != 0) {
        root = delete_entry(table_id, leaf, key, record, 0);
        free(record);
        return root;
    }
    return -1;
}
