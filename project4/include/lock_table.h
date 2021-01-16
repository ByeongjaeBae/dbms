#ifndef __LOCK_TABLE_H__
#define __LOCK_TABLE_H__

#include <stdint.h>
#include <pthread.h>
#include <unordered_map>
using namespace std;
typedef struct lock_t lock_t;

typedef struct hash_entry hash_entry;
/* APIs for lock table */
int hashcode(int table_id,int record_id);
int init_lock_table();
lock_t* lock_acquire(int table_id, int64_t key);
int lock_release(lock_t* lock_obj);

#endif /* __LOCK_TABLE_H__ */
