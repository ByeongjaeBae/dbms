#include "bpt.h"
#include "time.h"
int main(int argc, char** argv) {
    char* input_file;
    int input, range2;
    char instruction;
    char license_part;
    char buf[120];
    srand((unsigned int)time(NULL));
    char pathname[120];
    int nums[7000];
int num;
int j=0;
    printf("path name:> ");
    scanf("%s", pathname);
    open_table(pathname);
    for (int i = 0; i < 20000; i++) {
        sprintf(buf, "%d", i);
        if (!db_insert(i, buf))printf("i:%d\n", i);
        else printf("i failed\n");
    }
	for(int i=0;i<7000;i++){
		num=rand()%20000;
	nums[j]=num;
	j++;
	if (!db_delete(num))printf("d:%d\n", num);
        else printf("d failed\n");
}
int check=0;
for(int i=0;i<20000;i++){
	for(int j=0;j<7000;j++){
		if(nums[j]==i)check=1;
}
	if(!db_find(i,buf))printf("f :%s\n",buf);
	else{
		if(check==1);
		else printf("d error %d\n",i);
}
check=0;
}
    for (int i = 0; i < 20000; i++) {
        sprintf(buf, "%d", i);
        if (!db_insert(i, buf))printf("i:%d\n", i);
        else printf("i failed\n");
    }
for(int i=0;i<20000;i++){
	for(int j=0;j<7000;j++){
		if(nums[j]==i)check=1;
}
	if(!db_find(i,buf))printf("f :%s\n",buf);
	else{
		if(check==1);
		else printf("d error %d\n",i);
}
check=0;
}
    printf("> ");
    while (getchar() != (int)'\n');
    while (scanf("%c", &instruction) != EOF) {
        switch (instruction) {
        case 'd':
            scanf("%d", &input);
            if (db_delete(input))
                printf("invalid input\n");
            print_tree();
            break;
        case 'i':
            scanf("%d %s", &input, buf);
            if(db_insert(input, buf))
		printf("duplicated value\n");
            print_tree();
            break;
        case 'f':
            scanf("%s", pathname);
            open_table(pathname);
            break;
        case 'p':
            scanf("%d", &input);
            if (!db_find(input, buf))
                printf("%s\n", buf);
            else {
                printf("input is not exist:\n");
            }
            break;
        case 'r':
            /*scanf("%d %d", &input, &range2);
            if (input > range2) {
                int tmp = range2;
                range2 = input;
                input = tmp;
            }
            find_and_print_range(root, input, range2, instruction == 'p');*/
            break;
        case 'l':
            print_leaves();
            break;
        case 'q':
            while (getchar() != (int)'\n');
            return EXIT_SUCCESS;
            break;
        case 't':
            print_tree();
            break;
        default:
            usage_2();
            break;
        }
        while (getchar() != (int)'\n');
        printf("> ");
    }
    printf("\n");
    free(header_page);
    return EXIT_SUCCESS;
}
