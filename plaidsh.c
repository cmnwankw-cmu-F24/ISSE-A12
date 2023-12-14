/*
 * Plaid Shell
 *
 * Author: Nwankwo Chukwunonso Michael
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdbool.h>

#include "tlist.h"
#include "tokenize.h"
#include "token.h"
#include "parse.h"
#include "pipeline.h"

// colors
#define BOLD_RED

int main(int argc, char *argv[])
{
    char *input = NULL;
    bool time_to_quit = false;
    char errmsg[128] = {'\0'};
    // char errmsg2[128] = {'\0'};
    PipeTree tree = NULL;
    TList tokens = NULL;

    printf("\n\e[01;34mWelcome to \e[01;32mPlaid Shell!\e[01;39m\n");

    while (!time_to_quit)
    {
        // Step 1: Read User input
        input = readline("\n\e[01;31m#?\e[00;39m ");
        if (input == NULL || strcasecmp(input, "quit") == 0)
        {
            time_to_quit = true;
            goto loop_end;
        }

        if (*input == '\0' || feof(stdin)) // user just hit enter, no content
            goto loop_end;

        add_history(input);

        // Step 2: Tokenize the user input
        tokens = TOK_tokenize_input(input, errmsg, sizeof(errmsg));

        if (tokens == NULL)
        {
            fprintf(stderr, "%s\n", errmsg);
            goto loop_end;
        }

        if (TL_length(tokens) == 0)
            goto loop_end;

        // Step 3: parse the input
        tree = Parse(tokens, errmsg, sizeof(errmsg));


        if (tree == NULL)
        {
            fprintf(stderr, "%s\n", errmsg);
            goto loop_end;
        }

        // Step 4: evaluate the tree
        PT_evaluate(tree);
        goto loop_end;

    loop_end:
        free(input);
        input = NULL;
        TOK_free(tokens);
        tokens = NULL;
        PT_free(tree);
        tree = NULL;
    }

    return 0;
}
