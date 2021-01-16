#ifndef __TRX_H__
#define __TRX_H__
#include "lock_table.h"
extern int global_trx_id;
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
    int num;
}trx;
extern trx* trx_head;
int trx_begin();
int trx_commit(int trx_id);
int trx_abort(int trx_id);
trx* find_trx(int trx_id);
int trx_erase(int trx_id);
#endif
