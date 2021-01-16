#include "lock_table.h"
#include "trx_manager.h"
using namespace std;
unordered_map<int64_t,hash_enrty>* hash_t; 
pthread_mutex_t lock_table_latch;
vector<int> trx_table;
int gl=1;
int init=0;
int graph[20001][20001];
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
int find_cycle(int* visited,int graph[][20001],int* recur,int num,int start){
	if(visited[start])return 1;
	if(recur[start])return 0;
	visited[start]=1;
	recur[start]=1;
	for(int i=0;i<num;i++){
		if(graph[start][i]==1){
			if(find_cycle(visited,graph,recur,num,i))return 1;
		}
	}
	visited[start]=0;
	return 0;
}
lock_t*
lock_acquire(int table_id, int64_t key,int trx_id,int lock_mode)
{
	//printf("1\n");
	pthread_mutex_lock(&lock_table_latch);
	if(init==0){
		init_lock_table();
		init=1;
	}
	//printf("2\n");
	int hash_code=hashcode(table_id,key);
	if(auto itr=hash_t->find(hash_code)==hash_t->end()){
		//printf("lock_hash not found %d %d\n",table_id,key);
		hash_entry he;
		he.table_id=table_id;
		he.key=key;
		he.head=(lock_t*)malloc(sizeof(lock_t));
		he.tail=(lock_t*)malloc(sizeof(lock_t));
		he.head->next=NULL;
		he.head->prev=NULL;
		he.tail->next=NULL;
		he.check=0;
		he.exclusive=0;
		he.f_trx_id=0;
		he.num=0;
		hash_t->insert(unordered_map<int64_t,hash_entry>::value_type(hash_code,he));
	}
	auto itr=hash_t->find(hash_code);
	lock_t* lock=(lock_t*)malloc(sizeof(lock_t));
	lock->key=key;
	lock->next=NULL;
	lock->prev=NULL;
	lock->cond=PTHREAD_COND_INITIALIZER;
	lock->lock_mode=lock_mode;
	lock->trx_id=trx_id;
	lock->next_trx_ptr=NULL;
	lock->deadlock=0;
	lock->flag=0;
	lock->exclusive=0;
	//printf("2\n");
	//printf("f_trx_id:%d\n",itr->second.f_trx_id);
	if(itr->second.num!=0){
		lock_t* l=itr->second.head->next;
		//printf("%d,%d,%d,%d->\n",l->trx_id,l->lock_mode,l->flag,l->key);
		///printf("back end\n");
		//printf("lock next %d %d\n",table_id,key);
		//printf("%d,%d,%d\n",lock->trx_id,lock->lock_mode,lock->flag);
		/*lock_t* copy=itr->second.head->next;
		while(copy!=NULL){
			printf("%d,%d,%d->",copy->trx_id,copy->lock_mode,copy->flag);
			copy=copy->next;
		}*/
		//printf("\n");
		//printf("tail->next %d %d %d\n",itr->second.tail->next->trx_id,itr->second.tail->next->lock_mode,itr->second.tail->next->flag);
		(lock->sentinel)=&(itr->second);
		int f_trx_id=lock->sentinel->f_trx_id;
		lock_t* copy=lock->sentinel->head->next;
		lock->sentinel->check=0;
		lock->sentinel->exclusive=0;
		while(copy!=NULL){
			if(copy->flag==1&&copy->trx_id==lock->trx_id&&copy->lock_mode==EXCLUSIVE){
				pthread_mutex_unlock(&lock_table_latch);
				return lock;	
			}
			if(copy->flag==1&&copy->trx_id==lock->trx_id&&copy->lock_mode==SHARED&&lock_mode==SHARED){
				pthread_mutex_unlock(&lock_table_latch);
				return lock;	
			}
			if(copy->lock_mode==EXCLUSIVE)copy->sentinel->exclusive=1;
			if(f_trx_id!=copy->trx_id){
				lock->sentinel->check=1;
				break;
			}
			copy=copy->next;
		}
		if(f_trx_id!=lock->trx_id)lock->sentinel->check=1;
		itr->second.tail->next->next=lock;
		lock->prev=itr->second.tail->next;
		itr->second.tail->next=lock;
		//printf("check1\n");
		copy=lock->sentinel->head->next;
		/*while(copy!=NULL){
			printf("%d,%d,%d->",copy->trx_id,copy->lock_mode,copy->flag);
			copy=copy->next;
		}*/
		//rintf("\n");
		//printf("check=%d\n",lock->sentinel->check);
		if((lock->prev->lock_mode==SHARED&&lock->lock_mode==SHARED&&lock->prev->flag==1&&lock->sentinel->exclusive==0)
		||lock->sentinel->check==0){
			//printf("30\n");
			lock->flag=1;
		}
		else{
			lock->flag=0;
		}
	}
	else{
		//printf("4\n");
		//printf("lock first %d %d\n",table_id,key);
		itr->second.head->next=lock;
		itr->second.tail->next=lock;
		lock->prev=itr->second.head;
		(lock->sentinel)=&(itr->second);
		lock->flag=1;
		lock->sentinel->f_trx_id=trx_id;
		lock->sentinel->exclusive=0;
		lock->sentinel->check=0;
	}
	itr->second.num++;
		/*lock_t* copy=lock->sentinel->head->next;
		while(copy!=NULL){
			printf("%d,%d,%d,%d->",copy->trx_id,copy->lock_mode,copy->flag,copy->key);
			copy=copy->next;
		}
		printf("\n");*/
	//trx_manager connected
	trx* trx_find=find_trx(trx_id);
	lock_t* trx_lock=trx_find->lock_next;
	lock_t* next_trx_lock=trx_lock;
	if(trx_lock==NULL){
		trx_find->lock_next=lock;
		}
	else{
			lock->next_trx_ptr=trx_find->lock_next;
			trx_find->lock_next=lock;
	}
	/*while(next_lock!=NULL){
		printf("next_lock %d %d %d\n",next_lock->sentinel->key,next_lock->num,next_lock->trx_id);
		next_lock=next_lock->next_trx_ptr;
	}*/
	//printf("5\n");
	//trx_find=find_trx(trx_id);
	//trx_lock=trx_find->lock_next->sentinel->head->next;
	//printf("d %d\n",trx_lock->num);
			/*lock_t* copy=lock->sentinel->head->next;
			lock_t* f_copy=copy;
		while(copy!=NULL){
			printf("%d,%d,%d,%d->",copy->trx_id,copy->lock_mode,copy->flag,copy->key);
			if(f_copy->flag==0)exit(-1);
			copy=copy->next;
		}
		printf("\n");

		printf("\n");*/
	if(lock->flag==0){
		//printf("8\n");
			//construct graph
			int num=global_trx_id;
			for(int i=0;i<num;i++){
				for(int j=0;j<num;j++){
					graph[i][j]=0;
				}
			}
			trx* trx_finds;
			//printf("num:%d\n",num);
			int table_check=0;
			for(int i=1;i<num;i++){
				//printf("9 %d\n",i);
				for(int j=0;j<trx_table.size();j++){
					if(i==trx_table[j])table_check=1;
				}
				trx_finds=find_trx(i);
				if(table_check==0);
				else {
					table_check=0;
					continue;
				}
				if(trx_finds==NULL)continue;
				//printf("trx_id : %d\n",trx_finds->trx_id);
				lock_t* next_lock=trx_finds->lock_next;
				/*while(next_lock!=NULL){
					//printf("%d,%d,%d,%d->",next_lock->trx_id,next_lock->lock_mode,next_lock->flag,next_lock->key);
					next_lock=next_lock->next_trx_ptr;
				}*/
				//printf("\n");
				//next_lock=trx_finds->lock_next;
				while(next_lock!=NULL){
					int ex_check=0;
					lock_t* find_lock=next_lock;
					lock_t* head_lock=find_lock->sentinel->head;
					if(find_lock->flag==0){
						find_lock=find_lock->prev;
						while(find_lock!=head_lock){
							//printf("print1\n");
							//printf("%d,%d,%d %d->",find_lock->trx_id,find_lock->lock_mode,find_lock->flag,find_lock->key);
							//printf("%d\n",find_lock->num);
							if((find_lock->flag==1)||(next_lock->lock_mode==EXCLUSIVE)||(next_lock->lock_mode==SHARED&&ex_check==1)){
								graph[trx_finds->trx_id][find_lock->trx_id]=1;
							}
							if(find_lock->prev->lock_mode==EXCLUSIVE)ex_check=1;
							find_lock=find_lock->prev;
						}
					}
					ex_check=0;
					next_lock=next_lock->next_trx_ptr;
				}	
		}
		/*for(int i=1;i<num;i++){
			for(int j=1;j<num;j++){
				printf("%d ",graph[i][j]);
			}
			printf("\n");
		}
		printf("\n");*/
		int *visited=(int*)malloc(sizeof(int)*num);
		for(int i=0;i<num;i++){
			visited[i]=0;
		}
		int *recur=(int*)malloc(sizeof(int)*num);
		for(int i=0;i<num;i++){
			recur[i]=0;
		}		
		if(find_cycle(visited,graph,recur,num,trx_id)){
			trx_table.push_back(trx_id);
			//printf("trx %d cycle\n\n\n",trx_id);
			lock->deadlock=1;
			pthread_mutex_unlock(&lock_table_latch);
			return lock;
		}
		//printf("7\n");
	}
	while(lock->flag==0){
		//printf("wait thread: %ld key: %d id: %d\n\n\n",pthread_self(),key,trx_id);
		lock_t* copy=lock->sentinel->head->next;
		/*while(copy!=NULL){
			printf("%d,%d,%d %d->",copy->trx_id,copy->lock_mode,copy->flag,copy->key);
			copy=copy->next;
		}*/
		//printf("\n\n");
		//copy=lock->sentinel->tail->next;
		//printf("%d,%d,%d,%d->\n",copy->trx_id,copy->lock_mode,copy->sentinel->key,copy->flag);
		pthread_cond_wait(&lock->cond,&lock_table_latch);
		//printf("        awake thread: %ld key: %d id: %d\n\n\n",pthread_self(),key,trx_id);
		//printf("awake flag: %d\n",lock->flag);
	}
	pthread_mutex_unlock(&lock_table_latch);
	//printf("success\n");
	return lock;
}

