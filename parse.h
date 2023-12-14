/*
 * parse.h
 *
 * Simple description
 *
 * Author: Nwankwo Chukwunonso Michael
 */

#ifndef _PARSE_H_
#define _PARSE_H_

#include "clist.h"
#include "pipeline.h"
#include "tlist.h"

/**
 * Parses a list of tokens into a PipeTree structure.
 *
 * This function takes a list of tokens (TList) representing a sequence of commands
 * and their associated elements (like arguments, pipes, and redirections) and constructs
 * a PipeTree. The PipeTree structure represents the hierarchical command structure
 * suitable for execution in a shell-like environment.
 *
 * Parameters
 *  tokens A TList of tokens to be parsed into a PipeTree. Each token should
 *        represent a part of the command line, such as a command, argument,
 *        or an operator like a pipe or redirection.
 *   errmsg A buffer to store an error message in case parsing fails. The message
 *        will be a null-terminated string describing the parsing error.
 *    errmsg_sz The size of the errmsg buffer. This function ensures that
 *        the error message does not exceed this size (including the null terminator).
 *
 * Return A PipeTree representing the parsed command structure on success.
 *         If parsing fails, returns NULL, and an appropriate error message
 *         is stored in errmsg.
 *
 * Note: The function assumes that the token list and error message buffer are properly
 *       initialized. It's the caller's responsibility to manage the memory for
 *       the token list and the error message buffer. The returned PipeTree should
 *       be freed by the caller when it's no longer needed.
 */
PipeTree Parse(TList tokens, char *errmsg, size_t errmsg_sz);

#endif /* _PARSE_H_ */
