/*
 * parse.c
 *
 * Code that implements a recursive descent parser for arithmetic
 * expressions
 *
 * Author: Nwankwo Chukwunonso Michael
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "parse.h"
#include "tokenize.h"
#include "clist.h"

/*
 * Forward declarations for the functions (rules) to produce the
 * ExpressionWhizz grammar.  See the assignment writeup for the grammar.
 * Each function has the same signature, so we will document all of
 * them here.
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
static PipeTree pipe(TList tokens, char *errmsg, size_t errmsg_sz);
static PipeTree redirect(TList tokens, char *errmsg, size_t errmsg_sz);
static PipeTree primary(TList tokens, char *errmsg, size_t errmsg_sz);

static PipeTree pipe(TList tokens, char *errmsg, size_t errmsg_sz)
{
  PipeTree ret = redirect(tokens, errmsg, errmsg_sz);

  if (ret == NULL)
  {
    return NULL;
  }

  // check for zero or more occurence of a pipe
  if (TOK_next_type(tokens) == TOK_PIPE)
  {

    TOK_consume(tokens);

    PipeTree right = pipe(tokens, errmsg, errmsg_sz);

    if (right == NULL)
    {
      PT_free(ret); // free the malloc'd memory
      return NULL;
    }

    ret = PT_pipe(ret, right);
  }

  return ret;
}

/**
 * Parse a redirection expression
 * 
 * Parses a redirection expression from the token list, including the 
 * redirection operators >, >>, < and |.
 * 
 * Parameter
 *    tokens - Token list to parse 
 *    errmsg - Error message buffer
 *    errmsg_sz - Size of error message buffer
 * Return A parse tree for the redirection, or NULL on error
 */

static PipeTree redirect(TList tokens, char *errmsg, size_t errmsg_sz)
{

  PipeTree ret = primary(tokens, errmsg, errmsg_sz);

  if (ret == NULL)
  {
    return NULL;
  }

  // check for occurence of < or >
  if (TOK_next_type(tokens) == TOK_LESSTHAN || TOK_next_type(tokens) == TOK_GREATERTHAN)
  {

    // if it is an input, update the input file
    if (TOK_next_type(tokens) == TOK_LESSTHAN)
    {
      TOK_consume(tokens);

      // error handling, no file name
      if (TOK_next_type(tokens) != TOK_QUOTED_WORD && TOK_next_type(tokens) != TOK_WORD)
      {
        snprintf(errmsg, errmsg_sz, "Expect filename after redirection");
        PT_free(ret);
        return NULL;
      }

      // update the node
      setInputFiles(ret, TOK_next_word(tokens));

      // advance the token
      TOK_consume(tokens);

      // error handling, for multiple redirection
      if (TOK_next_type(tokens) == TOK_LESSTHAN)
      {
        snprintf(errmsg, errmsg_sz, "Multiple redirection");
        PT_free(ret);
        return NULL;
      }

      // check if output file needs to be set
      if (TOK_next_type(tokens) == TOK_GREATERTHAN)
      {
        TOK_consume(tokens);

        // error handling, no file name
        if (TOK_next_type(tokens) != TOK_QUOTED_WORD && TOK_next_type(tokens) != TOK_WORD)
        {
          snprintf(errmsg, errmsg_sz, "Expect filename after redirection");
          PT_free(ret);
          return NULL;
        }

        // update the node
        setOutputFiles(ret, TOK_next_word(tokens));

        TOK_consume(tokens);
      }
    }
    else
    {
      TOK_consume(tokens);

      // error handling, no file name
      if (TOK_next_type(tokens) != TOK_QUOTED_WORD && TOK_next_type(tokens) != TOK_WORD)
      {
        snprintf(errmsg, errmsg_sz, "Expect filename after redirection");
        PT_free(ret);
        return NULL;
      }

      // update the node
      setOutputFiles(ret, TOK_next_word(tokens));

      // advance the token
      TOK_consume(tokens);

      // error handling, for multiple redirection
      if (TOK_next_type(tokens) == TOK_GREATERTHAN)
      {
        snprintf(errmsg, errmsg_sz, "Multiple redirection");
        PT_free(ret);
        return NULL;
      }

      // check if input file needs to be set
      if (TOK_next_type(tokens) == TOK_LESSTHAN)
      {
        TOK_consume(tokens);

        // error handling, no file name
        if (TOK_next_type(tokens) != TOK_QUOTED_WORD && TOK_next_type(tokens) != TOK_WORD)
        {
          snprintf(errmsg, errmsg_sz, "Expect filename after redirection");
          PT_free(ret);
          return NULL;
        }

        // update the node
        setInputFiles(ret, TOK_next_word(tokens));

        TOK_consume(tokens);
      }
    }
  }

  return ret;
}


/**
 * Parse the primary part of an expression 
 *
 * This function parses the primary part of an expression from the given
 * token list. This includes literal values, identifiers, and parenthesized 
 * expressions.
 *
 * Parameters
 *    tokens - The token list to parse
 *    errmsg - Error message output buffer
 *    errmsg_sz - Size of the error message buffer
 * Returns A parse tree representing the primary expression.
*/
static PipeTree primary(TList tokens, char *errmsg, size_t errmsg_sz)
{

  if (TOK_next_type(tokens) == TOK_WORD || TOK_next_type(tokens) == TOK_QUOTED_WORD)
  {
    PipeTree ret = PT_word(TOK_next_word(tokens), NULL);
    TOK_consume(tokens);

    while (TOK_next_type(tokens) == TOK_WORD || TOK_next_type(tokens) == TOK_QUOTED_WORD)
    {
      PT_set_args(ret, TOK_next_word(tokens));

      TOK_consume(tokens);
    }

    return ret;
  }
  else
  {
    // error handling
    snprintf(errmsg, errmsg_sz, "No command specified");
    return NULL;
  }
}

// Documented in the .h file
PipeTree Parse(TList tokens, char *errmsg, size_t errmsg_sz)
{

  // if tokens is null or has only tok_end
  if (TOK_next_type(tokens) == TOK_END || tokens == NULL)
  {
    return NULL; // no further processing
  }

  PipeTree ret = pipe(tokens, errmsg, errmsg_sz);

  if (ret == NULL)
  {
    return NULL;
  }

  // check if the token list is at the end
  if (TOK_next_type(tokens) == TOK_END)
  {
    TOK_consume(tokens);
    return ret;
  }
  else
  {
    // handle error, unexpected token
    snprintf(errmsg, errmsg_sz, "Syntax error on token %s", TT_to_str(TOK_next_type(tokens)));
    PT_free(ret); // free malloc'd memory
    return NULL;
  }
}