int
lock_release(lock_t* lock_obj)
{
	if(pthread_mutex_trylock(&lock_table_latch)!=0){
		pthread_mutex_unlock(&lock_table_latch);
		pthread_mutex_lock(&lock_table_latch);
	}
	trx* trx_=find_trx(lock_obj->trx_id);
	lock_obj->prev->next=lock_obj->next;
	lock_obj->sentinel->exclusive=0;
	if(lock_obj->sentinel->tail->next==lock_obj){
		lock_obj->sentinel->tail->next=lock_obj->prev;
	}
	if(lock_obj->next==NULL)lock_obj->sentinel->f_trx_id=0;
	lock_t* next_lock;
	if(lock_obj->next!=NULL){
		lock_obj->next->prev=lock_obj->prev;
		if(lock_obj->next->prev==(lock_obj->sentinel->head)){
			next_lock=lock_obj->next;
			lock_obj->sentinel->f_trx_id=next_lock->trx_id;
			int f_trx_id=next_lock->trx_id;
			while(1){
				if(next_lock->lock_mode==EXCLUSIVE)next_lock->sentinel->exclusive=1;
				next_lock->flag=1;
				pthread_cond_signal(&next_lock->cond);
				if(next_lock->next==NULL||
				((next_lock->next->trx_id!=f_trx_id)&&(next_lock->next->lock_mode==EXCLUSIVE)||
				((next_lock->trx_id!=f_trx_id)&&(next_lock->sentinel->exclusive==1)))
				)break;
				else{
					next_lock=next_lock->next;
				}
			}
			}
		//printf("release next key: %d %d\n",lock_obj->next->table_id,lock_obj->next->key);
	}
	else{
		lock_obj->sentinel->f_trx_id=0;
	}
	//printf("release 1 %d\n",lock_obj->trx_id);
	trx* trx_find=find_trx(lock_obj->trx_id);
	lock_t* trx_lock=trx_find->lock_next;
	if(trx_lock==lock_obj){
		trx_find->lock_next=trx_lock->next_trx_ptr;
	}
	else{
		lock_t* next_trx_lock=trx_lock;
		while(next_trx_lock!=lock_obj){
			trx_lock=next_trx_lock;
			next_trx_lock=next_trx_lock->next_trx_ptr;
	}
		trx_lock->next_trx_ptr=next_trx_lock->next_trx_ptr;
	}
	lock_obj->sentinel->num--;
	//printf("lock_obj %d %d\n",lock_obj->sentinel->key,lock_obj->num);
	free(lock_obj);
	//printf("release 2 %d\n",lock_obj->trx_id);
	pthread_mutex_unlock(&lock_table_latch);
	//printf("release 3 %d\n",lock_obj->trx_id);
	return 0;
}
