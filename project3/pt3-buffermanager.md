# buffer-manager
## LRU-POLICY-design

> # 1. Design summary
~~~ 

     1. 버퍼풀에 페이지를 담고 memory에서 페이지를 수정할 수 있다. 이것을 통해 write와 read의 횟수를 줄일 수 있다. 
     2. 페이지에는 pin과 dirty가 존재하고 pinned 되어있으면 페이지를 현재 메모리에서 사용중인 것이고 dirty는 페이지가 수정된걸 의미한다.
     3. 페이지를 읽을때 버퍼에서 읽어오고 버퍼에 없을 경우 LRU-POLICY를 통해 버퍼에 새로운 페이지를 읽어온다.
     4. 우리는 페이지를 버퍼에서 받아올때 pin해주는 것으로 설정했다. 
     5. 페이지를 버퍼에서 받아오고 사용한다음 PUT을 해준다. LRU-POLICY정책에 따라 헤드->NEXT로 옮겨주고, 동시에 PIN을 해제한다. 
     6. LRU-POLICY를 통해 최근에 사용한것은 HEAD->NEXT로 보내주면서 가장 나중에 사용한것들은 HEAD->PREV로 가게 된다.
     7. 페이지가 수정이 된 경우 PUT을 할때 DIRTY를 체크해주는 작업을 통해 WRITE할 페이지를 결정한다.
     8. 여러개의 테이블을 사용하게 되므로 기존의 프로젝트에서 전역으로 선언한 페이지 수를 프리페이지를 추가해줄때마다 테이블에 맞는걸로 동 
        기화 해줘야 한다.
     9. get_frame과 put_frame의 호출 횟수는 동일해야 한다. 그래야 연산이 끝났을 때 pinned되어 있는 프레임이 없게 된다.
     10. 테이블의 id와 이름을 유지하기 위해 file descriptor의 배열과 pathname의 배열을 만들어서, 항상 유지시킨다.

~~~
    
> # 2. buffer manager
~~~ 
     1. struct buffer : 
        1-1 기본적인 과제명세에 있는 그대로 구현하였다.
        2-2 버퍼 풀에 있는 내부 구조체들을 모두 더블 linked list로 구현하도록 하였다.
    2. init_db(int num_buf) :
        2-1 더블 링크드리스트의 헤드를 구현해놓는다.
        2-2 받은 인자의 수만큼 buffer를 동적할당해주고 이것들을 헤드와 더블링크드리스트로 연결해준다.
        2-3 구조체 내부의 값들을 0으로 초기화 시켜준다.
    3. close_table(int table_id): 
        3-1 table_id에 해당하는 버퍼 구조체를 dirty일 경우 전부 write_buf api를 통해 write해주는 작업이다.
        3-2 그 이후 table을 close해준다.
    4. shutdown_db():
        4-1 모든 버퍼를 fllush해주는 작업이다. 구조체가 dirty일 경우 wrie_buf api를 통해 write해준다 
        4-2 현재 존재하는 모든 파일들을 close해준다.
    5. write_buf(buffer* frame): 
        5-1 인자로 받은 frame에 dirty를 체크하고 dirty일 경우 file_api를 통해 write해준다.
    6. get_frame(int table_id,pagenum_t page) :
        6-1 버퍼 리스트에서 table_id와 page에 해당하는 프레임을 가져온다. 
        6-2 이에 해당하는 프레임을 PINNED해준다.
        6-3 만약 해당하는 페이지가 없을 경우 LRU_POLICY 정책에 따라 HEAD->PREV부터 HEAD에서 먼 페이지들을 탐색한다.
        6-4 이때 프레임이 PINNED 되어 있지 않은 것을 버리고 원하는 페이지를 읽어온다. 
    7. put_frame(buffer* frame, int is_dirty) : 
        7-1 사용을 한 frame을 LRU-POLICY에 따라 HEAD->NEXT로 옮겨주는 작업을 한다.
        7-2 이때 UNPINNED로 바꿔주고 인자로 입력받은 DIRTY 값에 따라 is_dirty를 수정 해 준다.

~~~
# 3. LRU-POLICY 및 성능 분석

~~~
    1. 가장 최근에 가져온 프레임을 헤드에 가깝게 만들고 결과적으로 사용한지 오래된 것은 자동으로 헤드에서 멀어지게 된다.
    2. 버퍼풀이 전부 페이지로 가득 차 있을때 헤드에서 멀어진 것을 vitim으로 선정해 evict해주고 원하는 페이지를 가져온다.
    3. 1000개 insert기준 
       3-1 기존 비플트리 : read-11559회, write-1455회
       3-2 버퍼매니저 size-4 비플트리 : read :64회, wrtie:130회
       3-3 버퍼매니저 size-100 비플트리 : read : 62회, write :70회
       3-4 버퍼매니저 size-1000 비플트리 : read : 62회, write :70회
       -> 기존 비플트리에 비해 훨씬 disk i/o횟수가 적음을 알 수 있다. 그리고 일정 이상일 disk i/o 횟수가 같으나 연산양이 늘어나면
          버퍼풀이 클 수 록 유리하다. 
   4. 10000개 insert기준
      4-1 size-100 버퍼매니저 : read/write : 약 1,100회씩
      4-2 size-1000 버퍼매니저 : read/write : 약 600회씩 
   5. 20000개 insert, 7000개 random delete, 20000개 find, 20000개 insert, 20000개 find의 연속 연산 시간비교
      5-1 기존 비플트리 : 100초 가량
      5-2 버퍼매니저 비플트리(1000개 버퍼 풀) : 24초 가량
   6. 이처럼 disk i/o횟수가 버퍼매니저의 유무 연산량 그리고 버퍼풀의 크기에 따라 달라진다. 이를 통해 버퍼매니저의 속도 향상이 증명된다.
   

~~~