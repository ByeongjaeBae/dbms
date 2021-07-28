# lock_manager
## Multi-thread-design

> # 1. Design summary
~~~ 

     1. main 함수는 transfer_thread 8개와 scan_thread 1개가 각 함수에서 100만번씩 순회한다. 
     2. 기본적인 lock_table이 존재하고, 이 lock_table은 한번 접근할때 하나의 쓰레드만 접근해야 한다.
     3. 위처럼 쓰레드가 순차적으로 접근을 해줘야 하므로 lock_table에 접근하는 함수에는 mutex를 통해 쓰레드를 조절해준다.
     4. 하나의 레코드를 위한 접근을 위해 여러 쓰레드가 접근할 경우 순서를 부여해줘야 한다. FIFO의 순서이다. 
     5. hash_entry의 head와 tail을 통해 쓰레드를 이중 연결리스트로 연결해 순서를 부여한다.
     5. 레코드의 빠른 접근을 위해 해쉬테이블을 만들어주고 table_id와 key의 유니크한 조합을 해쉬테이블의 키로 가지게 한다.

~~~
    
> # 2. hash_table
~~~ 
    1. c++의 unordered_map을 사용하였다.
    2. table_id와 key의 유니크한 값을 만들어 주기 위해 hashcode를 table_id<<16+key 로 해쉬코드를 만들어준다.
    3. hash의 키를 hashcode로 value를 hash_entry라는 구조체로 해준다.
    4. hash_entry는 head와 tail 그리고 table_id와 record_id를 갖는다. 쓰레드들의 진입점이 된다.
~~~
> # 3. lock_t 구조체
~~~
    1. lock_t 구조체는 기본적으로 쓰레드가 작업을 위해 얻어내야 한다. lock_t를 얻어야 작업의 실행을 보장할 수 있다.
    2. 하지만 이 lock_t는 하나의 레코드에 대해서 한 쓰레드만 접근이 가능해야 한다.
    3. 따라서 lock_t 내부에는 한 레코드의 대해서 한개만 접근이 가능하게 만드는 변수가 필요하다.
    4. pthread_cond 밸류를 lock_t 구조체별로 선언해주고 이것이 깨어날 조건을 만들기 위한 변수 flag역시 같이 만든다.

~~~

> # 4. lock_t acquire
~~~
    1. 먼저 키값과 레코드값에 대해 hash_table에서 탐색해준다. 만약 없을 경우 hash_table에 레코드를 삽입한다.
    2. 그리고 hash_entry에서 가장 맨 처음 접근한 쓰레드인지 아닌지를 체크해줘서 그에 맞게 링크드리스트에 대입해준다.
    3. 첫번째 쓰레드가 아닐 경우 flag변수가 0으로 설정 되있고, cond_wait을 통해 잠재워준다.
    4. 첫번째 쓰레드의 lock_t를 return 해준다.
~~~

> # 5. lock_t release
~~~
    1. lock_t의 후임 쓰레드가 있는지 체크해준다.
    2. 후임 쓰레드가 있을 경우, 후임 쓰레드 lock_t의 flag값을 1로바꿔주고, 후임 쓰레드를 깨워준다.
    3. 그리고 인자 lock_t의 후임 쓰레드의 lock_t와 head를 연결해준다.
    4. 인자 lock_t를 할당해제해준다.
~~~
