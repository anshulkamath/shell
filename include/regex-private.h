/**
 * @file regex-private.h
 * @author Anshul Kamath
 * @brief A file for private regex functions (only include for testing purposes)
 * @version 0.1
 * @date 2022-05-03
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef REGEX_PRIV_H
#define REGEX_PRIV_H

/**
 * @brief character classes given by the following list:
 * --------
 *      c   CHAR        matches any literal character `c`
 *      .   DOT         matches any single character
 *      ^   BEGIN       matches the beginning of the input string
 *      $   END         matches the end of the input string
 *      *   STAR        matches zero or more occurrences of the previous character
 *      +   PLUS        matches one or more occurrences of the previous character
 *      ?   OPTIONAL    matches the previous character zero or once
 *    [abc] CHAR_CLASS  matches any character inside the class
 *    [^..] NEG_CLASS   matches any character not inside the class
 * 
 */
typedef enum class { 
    CHAR = 1, CHAR_CLASS, DOT = '.', STAR = '*', PLUS = '+', OPTIONAL = '?', BEGIN = '^', END = '$', TERMINAL = '\0'
} class_t;

/* type to manage character classes */
typedef struct re {
    union {
        int     c;      /* the character */
        char    *ccl;   /* list of the characters in class */
    } class;        /* union to the character or character class */
    class_t type;   /* CHAR, STAR, etc. */
    int     nccl;   /* true if character class is negated */
} re_t;

/**
 * @brief compiles a regexp pattern into a list of `re_t`s
 * NOTE:  this function allocates memory on the heap: must free
 * 
 * @param regexp the pattern to compile
 * @return re_t* 
 */
re_t *re_compile(const char *regexp);

/**
 * @brief frees the memory allocated by the given `re_t`
 * 
 * @param reg 
 */
void re_free(re_t *reg);

/**
 * @brief returns true if and only if `regexp` matches `text`
 * NOTE:  this is a private function - use re_is_match instead
 * TODO:  move this to a separate file
 * 
 * @param regexp the regexp to test against
 * @param text   the text to match
 * @return int 
 */
int match_here(const char *regexp, const char *text);

/**
 * @brief same as match_here, but matches an arbitrary number of
 *        character `c`s at the beginning of the string as well
 * NOTE:  this is a private function - use re_is_match instead
 * TODO:  move this to a separate file
 * 
 * @param c      the character to match at the beginning of the string
 * @param regexp the pattern to match against
 * @param text   the text to match
 * @return int 
 */
int match_kleene(int c, const char *regexp, const char *text);

#endif
