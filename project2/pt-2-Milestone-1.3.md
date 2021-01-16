# Disk-based b+tree
## Milestion01
### 3. (Naïve) designs or required changes for building on-disk b+ tree

# 1. File Manager API

~~~
    1. struct page_t : page의 레이아웃과(구성)과 page_num을 만들어야한다. -> PAGE의 종류별로 따로 구현한다. 
    2. pagenum_t file_alloc_page(): free페이지 리스트로부터 page를 할당한다. 
       -> free page가 없을 시 새로운 페이지를 생선한다.
    3. void file_free_page(pagenum_t pagenum): free페이지 리스트로부터 페이지를 free로바꾼다. 
       -> free page list의 가장 처음에 배치하도록 만든다.
    4. void file_read_page(pagenum_t pagenum,page_t* dest): 페이지구조로(dest) 부터 페이지를 읽는다.
       -> pagenum이라는 오프셋을 통해 파일에 원하는 위치에 페이지를 읽는다.
    5. void file_write_page(pagenum_t pagenum,const page_t *src): in-memory page를 disk-page로 만든다. 
       -> pagenum 오프셋을 이용하여 페이지를 파일에 작성한다.
~~~

# 2. Implement 4 commands 

~~~
    1. int open_table(char* pathname): Open existing data file using 'pathname' or create one if not existed. using file manager api.
       - fopen을 이용해서 file을 열고, 유니크한 테이블 id를 배열로 저장
    2. int db_insert(int key, char* value): insert input 'key/value(record)' to data file at the right place.
       - 메모리 기반의 node를 page로 전부 바꾼다.
    3. int db_find(int key,char* ret_val): Find the record conataining input 'key'and return matching 'value' or 'null' if not found.
       - 메모리 기반을 페이지로 바꾼다.
    4. int db_delete(int key) : Find the matching record and delete it if found. 
       -내부에 delayed merge구현
~~~

# 3. page구현 

~~~
    페이지가 4096바이트 단위이므로, 4096을 기준으로 offset을 설정해준다.
    구조체로 각각 선언해준다.
    1. headerpage :  0~7바이트는 first free page를 포인팅한다, 8~15바이트는 root page를 포인팅, 16~23바이트는 datafile개수 포인팅
    2. freepage : 0~7바이트는 next freepage를 가리킨다.
    3. leafpage : key/value records를 가진다. fixed size record(8+120) 최대 31개 가능
    4. pageheader : internal/leaf page have 128 bytes as a page header. 0~7 point parent page
                    8~11 leaf이면0 아니면 1 (4byte int), 12~15 페이지안에 있는 key의 개수
    5. leafpage의 120~127바이트를 right sibling pagenum을 가리키게한다.
    6. internal page : value대신 pagenumber(8바이트)를 가진다. 최대 248개 저장 가능
    7. 파일안에 4096바이트 단위로 잘라서 pagination을 실행한다.
~~~