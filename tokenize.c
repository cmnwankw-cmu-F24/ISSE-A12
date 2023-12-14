/*
 * tokenize.c
 *
 * Functions to tokenize and manipulate lists of tokens
 *
 * Author: Nwankwo Chukwunonso Michael
 */

#include <stdio.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <glob.h>

#include "tlist.h"
#include "tokenize.h"
#include "token.h"

// Documented in .h file
const char *TT_to_str(TokenType tt)
{
  switch (tt)
  {
  case TOK_WORD:
    return "WORD";
  case TOK_QUOTED_WORD:
    return "QUOTED_WORD";
  case TOK_LESSTHAN:
    return "LESSTHAN";
  case TOK_GREATERTHAN:
    return "GREATERTHAN";
  case TOK_PIPE:
    return "PIPE";
  case TOK_END:
    return "(end)";
  }

  __builtin_unreachable();
}

/**
 * Checks if a word needs glob expansion.
 *
 * This examines the given word and returns true if it contains
 * glob patterns like *, ?, [] that need expansion.
 *
 * Parameter
 *   word - The word to check
 *
 * Returns
 *   true if globbing needed, false otherwise
 *
 */

bool needsGlobbing(const char *word)
{
  // Check if the word contains '*', '?', or '[]'
  return strchr(word, '*') || strchr(word, '?') || (strchr(word, '[') && strchr(word, ']'));
}

// Documented in .h file
TList TOK_tokenize_input(const char *input, char *errmsg, size_t errmsg_sz)
{
  // initialize a TList of tokens
  TList tokens = TL_new();

  size_t pos = 0;

  Token token;

  while (*input != '\0')
  {

    // Skip through spaces
    if (isspace(*input))
    {
      // advance and do nothing
      pos++;
      input++;
      continue;
    }
    else if (*input == '<')
    {
      // tokenize the character <
      token.type = TOK_LESSTHAN;
      token.word = NULL;

      TL_append(tokens, token);
      pos++;
      input++;
    }
    else if (*input == '>')
    {
      // tokenize the character >
      token.type = TOK_GREATERTHAN;
      token.word = NULL;

      TL_append(tokens, token);
      pos++;
      input++;
    }
    else if (*input == '|')
    {
      // tokenize the character |
      token.type = TOK_PIPE;
      token.word = NULL;

      TL_append(tokens, token);
      pos++;
      input++;
    }
    else if (*input == '"')
    {
      // tokenize the quoted word

      input++;                   // skip past the double quotes
      const char *start = input; // keep a reference to the start

      // initialize the start memory capacity for words and length of characters seen
      size_t capacity = 8;
      size_t len = 0;

      // calloc some memory
      char *word = (char *)calloc(capacity, sizeof(char));
      assert(word);

      // Loop through till we find a terminating douuble quotes or null terminator
      while (*input != '\0' && *input != '\"')
      {

        // check for escape sequence conditions
        if (*input == '\\')
        {
          // skip the escape character
          input++;

          // Variable to hold escape character
          char escape_char[2] = {'\0'};

          switch (*input)
          {
          case 'n':
            escape_char[0] = '\n';
            break;
          case 'r':
            escape_char[0] = '\r';
            break;
          case 't':
            escape_char[0] = '\t';
            break;
          case '\"':
            escape_char[0] = '\"';
            break;
          case '\\':
            escape_char[0] = '\\';
            break;
          case ' ':
            escape_char[0] = ' ';
            break;
          case '|':
            escape_char[0] = '|';
            break;
          case '>':
            escape_char[0] = '>';
            break;
          case '<':
            escape_char[0] = '<';
            break;
          default: // Illegal escape sequence, error handling

            snprintf(errmsg, errmsg_sz, "Illegal escape character '%c'", *input);
            free(word);
            TOK_free(tokens);
            word = NULL;
            tokens = NULL;
            return NULL;
          }

          // append the character found
          strncat(word, escape_char, 1);
        }
        else
        {
          // append the character found
          strncat(word, input, 1);
        }

        len++;
        input++;
        word[len] = '\0';

        // if the capacity is about to be exceeded, double the capacity
        if (len == capacity - 1)
        {
          capacity *= 2;
          word = (char *)realloc(word, capacity);
          assert(word);
        }
      }

      if (*input == '\0')
      {
        // Error, I hit the null terminator without closing
        // before the terminating " for the quoted word
        snprintf(errmsg, errmsg_sz, "Unterminated quote");
        free(word);
        TOK_free(tokens);
        word = NULL;
        tokens = NULL;
        return NULL;
      }

      // check that at least one character was captured
      // in between the quotes
      if (input - start > 1)
      {
        token.type = TOK_QUOTED_WORD;
        token.word = word;

        TL_append(tokens, token);
      }
      else
      {
        free(word);
      }

      // update the current position
      pos += (input - start);

      // skip past the last "
      input++;
    }
    else
    {
      // Tokenize a regular word

      const char *start = input; // keep a reference to the start

      // initialize the start memory capacity for words and length of characters seen
      size_t capacity = 8;
      size_t len = 0;

      // calloc some memory
      char *word = (char *)calloc(capacity, sizeof(char));
      assert(word);

      // Loop through till we find a terminating condition for a word
      while (*input != '<' && *input != '>' && *input != '|' && *input != '"' && !isspace(*input) && *input != '\0')
      {

        // check for escape sequence conditions
        if (*input == '\\')
        {
          // skip the escape character
          input++;

          // Variable to hold escape character
          char escape_char[2] = {'\0'};

          switch (*input)
          {
          case 'n':
            escape_char[0] = '\n';
            break;
          case 'r':
            escape_char[0] = '\r';
            break;
          case 't':
            escape_char[0] = '\t';
            break;
          case '\"':
            escape_char[0] = '\"';
            break;
          case '\\':
            escape_char[0] = '\\';
            break;
          case ' ':
            escape_char[0] = ' ';
            break;
          case '|':
            escape_char[0] = '|';
            break;
          case '>':
            escape_char[0] = '>';
            break;
          case '<':
            escape_char[0] = '<';
            break;
          default: // Illegal escape sequence, error handling

            snprintf(errmsg, errmsg_sz, "Illegal escape character '%c'", *input);
            free(word);
            TOK_free(tokens);
            word = NULL;
            tokens = NULL;
            return NULL;
          }

          // append character found
          strncat(word, escape_char, 1);
        }
        else
        {
          // append the character found
          strncat(word, input, 1);
        }
        input++;
        len++;
        word[len] = '\0';

        // if the capacity is about to be exceeded, double the capacity
        if (len == (capacity - 1))
        {
          capacity *= 2;
          word = (char *)realloc(word, capacity);
          assert(word);
        }
      }

      // check if word found needs globbing
      if (needsGlobbing(word))
      {
        glob_t glob_result;
        memset(&glob_result, 0, sizeof(glob_result)); // zero out memory

        int result = glob(word, GLOB_TILDE, NULL, &glob_result);
        if (result == 0)
        {
          // Loop through all matched file names and append
          for (size_t i = 0; i < glob_result.gl_pathc; ++i)
          {
            token.type = TOK_WORD;
            token.word = strdup(glob_result.gl_pathv[i]);
            TL_append(tokens, token);
          }

          // free the malloc'd memory
          free(word);
        }
        else
        {
          // no matched file found, just tokenize
          token.type = TOK_WORD;
          token.word = word;
          TL_append(tokens, token);
        }

        // free malloc'd memory
        globfree(&glob_result);
      }
      else
      {
        // word does not need globbing, just append
        token.type = TOK_WORD;
        token.word = word;
        TL_append(tokens, token);
      }

      // update the current position
      pos += (input - start);
    }
  }

  return tokens;
}

