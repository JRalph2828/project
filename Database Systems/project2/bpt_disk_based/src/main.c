#include "bpt.h"
#include "pages.h"

int main(void) {

    int64_t input;
    char instruction;
    char buf[120];

    open_db("test.db");
 //   print_file();

//    printf("> ");
    while(scanf("%c", &instruction) != EOF){
        memset(BUF, 0, 120);
        switch(instruction){
            case 'd':
                scanf("%ld", &input);
                delete(input);
                break;
            case 'i':
                scanf("%ld %s", &input, buf);
                insert(input, buf);
                break;
            case 'f':
                scanf("%ld", &input);
                if(find(input) != NULL){
                    printf("Key: %ld, Value: %s\n", input, BUF);
                    fflush(stdout);
                }
                else{
                    printf("Not Exists\n");
                    fflush(stdout);
                }
                break;
            case 'l':
                print_file();
                break;
            case 'q':
                while(getchar() != (int64_t)'\n');
                return 0;
                break;
            case 'p':
                print_tree();
                break;
        }
        while(getchar() != (int)'\n');
        //        printf("> ");
    }
    printf("\n");

    return 0;
}
