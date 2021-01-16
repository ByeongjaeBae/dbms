#ifndef __LOCK_TABLE_H__
#define __LOCK_TABLE_H__
#include <stdint.h>
#include <pthread.h>
#include <unordered_map>
#include <vector>
using namespace std;
#define SHARED 0
#define EXCLUSIVE 1
#define CYCLE 1
#define NO_CYCLE 0
typedef struct lock_t {
	int key;
	int flag;
	pthread_cond_t cond;
	struct lock_t* next;
	struct lock_t* prev;
	struct hash_entry* sentinel;
	int lock_mode;
    int trx_id;
	int deadlock;
    struct lock_t* next_trx_ptr;
	struct lock_t* prev_trx_ptr;
	int exclusive;
}lock_t;

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
extern unordered_map<int64_t,hash_enrty>* hash_t; 
extern pthread_mutex_t lock_table_latch;
/* APIs for lock table */
int hashcode(int table_id,int record_id);
int init_lock_table();
lock_t* lock_acquire(int table_id, int64_t key,int trx_id,int lock_mode);
int lock_release(lock_t* lock_obj);

#endif /* __LOCK_TABLE_H__ */
