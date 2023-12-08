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

#include "clist.h"
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

char *handleEscapeSequences(const char *input)
{
  char *result = malloc(2048); // Allocate enough memory
  if (!result)
    return NULL; // Handle memory allocation failure

  const char *src = input;
  char *dst = result;

  while (*src)
  {
    if (*src == '\\')
    { // Check for escape character
      src++;
      switch (*src)
      {
      case 'n':
        *dst++ = '\n';
        break;
      case 'r':
        *dst++ = '\r';
        break;
      case 't':
        *dst++ = '\t';
        break;
      case '\"':
        *dst++ = '\"';
        break;
      case '\\':
        *dst++ = '\\';
        break;
      case ' ':
        *dst++ = ' ';
        break;
      case '|':
        *dst++ = '|';
        break;
      case '<':
        *dst++ = '<';
        break;
      case '>':
        *dst++ = '>';
        break;
      default:
        *dst++ = *src; // If it's not a recognized escape sequence, copy as is
      }
    }
    else
    {
      *dst++ = *src;
    }
    src++;
  }
  *dst = '\0'; // Null terminate the result string
  return result;
}

// Documented in .h file
CList TOK_tokenize_input(const char *input, char *errmsg, size_t errmsg_sz)
{
  CList tokens = CL_new();

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

      CL_append(tokens, token);
      pos++;
      input++;
    }
    else if (*input == '>')
    {
      token.type = TOK_GREATERTHAN;
      token.word = NULL;

      CL_append(tokens, token);
      pos++;
      input++;
    }
    else if (*input == '|')
    {
      token.type = TOK_PIPE;
      token.word = NULL;

      CL_append(tokens, token);
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
        if (*input == '\\' && (*(input + 1) == 'n' ||
                               *(input + 1) == 'r' ||
                               *(input + 1) == 't' ||
                               *(input + 1) == '\"' ||
                               *(input + 1) == '\\' ||
                               *(input + 1) == ' ' ||
                               *(input + 1) == '|' ||
                               *(input + 1) == '>' ||
                               *(input + 1) == '<'))
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
          // case '\\': escape_char[0] = '\\'; break;
          // case ' ': escape_char[0] = ' '; break;
          // case '|': escape_char[0] = '|'; break;
          // case '>': escape_char[0] = '>'; break;
          // case '<': escape_char[0] = '<'; break;
          default: // handle \" \\ space | > <
            escape_char[0] = *input;
          }
          strncat(word, escape_char, 1);
        }
        else
        {
          strncat(word, input, 1);
        }
        len++;
        input++;

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
        snprintf(errmsg, errmsg_sz, "Position %li: Unterminated \"quoted word\"", pos + 1);
        free(word);
        TOK_free(tokens);
        word = NULL;
        tokens = NULL;
        return NULL;
      }

      token.type = TOK_QUOTED_WORD;
      token.word = word;

      CL_append(tokens, token);

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
        if (*input == '\\' && (*(input + 1) == 'n' ||
                               *(input + 1) == 'r' ||
                               *(input + 1) == 't' ||
                               *(input + 1) == '\"' ||
                               *(input + 1) == '\\' ||
                               *(input + 1) == ' ' ||
                               *(input + 1) == '|' ||
                               *(input + 1) == '>' ||
                               *(input + 1) == '<'))
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
          // case '\"': escape_char[0] = '\"'; break;
          // case '\\': escape_char[0] = '\\'; break;
          // case ' ': escape_char[0] = ' '; break;
          // case '|': escape_char[0] = '|'; break;
          // case '>': escape_char[0] = '>'; break;
          // case '<': escape_char[0] = '<'; break;
          default: // handle \" \\ space | > <
            escape_char[0] = *input;
          }
          strncat(word, escape_char, 1);
        }
        else
        {
          strncat(word, input, 1);
        }
        input++;
        len++;

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

      CL_append(tokens, token);

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
TokenType TOK_next_type(CList tokens)
{
  // return the next token type or token type at the head of the list
  return CL_nth(tokens, 0).type;
}

// Documented in .h file
Token TOK_next(CList tokens)
{

  // return the next token or token at the head of the list
  return CL_nth(tokens, 0);
}

// Documented in .h file
void TOK_consume(CList tokens)
{
  // pop the head node
  CL_pop(tokens);

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
void TOK_print_callback(int pos, CListElementType token, void *cb_data)
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
void TOK_print(CList tokens)
{
  if (tokens == NULL)
  {
    return;
  }

  CL_foreach(tokens, TOK_print_callback, "Token");
}

void TOK_free_callback(int pos, CListElementType token, void *cb_data)
{
  if (token.type == TOK_QUOTED_WORD || token.type == TOK_WORD)
  {
    free(token.word);
  }
}

void TOK_free(CList tokens)
{

  // tokens is NULL
  if (tokens == NULL)
  {
    return;
  }

  // free the words
  CL_foreach(tokens, TOK_free_callback, NULL);

  // before freeing the list
  CL_free(tokens);
}

// int main()
// {

//   char errmsg[128];

//   CList tokens = NULL;

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
  // tokens = TOK_tokenize_input("a\"b\\nc\"", errmsg, sizeof(errmsg));
  // // Happy path
  // TOK_print(tokens);

  // // error
  // printf("%s \n", errmsg);
  // TOK_free(tokens);

//   return 0;
// }
