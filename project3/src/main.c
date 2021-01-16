#include "bpt.h"
#include <time.h>
int main(int argc, char** argv) {
    char* input_file;
    int input, range2;
    char instruction;
    char license_part;
    char buf[120];
    memset(buf, 0, 120);
    char buf2[120];
    char pathname[120];
int nums[7000];
int num;
int j=0;
    srand(time(NULL));
    if (head == NULL) {
        init_db(1000);
    }
    printf("path name> ");
    scanf("%s", pathname);
    int id = open_table(pathname);
    for (int i = 0; i < 20000; i++) {
        sprintf(buf, "%d", i);
        if (!db_insert(id, i, buf))printf("i:%d\n", i);
        else printf("i failed\n");
    }
	for(int i=0;i<7000;i++){
		num=rand()%20000;
	nums[j]=num;
	j++;
	if (!db_delete(id, num))printf("d:%d\n", num);
        else printf("d failed\n");
}
int check=0;
for(int i=0;i<20000;i++){
	for(int j=0;j<7000;j++){
		if(nums[j]==i)check=1;
}
	if(!db_find(id,i,buf))printf("f :%s\n",buf);
	else{
		if(check==1);
		else printf("d error %d\n",i);
}
check=0;
}
    for (int i = 0; i < 20000; i++) {
        sprintf(buf, "%d", i);
        if (!db_insert(id, i, buf))printf("i:%d\n", i);
        else printf("i failed\n");
    }
for(int i=0;i<20000;i++){
	for(int j=0;j<7000;j++){
		if(nums[j]==i)check=1;
}
	if(!db_find(id,i,buf))printf("f :%s\n",buf);
	else{
		if(check==1);
		else printf("d error %d\n",i);
}
check=0;
}
/*scanf("%s", pathname);
id = open_table(pathname);
    for (int i = 30; i < 50; i++) {
        sprintf(buf, "%d", i);
        if (!db_insert(id, i, buf))printf("i:%s\n", buf);
        else printf("failed\n");
    }*/
    /*scanf("%s", pathname);
    int id2 = open_table(pathname);
    for (int i = 0; i < 10; i++) {
        sprintf(buf, "%d", i);
        if (!db_insert(id2, i, buf))printf("%d\n", i);
        else printf("failed\n");
    }*/
    /*for (int i = 0; i < 3; i++) {
        sprintf(buf, "%d", i);
        if (!db_delete(id2, i))printf("%d\n", i);
        else printf("failed\n");
    }
    for (int i = 7; i < 9; i++) {
        sprintf(buf, "%d", i);
        if (!db_delete(id2, i))printf("%d\n", i);
        else printf("failed\n");
    }*/
    printf(">");
    while (getchar() != (int)'\n');
    while (scanf("%c", &instruction) != EOF) {
        switch (instruction) {
            case 'd':
                scanf("%d %d",&id, &input);
                if (db_delete(id,input))
                    printf("invalid input\n");
                print_tree(id);
                break;
        case 'i':
            scanf("%d %d %s",&id, &input, buf);
            if (db_insert(id, input, buf))
                printf("duplicated value\n");
            break;
	case 'm':
	            scanf("%s", pathname);
            id=open_table(pathname);
	    printf("%d %s\n",id,pathname);
	break;
        case 'f':
	j=0;
            scanf("%s", pathname);
            id=open_table(pathname);
	    printf("%d %s\n",id,pathname);
    for (int i = 0; i < 20000; i++) {
        sprintf(buf, "%d", i);
        if (!db_insert(id, i, buf))printf("i:%d\n", i);
        else printf("i failed\n");
    }
	for(int i=0;i<7000;i++){
		num=rand()%20000;
	nums[j]=num;
	j++;
	if (!db_delete(id, num))printf("d:%d\n", num);
        else printf("d failed\n");
}
int check=0;
for(int i=0;i<20000;i++){
	for(int j=0;j<7000;j++){
		if(nums[j]==i)check=1;
}
	if(!db_find(id,i,buf))printf("f :%s\n",buf);
	else{
		if(check==1);
		else printf("d error %d\n",i);
}
check=0;
}
    for (int i = 0; i < 20000; i++) {
        sprintf(buf, "%d", i);
        if (!db_insert(id, i, buf))printf("i:%d\n", i);
        else printf("i failed\n");
    }
for(int i=0;i<20000;i++){
	for(int j=0;j<7000;j++){
		if(nums[j]==i)check=1;
}
	if(!db_find(id,i,buf))printf("f :%s\n",buf);
	else{
		if(check==1);
		else printf("d error %d\n",i);
}
check=0;
}
            break;
        case 'p':
            scanf("%d",&id);
	for(int i=0;i<20000;i++){
	for(int j=0;j<7000;j++){
		if(nums[j]==i)check=1;
}
	if(!db_find(id,i,buf))printf("f :%s\n",buf);
	else{
		if(check==1);
		else printf("d error %d\n",i);
}}
            /*if (!db_find(id, input, buf))
                printf("%s\n", buf);
            else {
                printf("not exist\n");
            }*/
            break;
        case 'c':
		scanf("%d",&id);
		close_table(id);
            break;
        case 's':
            shutdown_db();
		    if (head == NULL) {
        init_db(1000);
    }
            break;
        case 'q':
            while (getchar() != (int)'\n');
            return EXIT_SUCCESS;
            break;
        case 't':
	     scanf("%d",&id);
            print_tree(id);
            break;
        default:
            break;
        }
        while (getchar() != (int)'\n');
        printf("> ");
    }
    printf("\n");
    return 0;
}