// Documented in .h file
TokenType TOK_next_type(TList tokens)
{
  // return the next token type or token type at the head of the list
  return TL_nth(tokens, 0).type;
}

// Documented in .h file
const char *TOK_next_word(TList tokens)
{
  // return the next token type or token type at the head of the list
  return TL_nth(tokens, 0).word;
}

// Documented in .h file
Token TOK_next(TList tokens)
{

  // return the next token or token at the head of the list
  return TL_nth(tokens, 0);
}

// Documented in .h file
void TOK_consume(TList tokens)
{

  if(tokens == NULL){
    return;
  }
  if (TL_nth(tokens, 0).type == TOK_WORD || TL_nth(tokens, 0).type == TOK_QUOTED_WORD)
    free((void *)TL_nth(tokens, 0).word);

  // pop the head node
  TL_pop(tokens);

  return;
}

/*
 *
 * Print a token's information to the console.
 *
 * This function is a callback used for printing the details of a token to the console.
 * It takes the position, token type, and token value, and prints them in a specific format
 * to provide a human-readable representation of the token.
 *
 * Parameters
 *      pos - The position of the token in the input.
 *      token - The token to be printed.
 *      cb_data - A pointer to custom callback data, which should be a string.
 *
 * Returns - None
 */
void TOK_print_callback(int pos, TListElementType token, void *cb_data)
{
  if (token.type == TOK_WORD || token.type == TOK_QUOTED_WORD)
  {
    printf("%s [%d] type ==> %s, word ==> %s\n", (char *)cb_data, pos, TT_to_str(token.type), token.word);
  }
  else
  {
    printf("%s [%d] type ==> %s\n", (char *)cb_data, pos, TT_to_str(token.type));
  }
}

// Documented in .h file
void TOK_print(TList tokens)
{
  if (tokens == NULL)
  {
    return;
  }

  TL_foreach(tokens, TOK_print_callback, "Token");
}

/**
 * Callback function to free token words.
 *
 * This is passed to TOK_free() to free the memory for
 * any TOK_WORD or TOK_QUOTED_WORD tokens.
 *
 * Parameters
 *      pos - The position of the token in the input.
 *      token - The token to be freed.
 *      cb_data - A pointer to custom callback data(optional, unused)
 * Returns
 *      None
 */

void TOK_free_callback(int pos, TListElementType token, void *cb_data)
{
  if (token.type == TOK_QUOTED_WORD || token.type == TOK_WORD)
  {
    free(token.word);
  }
}

/**
 * Free a token list and associated memory.
 *
 * This frees the list structure itself as well
 * as the words allocated for any WORD or
 * QUOTED_WORD tokens.
 *
 * Parameter
 *    tokens - Token list to free
 * Returns
 *    None
 */

void TOK_free(TList tokens)
{

  // tokens is NULL
  if (tokens == NULL)
  {
    return;
  }

  // free the words
  TL_foreach(tokens, TOK_free_callback, NULL);

  // before freeing the list
  TL_free(tokens);
}