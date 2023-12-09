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

    PipeTree right = primary(tokens, errmsg, errmsg_sz);

    if (right == NULL)
    {
      PT_free(ret); // free the malloc'd memory
      return NULL;
    }

    ret = PT_node(CMD_PIPE, ret, right);
  }

  return ret;
}

static PipeTree redirect(TList tokens, char *errmsg, size_t errmsg_sz)
{

  PipeTree ret = primary(tokens, errmsg, errmsg_sz);

  if (ret == NULL)
  {
    return NULL;
  }

  // check for zero or more occurence of < or >
  if (TOK_next_type(tokens) == TOK_LESSTHAN || TOK_next_type(tokens) == TOK_GREATERTHAN)
  {

    TokenType tmp_type = TOK_next_type(tokens); // keep a reference of whether it's a < or >

    // advance to the next token
    TOK_consume(tokens);

    // PipeTree left = ret; // keep a reference of the left child

    // check if there is a token at the head
    if (TOK_next_type(tokens) == TOK_END)
    {
      // if none, error handling
      PT_free(ret);
      snprintf(errmsg, errmsg_sz, "Unexpected token %s", TT_to_str(TOK_next_type(tokens)));
      return NULL;
    }

    PipeTree right = PT_word(TOK_next_word(tokens), NULL);

    TOK_consume(tokens);

    // the right child of a redirect should be a leaf node
    if (TOK_next_type(tokens) == TOK_WORD || TOK_next_type(tokens) == TOK_QUOTED_WORD)
    {
      PT_free(ret);
      PT_free(right);
      snprintf(errmsg, errmsg_sz, "Unexpected token %s", TT_to_str(TOK_next_type(tokens)));
      return NULL;
    }

    if(tmp_type == TOK_LESSTHAN ){
      ret = PT_node(CMD_LESS, ret, right);
    }else{
      ret = PT_node(CMD_GREAT, ret, right);
    }
  }

  return ret;
}

static PipeTree primary(TList tokens, char *errmsg, size_t errmsg_sz)
{
  
  if(TOK_next_type(tokens) == TOK_WORD || TOK_next_type(tokens) == TOK_QUOTED_WORD){
    PipeTree ret = PT_word(TOK_next_word(tokens), NULL);
    TOK_consume(tokens);


    while(TOK_next_type(tokens) == TOK_WORD || TOK_next_type(tokens) ==TOK_QUOTED_WORD){
      PT_set_args(ret, TOK_next_word(tokens));

      TOK_consume(tokens);
    }

    return ret;
  }else{
    //error handling
    snprintf(errmsg, errmsg_sz, "Unexpected token %s", TT_to_str(TOK_next_type(tokens)));
    return NULL;
  }
  
}



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
