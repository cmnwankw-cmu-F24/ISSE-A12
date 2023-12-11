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

// Documented in .h file
TList TOK_tokenize_input(const char *input, char *errmsg, size_t errmsg_sz)
{
  TList tokens = TL_new();

  size_t pos = 0;
  // char *invalid_char = NULL;
  Token token;

  while (*input != '\0')
  {

    if (isspace(*input))
    {
      // advance and do nothing
      pos++;
      input++;
      continue;
    }
    else if (*input == '<')
    {
      token.type = TOK_LESSTHAN;
      token.word = NULL;

      TL_append(tokens, token);
      pos++;
      input++;
    }
    else if (*input == '>')
    {
      token.type = TOK_GREATERTHAN;
      token.word = NULL;

      TL_append(tokens, token);
      pos++;
      input++;
    }
    else if (*input == '|')
    {
      token.type = TOK_PIPE;
      token.word = NULL;

      TL_append(tokens, token);
      pos++;
      input++;
    }
    else if (*input == '"')
    {
      // Quoted word
      const char *start = input; // keep a reference to the start
      input++;
      start = input;

      size_t capacity = 8;
      size_t len = 0;

      // calloc some memory
      char *word = (char *)calloc(capacity, sizeof(char));
      assert(word);

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
          default: // Illegal escape sequence

            snprintf(errmsg, errmsg_sz, "Illegal escape character '%c'", *input);
            free(word);
            TOK_free(tokens);
            word = NULL;
            tokens = NULL;
            return NULL;
          }
          strncat(word, escape_char, 1);
        }
        else
        {
          strncat(word, input, 1);
        }
        len++;
        input++;
        word[len] = '\0';

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

      pos += (input - start);

      // skip past the last "
      input++;
    }
    else
    {
      // Regular word
      const char *start = input;

      size_t capacity = 8;
      size_t len = 0;

      // calloc some memory
      char *word = (char *)calloc(capacity, sizeof(char));
      assert(word);

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
          default: // Illegal escape sequence

            snprintf(errmsg, errmsg_sz, "Illegal escape character '%c'", *input);
            free(word);
            TOK_free(tokens);
            word = NULL;
            tokens = NULL;
            return NULL;
          }
          strncat(word, escape_char, 1);
        }
        else
        {
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

      token.type = TOK_WORD;
      token.word = word;

      TL_append(tokens, token);

      pos += (input - start);
    }
  }

  // uncomment to delimit the tokenizer with TOK_END
  // token.type = TOK_END;
  // token.word = NULL;
  // CL_append(tokens, token);

  return tokens;

  // error_handling:
  // invalid character, return with an error message
  // snprintf(errmsg, errmsg_sz, "Position %li: unexpected character %s", pos, invalid_char);
  // TOK_free(tokens);
  // tokens = NULL;
  // return NULL;
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

void TOK_free_callback(int pos, TListElementType token, void *cb_data)
{
  if (token.type == TOK_QUOTED_WORD || token.type == TOK_WORD)
  {
    free(token.word);
  }
}

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

// int main()
// {

//   char errmsg[128];

//   TList tokens = NULL;

//   // echo a b
//   tokens = TOK_tokenize_input("\\", errmsg, sizeof(errmsg));
//   // Happy path
//   TOK_print(tokens);

//   // error
//   printf("%s \n", errmsg);
//   TOK_free(tokens);

// // echo a\ b
// tokens = TOK_tokenize_input("echo a\\ b", errmsg, sizeof(errmsg));
// // Happy path
// TOK_print(tokens);

// // error
// printf("%s \n", errmsg);
// TOK_free(tokens);

// // echo "a b"
// tokens = TOK_tokenize_input("echo \"a b\"", errmsg, sizeof(errmsg));
// // Happy path
// TOK_print(tokens);

// // error
// printf("%s \n", errmsg);
// TOK_free(tokens);

// // echo a\\ b
// tokens = TOK_tokenize_input("echo a\\\\ b", errmsg, sizeof(errmsg));
// // Happy path
// TOK_print(tokens);

// // error
// printf("%s \n", errmsg);
// TOK_free(tokens);

// // echo hello|grep "ell"
// tokens = TOK_tokenize_input("echo hello|grep \"ell\"", errmsg, sizeof(errmsg));
// // Happy path
// TOK_print(tokens);

// // error
// printf("%s \n", errmsg);
// TOK_free(tokens);

// // echo hello\|grep "ell"
// tokens = TOK_tokenize_input("echo hello\\|grep \"ell\"", errmsg, sizeof(errmsg));
// // Happy path
// TOK_print(tokens);

// // error
// printf("%s \n", errmsg);
// TOK_free(tokens);

// // echo boo >out_file
// tokens = TOK_tokenize_input("echo boo>out_file", errmsg, sizeof(errmsg));
// // Happy path
// TOK_print(tokens);

// // error
// printf("%s \n", errmsg);
// TOK_free(tokens);

// // echo"boo">out_file
// tokens = TOK_tokenize_input("echo\"boo\">out_file", errmsg, sizeof(errmsg));
// // Happy path
// TOK_print(tokens);

// // error
// printf("%s \n", errmsg);
// TOK_free(tokens);

// // no input
// tokens = TOK_tokenize_input("", errmsg, sizeof(errmsg));
// // Happy path
// TOK_print(tokens);

// // error
// printf("%s \n", errmsg);
// TOK_free(tokens);

// // echo "hello | grep"
// tokens = TOK_tokenize_input("hello | grep", errmsg, sizeof(errmsg));
// // Happy path
// TOK_print(tokens);

// // error
// printf("%s \n", errmsg);
// TOK_free(tokens);

// // echo a"b c"
// tokens = TOK_tokenize_input("echo \\<\\|\\> | cat", errmsg, sizeof(errmsg));
// // Happy path
// TOK_print(tokens);

// // error
// printf("%s \n", errmsg);
// TOK_free(tokens);

//   return 0;
// }
