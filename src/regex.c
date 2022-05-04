#include "regex.h"

static int match_here(const char *regexp, const char *text);
static int match_kleene(int c, const char *regexp, const char *text);

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
static int match_here(const char *regexp, const char *text) {
    while (1) {
        // if there are no more expressions to check, we matched everything
        if (regexp[0] == '\0')
            return 1;

        // if kleene star, then defer to helper function
        if (regexp[1] == '*')
            return match_kleene(regexp[0], regexp + 2, text);
        
        // if we hit a termination character and are at the end of the regexp
        if (regexp[0] == '$' && regexp[1] == '\0')
            return *text == '\0';

        // if we are not at the end of the string and either the regexp matches the
        // character literal or the regexp has the appropriate metacharacter
        if (*text == '\0' || !(regexp[0] == '.' || regexp[0] == text[0]))
            break;
        
        regexp++;
        text++;
    }

    // if none of the above hold, no match was found and return false
    return 0;
}

/* matches c*regexp at beginning of text */
static int match_kleene(int c, const char *regexp, const char *text) {
    // while there are matches for kleene character, check if the remaining
    // string matches the regexp
    do {
        if (match_here(regexp, text))
            return 1;
    } while (*text != '\0' && (*text++ == c || c == '.'));

    return 0;
}
