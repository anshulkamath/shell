#include <stdio.h>
#include "regex.h"

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: reg <pattern> <string>\n");
    }
    
    printf("A match was%sfound\n", re_is_match(argv[1], argv[2]) ? " " : " not ");
    return 0;
}
