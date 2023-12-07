/*
 * token.h
 * 
 * Enum containing all possible tokens
 *
 * Co-author: Nwankwo Chukwunonso Michael
 */

#ifndef _TOKEN_H_
#define _TOKEN_H_

typedef enum {
  TOK_WORD,
  TOK_QUOTED_WORD, 
  TOK_LESSTHAN, 
  TOK_GREATERTHAN, 
  TOK_PIPE,
  TOK_END
} TokenType;


typedef struct {
  TokenType type;
  char *word;
} Token;


#endif /* _TOKEN_H_ */
