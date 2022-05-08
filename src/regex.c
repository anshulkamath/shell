#include "regex.h"
#include "regex-private.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IS_METACHAR(x) \
    (x) == DOT || (x) == STAR || (x) == PLUS || (x) == OPTIONAL || (x) == BEGIN || (x) == END

/**
 * @brief checks a single character using a given class by checking if:
 * -------
 *  - we are not at the end of the string
 *  - the metacharacter matches against the character
 *  - the literal character matches
 *  - the literal character is in the character class
 * 
 * @param cl the class to check against
 * @param ch the character to match
 * @returns int
*/
static __attribute__((always_inline)) int check_char(const re_t *cl, char ch) {
    return  ch != '\0' && (
            (cl->type == DOT) || 
            (cl->type == CHAR && cl->class.c == ch) ||
            (cl->type == CHAR_CLASS && (cl->nccl ^ get_ind(cl->class.mask, ch)))
        );
}

/* takes in a string regexp and returns a list of `re_t`s representing the regexp */
re_t *re_compile(const char *regexp) {
    const size_t REGEXP_LEN = strlen(regexp);
    re_t *regex = calloc(REGEXP_LEN + 1, sizeof(re_t));

    // flag the last element in the array
    regex[REGEXP_LEN].type = TERMINAL;

    /* separate index variable since regexp may parse multiple characters at a time */
    size_t index = 0;

    for (size_t i = 0; i < REGEXP_LEN; i++) {
        // handle escaped sequence
        if (regexp[i] == ESCAPE) {
            regex[index].type = CHAR;

            // if the next character is a metacharacter, make it a character
            if (i + 1 < REGEXP_LEN && IS_METACHAR(regexp[i + 1]))
                regex[index].class.c = regexp[i + 1];
            else
                regex[index].class.c = ESCAPE;
            
            // account for the extra consumed input
            i++;
        }
        
        else if (IS_METACHAR(regexp[i])) {
            regex[index].class.c = regexp[i];
            regex[index].type = regexp[i];
        }

        // allowing for character classes
        else if (regexp[i] == BEGIN_CCL) {
            i++;

            // check for character class negation
            if (regexp[i] == BEGIN) {
                regex[index].nccl = 1;
                i++;
            }

            while (regexp[i] != END_CCL) {
                // take care of range
                if (regexp[i - 1] != ESCAPE && regexp[i] == RANGE) {
                    // catch if the range is not closed
                    if (regexp[i + 1] == '\0') {
                        fprintf(stderr, "unclosed range!\n");
                        return NULL;
                    }
                    
                    // add all the characters in the range to the mask 
                    char ch = regexp[i - 1];
                    for (; ch <= regexp[i + 1] && ch; ch++)
                        set_ind(regex[index].class.mask, ch);
                    
                    // move regexp pointer as needed
                    i++;
                }

                if (regexp[i] == '\0') {
                    fprintf(stderr, "unclosed character class!\n");
                    return NULL;
                }
                set_ind(regex[index].class.mask, regexp[i]);
                i++;
            }

            regex[index].type = CHAR_CLASS;
        }

        else {
            regex[index].class.c = regexp[i];
            regex[index].type = CHAR;
        }

        index++;
    }

    return regex;
}

int re_is_match(const char *regexp, const char *text) {
    re_t *reg = re_compile(regexp);
    int status;

    // checks if the text starts as desired
    if (reg[0].type == BEGIN) {
        status = match_here(reg + 1, text);
        re_free(reg);
        return status;
    }
    
    // match starting at any point in the text (even if text is empty)
    do {
        if ((status = match_here(reg, text))) {
            re_free(reg);
            return status;
        }
    } while (*text++ != '\0');

    re_free(reg);
    return 0;
}

/* search for regexp at the beginning of text */
int match_here(const re_t *reg, const char *text) {
    while (1) {
        // if there are no more expressions to check, we matched everything
        if (reg[0].type == TERMINAL)
            return 1;

        // if kleene star, then defer to helper function
        else if (reg[1].type == STAR)
            return match_kleene(&reg[0], reg + 2, text);
        
        // if we hit a termination character and are at the end of the regexp
        else if (reg[0].type == END && reg[1].type == TERMINAL)
            return *text == '\0';

        // if we hit a `+` character, check one or more
        else if (reg[1].type == PLUS) {
            if (!check_char(reg, text[0]))
                return 0;
            return match_kleene(&reg[0], reg + 2, text);
        }

        // if we hit a `?` character, check 0 or 1
        else if (reg[1].type == OPTIONAL) {
            // if we are at the end of our string, check that we are done matching
            if (text[0] == '\0')
                return reg[2].type == TERMINAL;
                
            // there is more than one instance of the character
            if (reg[0].class.c == text[1])
                return 0;
            
            // if the first character is the same, then we consume it
            if (reg[0].class.c == text[0] || (reg[0].type == DOT))
                text++;
            
            // skip over instruction in regexp
            reg += 2;

            // if the regexp is done but there are more (of the samee)
            continue;
        }

        /* if we are not at the end of the string and either:
         *  - the metacharacter matches
         *  - the literal character matches
         *  - the literal character is in the character class
         */ 
        else if (!check_char(reg, text[0]))
            break;
        
        reg++;
        text++;
    }

    // if none of the above hold, no match was found and return false
    return 0;
}

/* matches c*regexp at beginning of text */
int match_kleene(const re_t *c, const re_t *reg, const char *text) {
    // check for correct type coming in
    if (!(c->type == CHAR || c->type == DOT || c->type == CHAR_CLASS)) {
        fprintf(stderr, "incorrect type given to match_kleene: %d\n", c->type);
        return 0;
    }
    
    // while there are matches for kleene character, check if the remaining
    // string matches the regexp
    do {
        if (match_here(reg, text))
            return 1;
    } while (check_char(c, *text++));

    return 0;
}

void re_free(re_t *reg) {
    free(reg);
}
