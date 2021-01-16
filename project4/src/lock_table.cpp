#include "lock_table.h"
#include<assert.h>
using namespace std;
typedef struct lock_t {
	int flag;
	pthread_cond_t cond;
	lock_t* next;
	lock_t* prev;
}lock_t;

typedef struct hash_entry{
	int table_id;
	int64_t key;
	lock_t* head;
	lock_t* tail;
}hash_enrty;
unordered_map<int64_t,hash_enrty>* hash_t; 
pthread_mutex_t lock_table_latch;
int hashcode(int table_id,int record_id){
	return (table_id<<16)+record_id;
}
int
init_lock_table()
{
	hash_t=new unordered_map<int64_t,hash_enrty>;
	lock_table_latch=PTHREAD_MUTEX_INITIALIZER;
	return 0;
}

lock_t*
lock_acquire(int table_id, int64_t key)
{
	pthread_mutex_lock(&lock_table_latch);
	//printf("lock thread: %ld\n",pthread_self());
	int hash_code=hashcode(table_id,key);
	if(auto itr=hash_t->find(hash_code)==hash_t->end()){
		//printf("lock_hash not found %d %d\n",table_id,key);
		hash_entry he;
		he.table_id=table_id;
		he.key=key;
		he.head=(lock_t*)malloc(sizeof(lock_t));
		he.tail=(lock_t*)malloc(sizeof(lock_t));
		he.head->next=NULL;
		he.tail->next=NULL;
		hash_t->insert(unordered_map<int64_t,hash_entry>::value_type(hash_code,he));
	}
	auto itr=hash_t->find(hash_code);
	lock_t* lock=(lock_t*)malloc(sizeof(lock_t));
	lock->next=NULL;
	lock->prev=NULL;
	lock->cond=PTHREAD_COND_INITIALIZER;
	if(itr->second.head->next!=NULL){
		//printf("lock next %d %d\n",table_id,key);
		itr->second.tail->next->next=lock;
		lock->prev=itr->second.tail->next;
		itr->second.tail->next=lock;
		lock->flag=0;
	}
	else{
		//printf("lock first %d %d\n",table_id,key);
		itr->second.head->next=lock;
		itr->second.tail->next=lock;
		lock->prev=itr->second.head;
		lock->flag=1;
	}
	while(lock->flag==0){
		//printf("wait thread: %ld key: %d %d\n",pthread_self(),table_id,key);
		pthread_cond_wait(&lock->cond,&lock_table_latch);
		//printf("awake thread: %ld key: %d %d\n",pthread_self(),table_id,key);
		//printf("awake flag: %d\n",lock->flag);
	}
	pthread_mutex_unlock(&lock_table_latch);
	return lock;
}

int
lock_release(lock_t* lock_obj)
{
	pthread_mutex_lock(&lock_table_latch);
	if(lock_obj->flag!=1){
		printf("flag error\n");
	}
	//printf("rel\n");
	lock_obj->prev->next=lock_obj->next;
	if(lock_obj->next!=NULL){
		lock_obj->next->prev=lock_obj->prev;
		lock_obj->next->flag=1;
		//printf("release next key: %d %d\n",lock_obj->next->table_id,lock_obj->next->key);
		pthread_cond_signal(&lock_obj->next->cond);
	}
	//printf("release key: %d %d\n",lock_obj->table_id,lock_obj->key);
	free(lock_obj);
	//printf("release\n");
	pthread_mutex_unlock(&lock_table_latch);
	return 0;
}
