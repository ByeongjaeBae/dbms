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
