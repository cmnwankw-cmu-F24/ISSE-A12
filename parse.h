/*
 * parse.h
 * 
 * Simple description
 *
 * Author: Howdy Pierce <howdy@sleepymoose.net>
 * Co-author: Nwankwo Chukwunonso Michael
 */


#ifndef _PARSE_H_
#define _PARSE_H_

#include "clist.h"
#include "pipeline.h"
#include "tlist.h"

/*
 * Parses a list of tokens into an ExprTree, which is the abstract
 * syntax tree for the ExpressionWhizz grammar.  See the assignment
 * writeup for the BNF.
 *
 * Parameters:
 *   tokens     List of tokens remaining to be parsed
 *   errmsg     Return space for an error message, filled in in case of error
 *   errmsg_sz  The size of errmsg
 * 
 * Returns: The parsed ExprTree on success. If a parsing error is
 *   encountered, copies an error message into errmsg and returns
 *   NULL.
 */
PipeTree Parse(TList tokens, char *errmsg, size_t errmsg_sz);

#endif /* _PARSE_H_ */
