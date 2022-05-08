#include "regex.h"
#include "regex-private.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IS_METACHAR(x) \
    ((x) == DOT || (x) == STAR || (x) == PLUS || (x) == OPTIONAL || (x) == BEGIN || (x) == END)

#define IS_ABBR(x) \
    ((x) == DIGIT || (x) == N_DIGIT || (x) == ALPH || (x) == N_ALPH || (x) == SPACE || (x) == N_SPACE || (x) == WORD || (x) == N_WORD)

/* hard-coded manual patterns */
static char RE_DIGIT[]    = "[0-9]";
static char RE_N_DIGIT[]  = "[^0-9]";
static char RE_ALPH[]     = "[a-z]";
static char RE_N_ALPH[]   = "[^a-z]";
static char RE_SPACE[]    = "[\n\t\r ]";
static char RE_N_SPACE[]  = "[^\n\t\r ]";
static char RE_WORD[]     = "[a-zA-Z0-9]";
static char RE_N_WORD[]     = "[^a-zA-Z0-9]";

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

char *re_precompile(const char *regexp) {
    const int RE_LEN = strlen(regexp);
    int length = RE_LEN;

    char *str = calloc(1, length);
    int idx = 0; /* index of str */

    for (int i = 0; regexp[i]; i++) {
        if (idx == length) {
            length *= 2;
            str = realloc(str, length);
        }

        // we do not encounter a shortcut character, copy it to the buffer and move on
        if (!(regexp[i] == '\\' && IS_ABBR(regexp[i + 1]))) {
            str[idx++] = regexp[i];
            continue;
        }

        // we have encountered a shortcut
        i++;  // move pointer to shortcut character

        // get pattern
        char *pattern;
        switch(regexp[i]) {
            case DIGIT:
                pattern = RE_DIGIT;
                break;
            case N_DIGIT:
                pattern = RE_N_DIGIT;
                break;
            case ALPH:
                pattern = RE_ALPH;
                break;
            case N_ALPH:
                pattern = RE_N_ALPH;
                break;
            case SPACE:
                pattern = RE_SPACE;
                break;
            case N_SPACE:
                pattern = RE_N_SPACE;
                break;
            case WORD:
                pattern = RE_WORD;
                break;
            case N_WORD:
                pattern = RE_N_WORD;
                break;
        }

        // guarantee space for pattern
        int patt_len = strlen(pattern);
        if (idx + patt_len >= length) {
            length *= 2;
            str = realloc(str, length);
        }

        // copy pattern to string
        memcpy(str + idx, pattern, patt_len);
        idx += patt_len;
    }

    return str;
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

            // add all characters to class bit map
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

                // set flag in bit map to indicate part of char class
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

int re_is_match(char *regexp, char *text) {
    char *exp_regexp = re_precompile(regexp);
    re_t *reg = re_compile(exp_regexp);
    free(exp_regexp);

    int status;

    // checks if the text starts as desired
    if (reg[0].type == BEGIN) {
        status = !!match_here(reg + 1, text);
        re_free(reg);
        return status;
    }
    
    // match starting at any point in the text (even if text is empty)
    do {
        if ((status = !!match_here(reg, text))) {
            re_free(reg);
            return status;
        }
    } while (*text++ != '\0');

    re_free(reg);
    return 0;
}

char *re_get_match(char *regexp, char *text) {
    char *exp_regexp = re_precompile(regexp);
    re_t *reg = re_compile(exp_regexp);
    free(exp_regexp);

    char *end_match;

    // checks if the text starts as desired
    if (reg[0].type == BEGIN) {
        end_match = match_here(reg + 1, text);
    } else {
        // match starting at any point in the text (even if text is empty)
        do {
            if ((end_match = match_here(reg, text)))
                break;
        } while (*text++ != '\0');
    }

    re_free(reg);
    if (!end_match) return NULL;
    
    char *str = calloc(1, (end_match - text + 1));
    memcpy(str, text, (end_match - text));
    return str;
}

/* search for regexp at the beginning of text */
char *match_here(const re_t *reg, char *text) {
    while (1) {
        // if there are no more expressions to check, we matched everything
        if (reg[0].type == TERMINAL)
            return text;

        // if kleene star, then defer to helper function
        else if (reg[1].type == STAR) {
            if (!(text = match_kleene(&reg[0], reg + 2, text)))
                return NULL;
            
            reg += 2;
            continue;
        }
        
        // if we hit a termination character and are at the end of the regexp
        else if (reg[0].type == END && reg[1].type == TERMINAL)
            return *text == '\0' ? text : NULL;

        // if we hit a `+` character, check one or more
        else if (reg[1].type == PLUS) {
            if (!check_char(reg, text[0]))
                return NULL;
            
            if (!(text = match_kleene(&reg[0], reg + 2, text)))
                return NULL;
            
            reg += 2;
            continue;
        }

        // if we hit a `?` character, check 0 or 1
        else if (reg[1].type == OPTIONAL) {
            // if we are at the end of our string, check that we are done matching
            if (text[0] == '\0')
                return reg[2].type == TERMINAL ? text : NULL;
                
            // there is more than one instance of the character
            if (reg[0].class.c == text[1])
                return NULL;
            
            // if the first character is the same, then we consume it
            if (reg[0].class.c == text[0] || (reg[0].type == DOT))
                text++;
            
            // skip over instruction in regexp
            reg += 2;

            // if the regexp is done but there are more (of the samee)
            continue;
        }

        // if the next character does not pass the subsequent regex task,
        // break and return 0
        else if (!check_char(reg, text[0]))
            break;
        
        reg++;
        text++;
    }

    // if none of the above hold, no match was found and return false
    return NULL;
}

/* matches c*regexp at beginning of text */
char *match_kleene(const re_t *c, const re_t *reg, char *text) {
    // check for correct type coming in
    if (!(c->type == CHAR || c->type == DOT || c->type == CHAR_CLASS)) {
        fprintf(stderr, "incorrect type given to match_kleene: %d\n", c->type);
        return 0;
    }
    
    // while there are matches for kleene character, check if the remaining
    // string matches the regexp
    do {
        if (match_here(reg, text))
            return text;
    } while (check_char(c, *text++));

    return NULL;
}

void re_free(re_t *reg) {
    free(reg);
}
