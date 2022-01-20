#ifndef __MATCH_H
#define __MATCH_H

#include <stdbool.h>

/**
 * Returns true iff the pattern matches the whole text.
 * @param pattern the regular expression to match with
 * @param text the pattern to match
 **/
bool match(char *pattern, char *text);

#endif /* __MATCH_H */
