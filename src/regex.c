#include "regex.h"
#include "regex-private.h"

#include <stdlib.h>
#include <string.h>

#define IS_METACHAR(x) \
    (x) == DOT || (x) == STAR || (x) == PLUS || (x) == OPTIONAL || (x) == BEGIN || (x) == END

int re_is_match(const char *regexp, const char *text) {
    // checks if the text starts as desired
    if (regexp[0] == '^')
        return match_here(regexp + 1, text);
    
    // match starting at any point in the text (even if text is empty)
    do {
        if (match_here(regexp, text)) return 1;
    } while (*text++ != '\0');

    return 0;
}

/* search for regexp at the beginning of text */
int match_here(const char *regexp, const char *text) {
    // bool to keep track of if we should escape the next metacharacter
    int is_escaped = 0;

    while (1) {
        // if there are no more expressions to check, we matched everything
        if (regexp[0] == '\0')
            return 1;

        // if the first character is an escape, then toggle the is_escaped bool
        if (regexp[0] == '\\') {
            is_escaped = !is_escaped;

            // if we are escaping, continue to the next character
            // if we are not, then keep matching
            if (is_escaped) {
                regexp++;
                continue;
            }
        }

        // if kleene star, then defer to helper function
        if (!is_escaped && regexp[1] == '*')
            return match_kleene(regexp[0], regexp + 2, text);
        
        // if we hit a termination character and are at the end of the regexp
        if (!is_escaped && regexp[0] == '$' && regexp[1] == '\0')
            return *text == '\0';

        // if we hit a `+` character, check one or more
        if (!is_escaped && regexp[1] == '+') {
            if (text[0] == '\0' || !(regexp[0] == '.' || text[0] == regexp[0]))
                return 0;
            return match_kleene(regexp[0], regexp + 2, text);
        }

        // if we hit a `?` character, check 0 or 1
        if (!is_escaped && regexp[1] == '?') {
            // if we are at the end of our string, check that we are done matching
            if (text[0] == '\0')
                return regexp[2] == '\0';
                
            // there is more than one instance of the character
            if (regexp[0] == text[1])
                return 0;
            
            // if the first character is the same, then we consume it
            if (regexp[0] == text[0] || (!is_escaped && regexp[0] == '.'))
                text++;
            
            // skip over instruction in regexp
            regexp += 2;

            // if the regexp is done but there are more (of the samee)
            continue;
        }

        // if we are not at the end of the string and either the regexp matches the
        // character literal or the regexp has the appropriate metacharacter
        if (*text == '\0' || !((!is_escaped && regexp[0] == '.') || regexp[0] == text[0]))
            break;
        
        regexp++;
        text++;
        is_escaped = 0;
    }

    // if none of the above hold, no match was found and return false
    return 0;
}

/* matches c*regexp at beginning of text */
int match_kleene(int c, const char *regexp, const char *text) {
    // while there are matches for kleene character, check if the remaining
    // string matches the regexp
    do {
        if (match_here(regexp, text))
            return 1;
    } while (*text != '\0' && (*text++ == c || c == '.'));

    return 0;
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
        if (regexp[i] == '\\') {
            regex[index].type = CHAR;

            // if the next character is a metacharacter, make it a character
            if (i + 1 < REGEXP_LEN && IS_METACHAR(regexp[i + 1]))
                regex[index].class.c = regexp[i + 1];
            else
                regex[index].class.c = '\\';
            
            // account for the extra consumed input
            i++;
        }
        
        else if (IS_METACHAR(regexp[i])) {
            regex[index].class.c = regexp[i];
            regex[index].type = regexp[i];
        }

        else {
            regex[index].class.c = regexp[i];
            regex[index].type = CHAR;
        }

        index++;
    }

    return regex;
}

void re_free(re_t *reg) {
    for (size_t i = 0; reg[i].type != TERMINAL; i++) {
        // free the buffer of the character class, if possible
        if (reg[i].type == CHAR_CLASS)
            free(reg[i].class.ccl);
    }

    // free the dynamically located array itself
    free(reg);
}
