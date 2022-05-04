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
 * 
 *          c   matches any literal character `c`
 *          .   matches any single character
 *          ^   matches the beginning of the input string
 *          $   matches the end of the input string
 *          *   matches zero or more occurrences of the previous character
 * 
 * @param pattern   a pointer to the pattern to check
 * @param string    a pointer to the string to match
 * @return int 
 */
int re_is_match(const char *pattern, const char *string);

#endif
