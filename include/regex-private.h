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
