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

int main(int argc, char *argv[])
{
    char *input = NULL;
    bool time_to_quit = false;
    char errmsg[128] = {'\0'};
    // char errmsg2[128] = {'\0'};

    printf("Welcome to Plaid Shell!\n");

    while (!time_to_quit)
    {

        input = readline("\n\e[01;31m#?\e[00;39m ");
        if (input == NULL || strcasecmp(input, "quit") == 0)
        {
            time_to_quit = true;
            goto loop_end;
        }

        if (*input == '\0' || feof(stdin)) // user just hit enter, no content
            goto loop_end;

        add_history(input);

        // printf("%s\n", input);

        TList tokens = TOK_tokenize_input(input, errmsg, sizeof(errmsg));

        // uncomment for more debug info
        // TOK_print(tokens);
        // printf("sectioning things");

        if (tokens == NULL)
        {
            fprintf(stderr, "%s\n", errmsg);
            goto loop_end;
        }

        if (TL_length(tokens) == 0)
            goto loop_end;

        PipeTree tree = Parse(tokens, errmsg, sizeof(errmsg));


        // PT_tree2string(tree, errmsg2, sizeof(errmsg2));
        // printf("%s", errmsg2);

        // evaluate the tree

        if (tree == NULL)
        {
            fprintf(stderr, "%s\n", errmsg);
            goto loop_end;
        }

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
