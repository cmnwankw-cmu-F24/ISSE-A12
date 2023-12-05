/*
 * expr_whizz.c
 *
 * A recursive descent parser for a simple arithmetic calculator, with
 * the operators + - * / % ^, and unary minus. Values are held as
 * doubles.
 *
 * Author: Nwankwo Chukwunonso Michael
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdbool.h>

int main(int argc, char *argv[])
{
    char *input = NULL;
    bool time_to_quit = false;


    printf("Welcome to Plaid Shell!\n");

    while (!time_to_quit)
    {

        input = readline("\e[01;31m#?\e[00;39m ");
        if (input == NULL || strcasecmp(input, "quit") == 0)
        {
            time_to_quit = true;
            goto loop_end;
        }

        if (*input == '\0') // user just hit enter, no content
            goto loop_end;

        add_history(input);

        printf("%s\n", input);

        // uncomment for more debug info
        // TOK_print(tokens);

    loop_end:
        free(input);
        input = NULL;
    }

    return 0;
}