# Concurrency Control

이번 과제는 Concurrency Control을 위한 Transaction과 Lock Manager 구현을 목표로 한다.

## 1, Definition

### 1.1. Transaction

Transaction (이하 TRX) 은 User Defined Instruction Sequence로, ACID의 속성을 가진다.

- Atomicity: 전체 TRX이 실행에 성공하거나, TRX 자체가 실행되지 않는다.
- Consistency: 읽고 쓰는 값의 일관성이 보장된다.
- Isolation: 사용자가 다른 사용자로부터 시스템이 독립되었다 느낀다.
- Durability: 실행에 성공한 Transaction에 대한 결과가 영속적으로 보존된다.

#### 1.1.1. Consistency

Consistency은 주어진 값을 여러 TRX이 수정하고자 할 때 위협을 받으며, 이때 일어날 수 있는 문제는 크게 3가지로 나뉜다.

1. lost update: 동시에 두 write operation이 발생하면, 하나의 write operation이 손실될 수 있다.
2. inconsistent read: 수정될 값을 읽을 때 하나의 개체에 대해 읽어온 두 값이 다를 수 있다.
3. dirty read: 아직 confirm 되지 않은 임시 값을 읽을 때 Undo 과정에서 값이 손실될 수 있다.

#### 1.1.2. Isolation

동시에 여러 트랜잭션이 처리될 때, 특정 트랜잭션이 다른 트랜잭션에서 변경하거나 조회하는 데이터를 볼 수 있도록 허용할지 말지를 결정하는 것.

### 1.2. Lock Manager

TRX의 ACID 속성을 보장하기 위해 시스템 전체에 대한 접근을 Lock을 통해 제어하는 장치이다.

#### 1.2.1. Accessibility

Lock은 TRX이 Database, Table, Page, Record 등 대상 개체에 접근하는 것을 제한하며, TRX이 요청한 권한과 대상 개체에 접근 중인 TRX에서 접근 안전성을 고려하여 대기 혹은 접근의 결정을 내린다.

권한은 크게 Shared와 Exclusive로 나뉘며, 개체를 수정하지 않아 다른 TRX과 접근을 공유할 수 있는 경우 Shared, 개체에 대한 추가 처리가 이루어져 다른 TRX과 접근을 공유할 수 없는 경우 Exclusive 권한을 요청하게 된다.

Shared 권한을 요청한 TRX은 서로 접근을 공유할 수 있으며, Exclusive 권한을 요청한 경우 다른 TRX이 접근할 수 없다. Lock은 이러한 권한과 접근 상태를 통해 접근 안전성을 고려한다.

#### 1.2.2. Deadlock

서로 다른 TRX이 상대방이 소유한 개체에 대한 접근을 기다릴 때 두 TRX은 무한정 서로를 기다리게 되는데, 이를 Deadlock이라고 한다. Lock Manager은 주기적으로 혹은 매번 Lock을 처리할 때마다 TRX 사이에 Lock Cycle이 존재하는지 확인하고, 존재할 경우 Cycle을 풀기 위해 Abort 할 TRX을 지정하여 실행을 취소해야 한다.

## 2. Design

### 2.1. Lock Structure

```c++
typedef struct lock_t {
	int key;
	int flag;
	pthread_cond_t cond;
	struct lock_t* next;
	struct lock_t* prev;
	struct hash_entry* sentinel;
	int lock_mode;
        int trx_id;
        struct lock_t* next_trx_ptr;
	struct lock_t* prev_trx_ptr;
}lock_t;

```

Lock의 기본적인 구조에 s,x모드의 구분을 위한 lock_mode와 트랜잭션 구분을 위한 trx_id 그리고 트랜잭션의 락을 확인하기 위한 next_trx_ptr등이 있다.

```c++
typedef struct hash_entry{
	int num;
	int f_trx_id;
	int check;
	int exclusive;
	int table_id;
	int64_t key;
	lock_t* head;
	lock_t* tail;
}hash_enrty;

```
hash_entry는 공들인 부분으로, 레코드의 전반적인 locking을 관리하고 있다. 하나의 레코드에 하나의 트랜잭션만 접근할 시, wait을 안하게 하기 위해, int check와 acquired 된 락 중에 x모드가 있는지 체크하기 위한 exclusive등을 활용한다.

### 2.2 Transaction structure

```c++
typedef struct prev_record{
    int table_id;
    int64_t key;
    char value[120];
}prev_record;
typedef struct trx{
    int trx_id;
    struct trx* trx_next;
    lock_t* lock_next;
    vector<prev_record>record;
    int deadlock;
}trx;

```
트랜잭션은 생성과 즉시 TRX ID를 부여받으며, Operation 중에는 Lock Manager와 통신하며 작업에 필요한 Lock을 획득, 사용자가 trx_commit() 메소드를 호출하기 전까지 획득한 lock을 보존하는 역할을 한다. 이 과정에서 trx_commit()까지 정상 호출되면 lock을 release하고, 그 전에 deadlock이 발생한 경우 abort 하게 된다.
이때, prev_record는 트랜잭션이 abort될 때, undo를 위한 구조체로, update를 할 때, 이전의 레코드 내용을 저장해 abort될 때, 원래대로 돌려준다.

### 2.3. Deadlock detection

Deadlock detection은 Lock의 prev 포인터와 Transaction이 가지고 있는 lock list를 통해 가능하다.

하나의 TRX을 선택하고, 소유하고 있는 lock을 순회한다. 해당 lock이 가리키는 page ID의 lock 리스트에서 그 lock이 wait하고 있을 시, 그 lock이 현재 기다리고 있는 lock 뿐만 아니라 향후 기다리게 될 lock까지 전부 체크해서 그래프를 생성해준다.이는 Graph 탐색 문제이므로 dfs를 통해 사이클을 탐색해준다.

### 2.4. Buffer Pin and page_latch

Buffer에서 사용하던 pin을 mutex인 page_latch로 수정해준다. page_latch를 잡기 위해서는 buffer_latch를 잡아야 하고, 레코드 락을 잡기전에 page_latch를 풀어줘야 데드락 방지가 가능하다. 레코드 락을 잡아주고는 다시 page_latch를 잡아준다.

### 2.5. Buffer Manager

Buffer manager에 대해서 쓰레드의 접근을 제한하기 위한 buffer_latch를 활성화 해준다. 이때, LRU_list의 수정은 버퍼가 페이지를 얻어올때, 한번 실시해준다. buffer_latch->page_latch의 구조를 지키기 위해 이러한 디자인이 필요하다. 이렇게 하지 않을 경우 데드락이 발생할 수 있어 이러한 디자인을 고려하였다. 또한 버퍼풀이 꽉차있고, 모든 버퍼페이지를 사용중이여서 원하는 페이지를 가져올 수 없을때는 기다리지 않고 NULL을 리턴하도록 핸들링하였다.

### 2.6. Strict 2-Phase-Lock

2PL 구현을 위해서 추후 TRX Commit 혹은 Abort후에 Lock을 Release 하는 방식으로 구현하였다.

