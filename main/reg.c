#include <stdio.h>
#include "regex.h"

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: reg <pattern> <string>\n");
    }
    
    const char *match = re_is_match(argv[1], argv[2]);
    
    printf("A match was%sfound\n", match ? " " : " not ");

    if (match) {
        printf("\nMatch: %s\n", match);
    }
    return 0;
}
