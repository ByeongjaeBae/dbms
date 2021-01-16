# Disk-based b+tree
## Milestion02
### 1. Possible call path of the insert/delete operation
    * 도움말
    {} : 함수를 의미한다.
    
> # 1. File Manager API
~~~ 
     1. struct page_t : 
        1-1 union 공용체로 page_t를 선언한다.
        1-2 페이지를 free, header, internal, leaf로 나눠서 구조체로 만들어준다.
        1-3 공용체로 선언해준 page_t에 위에서 만든 페이지 구조체로 만들어준다. 
    2. pagenum_t file_alloc_page() :
        2-1 free페이지에서 페이지 하나를 받아온다.
        2-2 이때 free페이지가 없을 경우, 10개의 새로운 프리페이지를 할당한다.
    3. void file_free_page(pagenum_t pagenum): 
        3-1 페이지를 free페이지로 바꿔준다.
        3-2 free페이지 리스트의 헤더로 설정해준다.
    4. void file_read_page(pagenum_t pagenum,page_t* dest):
        4-1 페이지 오프셋을 통해 dest로 오프셋에 해당하는 페이지 구조체를 저장한다.
        4-2 fread를 활용한다.
    5. void file_write_page(pagenum_t pagenum,const page_t *src): 
        5-1 디스크에 접근해서 in-memory page로 페이지로 할당해준다
        5-2 fwrite를 활용한다.
~~~
# 2. Implement disked on b+tree

~~~
    1. int open_table(char* pathname): 
        1-1 fopen으로 patnname에 파일을 열고, table_id를 전역으로 선언하고, 파일 이름을 저장할 배열을 선언한다.
        1-2 이미 한번 열었던 파일은 배열에서 이름을 확인한 후, 그 id를 리턴한다.
        1-3 새로 여는 파일일 경우는 본인의 유니크한 id를 리턴한다.
    2. int db_insert(int key, char* value): 
        2-1 함수 내부에서 insert함수를 call해준다. caller함수의 역할을 해준다.
        2-2 in-memory b+tree의 내용을 page에 맞게 설정해주면 된다.
        2-3 이때, internal노드의 자식과 링킹되는 부분이 연속적이지 않다. left_page_number는 따로 존재하기에 고려해서 디자인했다.
        2-4 이때, 성공하면 return 0, 이미 존재하는 key값이면 non zero value를 리턴한다.
    3. int db_find(int key,char* ret_val):
        3-1 find함수를 call해주는 caller함수의 역할을 한다.
        3-2 find함수에서 레코드를 반환하고 레코드의 value를 ret_val에다가 할당해준다.
        3-3 만약 찾지 못할 경우 non zero value를 리턴, 찾았을 경우 0을 리턴한다.
    4. int db_delete(int key) : 
        4-1 delete함수를 call해주는 caller함수의 역할을 한다.
        4-2 in-memory b+tree의 내용을 page에 맞게 설정해주면 된다.
        4-3 이때 delete같은 경우 re-distributed를 없애고 merge도 delayed merge형태로 바꿔줘야한다.
        4-4 insert와 마찬가지로 left-most-page-number가 따로 존재하기에 이거에 맞춰 delete관련 함수들을 바꿔줘야 한다.
        4-5 delete에 성공하면 0을 return 실패할경우 non zero value를 리턴한다.
    5. 위에 4가지 api를 돕기 위한 getter setter함수를 설정한다.
        5-1 페이지 구조체의 멤버들을 getter와 setter를 이용해서 접근하게 한다.
        5-2 대부분의 멤버는 그렇게 접근한다.
    6. delayed merge:
        6-1 internal page와 leaf page의 데이터가 전부 삭제 될때 merge가 되게 해야한다.
        6-2 neighbor index를 얻을 때 left-most는 배열의 시퀀스한 형태로 얻을 수 없기에 예외처리로 해준다.
        6-3 그리고 merge될 페이지가 left most인 경우와 아닌 경우로 나눈다.
        6-4 그 사이에서 leaf와 internal로 나누고, internal은 child page의 정보와 key를 전부 옮긴다.
        6-5 leaf같은 경우는 sibling page number와 parent page number만 다시 설정해주면 된다.
    
~~~


# Disk-based b+tree
## Milestion01
### 3. (Naïve) designs or required changes for building on-disk b+ tree

# 1. File Manager API

~~~
    1. struct page_t : page의 레이아웃과(구성)과 page_num을 만들어야한다. -> PAGE의 종류별로 따로 구현한다. 
    2. pagenum_t file_alloc_page(): free페이지 리스트로부터 page를 할당한다. -> free page가 없을 시 새로운 페이지를 생선한다.
    3. void file_free_page(pagenum_t pagenum): free페이지 리스트로부터 페이지를 free로바꾼다. -> free page list의 가장 처음에 배치하도록 만든다.
    4. void file_read_page(pagenum_t pagenum,page_t* dest): 페이지구조로(dest) 부터 페이지를 읽는다.
        -> pagenum이라는 오프셋을 통해 파일에 원하는 위치에 페이지를 읽는다.
    5. void file_write_page(pagenum_t pagenum,const page_t *src): in-memory page를 disk-page로 만든다. -> pagenum 오프셋을 이용하여 페이지를 파일에 작성한다.
~~~

# 2. Implement 4 commands 

~~~
    1. int open_table(char* pathname): Open existing data file using 'pathname' or create one if not existed. using file manager api.
    2. int db_insert(int key, char* value): insert input 'key/value(record)' to data file at the right place.
    3. int db_find(int key,char* ret_val): Find the record conataining input 'key'and retirn matching 'value' or 'null' if not found.
    4. int db_delete(int key) : Find the matching record and delete it if found.
~~~

# 3. page구현 

~~~
    페이지가 4096바이트 단위이르모, 4096을 기준으로 offset을 설정해준다.
    1. headerpage :  0~7바이트는 first free page를 포인팅한다, 8~15바이트는 root page를 포인팅, 16~23바이트는 datafile개수 포인팅
    2. freepage : 0~7바이트는 next freepage를 가리킨다.
    3. leafpage : key/value records를 가진다. fixed size record(8+120) 최대 31개 가능
    4. pageheader : internal/leaf page have 128 bytes as a page header. 0~7 point parent page
    8~11 leaf이면0 아니면 1 (4byte int), 12~15 페이지안에 있는 key의 개수
    5. leafpage의 120~127바이트를 right sibling pagenum을 가리키게한다.
    6. internal page : value대신 pagenumber(8바이트)를 가진다. 최대 249개 저장 가능
    7. 파일안에 4096바이트 단위로 잘라서 pagination을 실행한다.
~~~