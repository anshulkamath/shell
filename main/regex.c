#include <stdio.h>
#include <stdlib.h>
#include "regex.h"

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: reg <pattern> <string>\n");
    }
    
    char *match = re_get_match(argv[1], argv[2]);
    
    printf("A match was%sfound\n", match ? " " : " not ");

    if (match)
        printf("\nMatch: %s\n", match);

    free(match);
    return 0;
}
