/**
 * @file regex.h
 * @author Anshul Kamath
 * @brief A simple implementation of Regex for tokenization
 * @version 0.1
 * @date 2022-05-03
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef REGEX_H
#define REGEX_H

/**
 * @brief returns true if and only if the given pattern matches
 *        the given string. Support for the following constructs:
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
 * @param pattern   a pointer to the pattern to check
 * @param string    a pointer to the string to match
 * @return char * 
 */
int re_is_match(char *pattern, char *string);

/**
 * @brief returns a string of the first match found with the given `pattern`
 *        or NULL if no such match is found.
 * NOTE:  this pointer must be freed
 * 
 * @param pattern a pointer to the pattern to check
 * @param string  a pointer to the string to match
 * @return char* 
 */
char *re_get_match(char *pattern, char *string);

#endif
