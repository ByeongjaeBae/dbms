#include "trx_manager.h"
#include "bpt.h"
int global_trx_id=1;
int trx_init=0;
trx* trx_head;
trx* trx_tail;
vector<prev_record>v;
pthread_mutex_t trx_manager=PTHREAD_MUTEX_INITIALIZER;
int trx_begin(){
    //printf("trx_beign\n");
    pthread_mutex_lock(&trx_manager);
    //printf("trx_beign2\n");
    if(trx_init==0){
        trx_head=(trx*)malloc(sizeof(trx));
        trx_head->trx_next=NULL;
        trx_head->lock_next=NULL;
        trx_init=1;
    }
    trx* new_trx=new trx;
    new_trx->trx_next=NULL;
    new_trx->lock_next=NULL;
 new_trx->num=0;
    if(new_trx->record.size()!=0){
        printf("size error\n");
        new_trx->record=v;
    }
   // printf("vector trx_id %d %d\n",global_trx_id,new_trx->record.size());
    if(new_trx==NULL){
        printf("memory error\n");
        return 0;
    }
    new_trx->trx_id=global_trx_id;
    if(trx_head->trx_next==NULL){
        trx_head->trx_next=new_trx;
    }
    else{

        new_trx->trx_next=trx_head->trx_next;
        trx_head->trx_next=new_trx;
    }
    new_trx->lock_next=NULL;
    pthread_mutex_unlock(&trx_manager);
    return global_trx_id++;
}
int trx_commit(int trx_id){
    pthread_mutex_lock(&trx_manager);
   // printf("trx_commit1\n");
   // printf("trx_commit2\n");
    trx* trx_find;
    trx_find=find_trx(trx_id);
    if(trx_find==NULL){
        pthread_mutex_unlock(&trx_manager);
        return 0;
    }
    //printf("trx_commit3\n");
    lock_t* lock;
    lock_t* lock_next=trx_find->lock_next;
    while(lock_next!=NULL){
        //printf("lock_next %d %d %d\n",lock_next->sentinel->key,lock_next->num,lock_next->trx_id);
        lock=lock_next;
        lock_next=lock->next_trx_ptr;
        lock_release(lock);
    }
   // printf("trx_commit4\n");
    trx_erase(trx_id);
   // printf("trx_commit5\n");
    pthread_mutex_unlock(&trx_manager);
    return trx_id;
}
int trx_erase(int trx_id){
    //printf("erase\n");
    trx* find=trx_head;
    trx* prev=trx_head;
    while((find=find->trx_next)->trx_id!=trx_id){
        prev=prev->trx_next;
        if(find==NULL){
            printf("erase_invalid trx_id\n");
            return -1;
        }
    }
    prev->trx_next=find->trx_next;
    find->num=1;
    return 0;
}
int trx_abort(int trx_id){
    //printf("abort 1 %d\n",trx_id);
    //return 0;
    pthread_mutex_lock(&trx_manager);
    //printf("abort 2 %d\n",trx_id);
    int table_id;
    int64_t key;
    prev_record record;
    trx* trx_find=find_trx(trx_id);
   // printf("a_check1\n");
    for(int j=0;j<trx_find->record.size();j++){
        //printf("%d a_check2\n",trx_find->record.size());
        record=trx_find->record[j];
        //printf("a_check1\n");
        table_id=record.table_id;
        key=record.key;
        int i = 0;
        buffer* buf;
        page_t* page;
        pagenum_t leaf_num = find_leaf_trx(table_id, key,trx_id);
        buf = get_frame(table_id, leaf_num);
        page = buf->page;
        record_t* ret = (record_t*)malloc(sizeof(record_t));
        for (i = 0; i < ((leaf_page_t*)page)->Number_of_Keys; i++)
            if (((leaf_page_t*)page)->record[i].key == key) break;
        if (i == ((leaf_page_t*)page)->Number_of_Keys) {
            put_frame(buf, 1);
            //printf("trx_abort error\n");
        }
        else {
            page=buf->page;
            strncpy(((leaf_page_t*)page)->record[i].value,record.value,120);
            put_frame(buf,1);
        }
    }
    //printf("abort2\n");
   lock_t* lock;
    lock_t* lock_next=trx_find->lock_next;
    while(lock_next!=NULL){
        //printf("lock_next %d %d %d\n",lock_next->sentinel->key,lock_next->num,lock_next->trx_id);
        lock=lock_next;
        lock_next=lock->next_trx_ptr;
        lock_release(lock);
    }
    //printf("abort3 %d\n",trx_id);
    if(trx_erase(trx_id)==-1){
        pthread_mutex_unlock(&trx_manager);
       // printf("trx_erase error\n");
        return -1;
    }
    pthread_mutex_unlock(&trx_manager);
    return 0;
}
trx* find_trx(int trx_id){
   // printf("trx_find1\n");
    int check=0;
    if(pthread_mutex_trylock(&trx_manager)!=0)check=1;
    trx* trx_find;
    trx_find=trx_head->trx_next;
    if(trx_find==NULL){
        if(check==0)pthread_mutex_unlock(&trx_manager);
        return NULL;
    }
    while(trx_find->trx_id!=trx_id){
        //printf("find error\n");
        trx_find=trx_find->trx_next;
        if(trx_find==NULL){
            //printf("invalid trx_id %d\n",trx_id);
            if(check==0)pthread_mutex_unlock(&trx_manager);
            return NULL;
            }
    }
if(trx_find->num==1){
	printf("invalid trx_id %d\n",trx_id);
	return NULL;
}
   // printf("trx_find2\n");
    if(check==0)pthread_mutex_unlock(&trx_manager);
    return trx_find;
}
