# Disk-based b+tree
## Milestion02
### Disk-based b+tree design
    
> # 1. File Manager API
~~~ 
     1. struct page_t : 
        1-1 구조체 page_t를 선언한다.
        1-2 페이지를 free, header, internal, leaf로 나눠서 구조체로 해서 page헤더파일에 생성해준다.
    2. pagenum_t file_alloc_page() :
        2-1 헤더페이지를 읽어서 free페이지에서 페이지 하나를 받아온다.
        2-2 이때 free페이지가 없을 경우, 10개의 새로운 프리페이지를 할당한다.
    3. void file_free_page(pagenum_t pagenum): 
        3-1 페이지를 free페이지로 바꿔준다.
        3-2 헤더페이지를 읽어서 free페이지 리스트의 헤더로 설정해준다.
    4. void file_read_page(pagenum_t pagenum,page_t* dest):
        4-1 페이지 오프셋을 통해 dest로 오프셋에 해당하는 페이지 구조체를 저장한다.
        4-2 read를 활용한다.
    5. void file_write_page(pagenum_t pagenum,const page_t *src): 
        5-1 디스크에 접근해서 in-memory page로 페이지로 할당해준다
        5-2 write를 활용한다.
    6. void initial_page(page_t* page) : page의 내부를 전부 0으로 초기화한다.
    7. int add_free_page() : file_alloc_page()와 연계하여 free_page 10개를 추가한다.
    8. int feof_file() : file이 비어있는지 체크해준다. open_table과 연계
~~~
# 2. Implement disked on b+tree

~~~
    1. int open_table(char* pathname): 
        1-1 open으로 patnname에 파일을 열고, table_id를 전역으로 선언하고, 파일 이름을 저장할 배열을 선언한다.
        1-2 이미 한번 열었던 파일은 배열에서 이름을 확인한 후, 그 id를 리턴한다.
        1-3 새로 여는 파일일 경우는 본인의 유니크한 id를 리턴한다.
        1-4 file이 비었을 경우 헤더페이지를 0으로 초기화하고 write해준다.
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
        6-6 만약 neighbor_internal_page가 이미 full일 경우 merge가 불가능하다. 따라서 redistribution을 활용한다.
    
~~~