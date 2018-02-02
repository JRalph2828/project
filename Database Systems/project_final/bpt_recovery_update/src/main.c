#include "bpt.h"
#include "pages.h"

int main(void) {

    int table_id;
    int table_id2;
    int size;
    int64_t input;
    char instruction;
    char buf[120];
    char path[120];
setvbuf(stdin, NULL, _IONBF, 0);
setvbuf(stdout, NULL, _IONBF, 0);
 //   print_file();
//    init_db(20);
//    open_table("test.db");
//    printf("> ");
    while(scanf("%c", &instruction) != EOF){
        switch(instruction){
            case 'a':
                abort_transaction();
                printf("abort done!\n");
                break;
            case 'b':
                begin_transaction();
                break;
            case 'c':
                scanf("%d", &table_id);
                close_table(table_id);
                break;
            case 'd':
                scanf("%d %ld", &table_id, &input);
                delete(table_id, input);
                break;
            case 'i':
                scanf("%d %ld %s", &table_id, &input, buf);
                insert(table_id, input, buf);
                break;
            case 'f':
                scanf("%d %ld", &table_id, &input);
                if(find(table_id, input) != NULL){
                    printf("Key: %ld, Value: %s\n", input, BUF);
                    fflush(stdout);
                }
                else{
                    printf("Not Exists\n");
                    fflush(stdout);
                }
                break;
            case 'j':
                scanf("%d %d %s", &table_id, &table_id2, path);
                join_table(table_id, table_id2, path);
                fprintf(stderr, "join done!\n");
                printf("\n");
                break;
            case 'k':
                print_log_file();
                break;
            case 'l':
                scanf("%d", &table_id);
                print_file(table_id);
                break;
            case 'm':
                commit_transaction();
                printf("commit done!\n");
                break;
            case 'n':
                scanf("%d", &size);
                init_db(size);
                break;
            case 'o':
                scanf("%s", path);
                table_id = open_table(path);
                printf("%d\n", table_id);
                break;
            case 'p':
                print_buffer();
                print_log_buffer();
                break;
            case 'q':
                shutdown_db();
                while(getchar() != (int64_t)'\n');
                return 0;
                break;
            case 'u':
                scanf("%d %ld %s", &table_id, &input, buf);
                update(table_id, input, buf);
                break;
            case 'x':
                exit(0);
                break;
        }
        while(getchar() != (int)'\n');
//                printf("> ");
    }
    printf("\n");

    return 0;
}
