#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h> // strlen
#include <ctype.h>  // isblank
#include <math.h>   // fabs
#include <stdbool.h>

#include "token.h"
#include "tokenize.h"
#include "parse.h"
#include "pipeline.h"

// Checks that value is true; if not, prints a failure message and
// returns 0 from this function
#define test_assert(value)                                               \
    {                                                                    \
        if (!(value))                                                    \
        {                                                                \
            printf("FAIL %s[%d]: %s\n", __FUNCTION__, __LINE__, #value); \
            goto test_error;                                             \
        }                                                                \
    }

#define MAX_TOKENS 128

// Test case data struct
typedef struct
{
    const char *command;
    const char **args;
    int arg_sz;
    const char *input_file;
    const char *output_file;
    Token tokens[MAX_TOKENS];
} PTest;

/*
 * Tests the TOK_tokenize_input function
 *
 * Returns: 1 if all tests pass, 0 otherwise
 */
int test_tokenization()
{
    typedef struct
    {
        const char *string;
        const Token exp_tokens[20];
    } test_matrix_t;

    test_matrix_t tests[] =
        {
            // from writeup examples
            {"echo a b", {{.type = TOK_WORD, .word = "echo"}, {.type = TOK_WORD, .word = "a"}, {.type = TOK_WORD, .word = "b"}, {.type = TOK_END}}},
            {"echo a\\ b", {{.type = TOK_WORD, .word = "echo"}, {.type = TOK_WORD, .word = "a b"}, {TOK_END}}},
            {"echo \"a b\"", {{TOK_WORD, .word = "echo"}, {TOK_QUOTED_WORD, .word = "a b"}, {TOK_END}}},
            {"echo a\\\\ b", {{TOK_WORD, .word = "echo"}, {TOK_WORD, .word = "a\\"}, {TOK_WORD, .word = "b"}, {TOK_END}}},
            {"echo hello|grep \"ell\"", {{TOK_WORD, .word = "echo"}, {TOK_WORD, .word = "hello"}, {TOK_PIPE}, {TOK_WORD, .word = "grep"}, {TOK_QUOTED_WORD, .word = "ell"}, {TOK_END}}},
            {"echo hello\\|grep \"ell\"", {{TOK_WORD, .word = "echo"}, {TOK_WORD, .word = "hello|grep"}, {TOK_QUOTED_WORD, .word = "ell"}, {TOK_END}}},
            {"echo boo >out_file", {{TOK_WORD, .word = "echo"}, {TOK_WORD, .word = "boo"}, {TOK_GREATERTHAN}, {TOK_WORD, .word = "out_file"}, {TOK_END}}},
            {"echo \"boo\" >out_file", {{TOK_WORD, .word = "echo"}, {TOK_QUOTED_WORD, .word = "boo"}, {TOK_GREATERTHAN}, {TOK_WORD, .word = "out_file"}, {TOK_END}}},
            {"", {{TOK_END}}},
            {"echo \"hello | grep\"", {{TOK_WORD, .word = "echo"}, {TOK_QUOTED_WORD, .word = "hello | grep"}, {TOK_END}}},
            {"echo a\"b c\"", {{TOK_WORD, .word = "echo"}, {TOK_WORD, .word = "a"}, {TOK_QUOTED_WORD, .word = "b c"}, {TOK_END}}},
            // unescaped whitespaces
            {"", {{TOK_END}}},
            {"\t   \n \r  \t \t", {{TOK_END}}},
            {"     \t", {{TOK_END}}},
            // escape sequences in regular word
            {"cat \\| next.txt", {{TOK_WORD, .word = "cat"}, {TOK_WORD, .word = "|"}, {TOK_WORD, .word = "next.txt"}, {TOK_END}}},
            {"cat \\> next.txt", {{TOK_WORD, .word = "cat"}, {TOK_WORD, .word = ">"}, {TOK_WORD, .word = "next.txt"}, {TOK_END}}},
            {"cat \\< next.txt", {{TOK_WORD, .word = "cat"}, {TOK_WORD, .word = "<"}, {TOK_WORD, .word = "next.txt"}, {TOK_END}}},
            {"cat \\\" next.txt", {{TOK_WORD, .word = "cat"}, {TOK_WORD, .word = "\""}, {TOK_WORD, .word = "next.txt"}, {TOK_END}}},
            {"cat\\ next.txt", {{TOK_WORD, .word = "cat next.txt"}, {TOK_END}}},
            {"cat\\nnext.txt", {{TOK_WORD, .word = "cat\nnext.txt"}, {TOK_END}}},
            {"cat\\rnext.txt", {{TOK_WORD, .word = "cat\rnext.txt"}, {TOK_END}}},
            {"cat\\tnext.txt", {{TOK_WORD, .word = "cat\tnext.txt"}, {TOK_END}}},
            // escape sequences in quoted word
            {"sed \"math\\| file\"", {{TOK_WORD, .word = "sed"}, {TOK_QUOTED_WORD, .word = "math| file"}, {TOK_END}}},
            {"sed \"math\\> file\"", {{TOK_WORD, .word = "sed"}, {TOK_QUOTED_WORD, .word = "math> file"}, {TOK_END}}},
            {"sed \"math\\< file\"", {{TOK_WORD, .word = "sed"}, {TOK_QUOTED_WORD, .word = "math< file"}, {TOK_END}}},
            {"sed \"math\\\" file\"", {{TOK_WORD, .word = "sed"}, {TOK_QUOTED_WORD, .word = "math\" file"}, {TOK_END}}},
            {"sed \"math\\ file\"", {{TOK_WORD, .word = "sed"}, {TOK_QUOTED_WORD, .word = "math file"}, {TOK_END}}},
            {"sed \"math\\nfile\"", {{TOK_WORD, .word = "sed"}, {TOK_QUOTED_WORD, .word = "math\nfile"}, {TOK_END}}},
            {"sed \"math\\rfile\"", {{TOK_WORD, .word = "sed"}, {TOK_QUOTED_WORD, .word = "math\rfile"}, {TOK_END}}},
            {"sed \"math\\tfile\"", {{TOK_WORD, .word = "sed"}, {TOK_QUOTED_WORD, .word = "math\tfile"}, {TOK_END}}},
            // 8 character word test
            {"echo abcdefgh", {{TOK_WORD, .word = "echo"}, {TOK_WORD, .word = "abcdefgh"}, {TOK_END}}},
            {"echo \"abcdefgh\"", {{TOK_WORD, .word = "echo"}, {TOK_QUOTED_WORD, .word = "abcdefgh"}, {TOK_END}}},
            // 16 character word test
            {"touch abcdefghijklmnop", {{TOK_WORD, .word = "touch"}, {TOK_WORD, .word = "abcdefghijklmnop"}, {TOK_END}}},
            {"touch \"abcdefghijklmnop\"", {{TOK_WORD, .word = "touch"}, {TOK_QUOTED_WORD, .word = "abcdefghijklmnop"}, {TOK_END}}},
            // 24 character word test
            {"mkdir abcdefghijklmnopqrstuvwx", {{TOK_WORD, .word = "mkdir"}, {TOK_WORD, .word = "abcdefghijklmnopqrstuvwx"}, {TOK_END}}},
            {"mkdir \"abcdefghijklmnopqrstuvwx\"", {{TOK_WORD, .word = "mkdir"}, {TOK_QUOTED_WORD, .word = "abcdefghijklmnopqrstuvwx"}, {TOK_END}}},
            // 32 character word test
            {"ls -l abcdefghijklmnopqrstuvwx12345678", {{TOK_WORD, .word = "ls"}, {TOK_WORD, .word = "-l"}, {TOK_WORD, .word = "abcdefghijklmnopqrstuvwx12345678"}, {TOK_END}}},
            {"ls -l \"abcdefghijklmnopqrstuvwx12345678\"", {{TOK_WORD, .word = "ls"}, {TOK_WORD, .word = "-l"}, {TOK_QUOTED_WORD, .word = "abcdefghijklmnopqrstuvwx12345678"}, {TOK_END}}},
            // redirections and pipes
            {"<", {{TOK_LESSTHAN}, {TOK_END}}},
            {">", {{TOK_GREATERTHAN}, {TOK_END}}},
            {"|", {{TOK_PIPE}, {TOK_END}}},
            {">><<", {{TOK_GREATERTHAN}, {TOK_GREATERTHAN}, {TOK_LESSTHAN}, {TOK_LESSTHAN}, {TOK_END}}},
            {">>|<<", {{TOK_GREATERTHAN}, {TOK_GREATERTHAN}, {TOK_PIPE}, {TOK_LESSTHAN}, {TOK_LESSTHAN}, {TOK_END}}},
            // all tokens
            {"echo \"Hello\\tWorld\\n\" > output.txt | cat < output.txt | grep \"Hello\\tWorld\\n\"", {{TOK_WORD, .word = "echo"}, {TOK_QUOTED_WORD, .word = "Hello\tWorld\n"}, {TOK_GREATERTHAN}, {TOK_WORD, .word = "output.txt"}, {TOK_PIPE}, {TOK_WORD, .word = "cat"}, {TOK_LESSTHAN}, {TOK_WORD, .word = "output.txt"}, {TOK_PIPE}, {TOK_WORD, .word = "grep"}, {TOK_QUOTED_WORD, .word = "Hello\tWorld\n"}, {TOK_END}}}};
    const int num_tests = sizeof(tests) / sizeof(test_matrix_t);
    char errmsg[128] = {'\0'};

    TList list;

    for (int i = 0; i < num_tests; i++)
    {
        list = TOK_tokenize_input(tests[i].string, errmsg, sizeof(errmsg));
        for (int t = 0; tests[i].exp_tokens[t].type != TOK_END; t++)
        {
            test_assert(TOK_next_type(list) == tests[i].exp_tokens[t].type);
            if (TOK_next_type(list) == TOK_WORD || TOK_next_type(list) == TOK_QUOTED_WORD)
            {
                test_assert(strcmp(TOK_next(list).word, tests[i].exp_tokens[t].word) == 0);
            }
            TOK_consume(list);
        }

        TL_free(list);
        list = NULL;
    }

    // Test erroneous inputs
    
    // Illegal escape sequence in regular word
    test_assert(TOK_tokenize_input("echo \\g", errmsg, sizeof(errmsg)) == NULL);
    test_assert(strcasecmp(errmsg, "Illegal escape character 'g'") == 0);
    
    // Illegal escape sequence in quoted word
    test_assert(TOK_tokenize_input("echo \"me\\c\"", errmsg, sizeof(errmsg)) == NULL);
    test_assert(strcasecmp(errmsg, "Illegal escape character 'c'") == 0);
    
    // Unterminated quotes, in regular word
    test_assert(TOK_tokenize_input("touch \"hacker.txt", errmsg, sizeof(errmsg)) == NULL);
    test_assert(strcasecmp(errmsg, "Unterminated quote") == 0);
    
    // Unterminated quotes, in quoted word
    test_assert(TOK_tokenize_input("echo \"This is \\a test\"", errmsg, sizeof(errmsg)) == NULL);
    test_assert(strcasecmp(errmsg, "Illegal escape character 'a'") == 0);
    
    // Illegal escape character
    test_assert(TOK_tokenize_input("\\", errmsg, sizeof(errmsg)) == NULL);
    test_assert(strcasecmp(errmsg, "Illegal escape character '") == 0);

    // Illegal escape character
    test_assert(TOK_tokenize_input("echo \"This is a test\\", errmsg, sizeof(errmsg)) == NULL);
    test_assert(strcasecmp(errmsg, "Illegal escape character '") == 0);

    return 1;

test_error:
    TL_free(list);
    return 0;
}

/*
 * Tests the recursive descent parser
 *
 * Returns: 1 if all tests pass, 0 otherwise
 */

int test_parsing()
{
    PTest test_cases[] = {
        // echo
        {"echo", NULL, 0, NULL, NULL, {{TOK_WORD, .word = "echo"}, {TOK_END}}},
        // ls -l
        {"ls", (const char *[]){"-l", NULL}, 1, NULL, NULL, {{TOK_WORD, .word = "ls"}, {TOK_WORD, .word = "-l"}, {TOK_END}}},
        // grep pattern < input.txt
        {"grep", (const char *[]){"pattern", NULL}, 1, "input.txt", NULL, {{TOK_WORD, .word = "grep"}, {TOK_WORD, .word = "pattern"}, {TOK_LESSTHAN}, {TOK_WORD, .word = "input.txt"}, {TOK_END}}},
        // cat file1 file2 > output.txt
        {"cat", (const char *[]){"file1", "file2", NULL}, 2, NULL, "output.txt", {{TOK_WORD, .word = "cat"}, {TOK_WORD, .word = "file1"}, {TOK_WORD, .word = "file2"}, {TOK_GREATERTHAN}, {TOK_WORD, .word = "output.txt"}, {TOK_END}}},
        // mkdir newdir
        {"mkdir", (const char *[]){"newdir", NULL}, 1, NULL, NULL, {{TOK_WORD, .word = "mkdir"}, {TOK_WORD, .word = "newdir"}, {TOK_END}}},
        // touch newfile.txt
        {"touch", (const char *[]){"newfile.txt", NULL}, 1, NULL, NULL, {{TOK_WORD, .word = "touch"}, {TOK_WORD, .word = "newfile.txt"}, {TOK_END}}},
        // rm -rf oldfolder
        {"rm", (const char *[]){"-rf", "oldfolder", NULL}, 2, NULL, NULL, {{TOK_WORD, .word = "rm"}, {TOK_WORD, .word = "-rf"}, {TOK_WORD, .word = "oldfolder"}, {TOK_END}}},
        // find ./ name *.txt
        {"find", (const char *[]){"./", "-name", "*.txt", NULL}, 3, NULL, NULL, {{TOK_WORD, .word = "find"}, {TOK_WORD, .word = "./"}, {TOK_WORD, .word = "-name"}, {TOK_WORD, .word = "*.txt"}, {TOK_END}}},
        // tar -czf archive.tar.gz folder
        {"tar", (const char *[]){"-czf", "archive.tar.gz", "folder", NULL}, 3, NULL, NULL, {{TOK_WORD, .word = "tar"}, {TOK_WORD, .word = "-czf"}, {TOK_WORD, .word = "archive.tar.gz"}, {TOK_WORD, .word = "folder"}, {TOK_END}}},
        // wc -l file.txt
        {"wc", (const char *[]){"-l", "file.txt", NULL}, 2, NULL, NULL, {{TOK_WORD, .word = "wc"}, {TOK_WORD, .word = "-l"}, {TOK_WORD, .word = "file.txt"}, {TOK_END}}},
        // curl -O http://example.com/file
        {"curl", (const char *[]){"-O", "http://example.com/file", NULL}, 2, NULL, NULL, {{TOK_WORD, .word = "curl"}, {TOK_WORD, .word = "-O"}, {TOK_WORD, .word = "http://example.com/file"}, {TOK_END}}},
        // chmod +x script.sh
        {"chmod", (const char *[]){"+x", "script.sh", NULL}, 2, NULL, NULL, {{TOK_WORD, .word = "chmod"}, {TOK_WORD, .word = "+x"}, {TOK_WORD, .word = "script.sh"}, {TOK_END}}},
        // ping -c 4 example.com
        {"ping", (const char *[]){"-c", "4", "example.com", NULL}, 3, NULL, NULL, {{TOK_WORD, .word = "ping"}, {TOK_WORD, .word = "-c"}, {TOK_WORD, .word = "4"}, {TOK_WORD, .word = "example.com"}, {TOK_END}}},
        // sort -r data.txt > sorted.txt
        {"sort", (const char *[]){"-r", "data.txt", NULL}, 2, NULL, "sorted.txt", {{TOK_WORD, .word = "sort"}, {TOK_WORD, .word = "-r"}, {TOK_WORD, .word = "data.txt"}, {TOK_GREATERTHAN}, {TOK_WORD, .word = "sorted.txt"}, {TOK_END}}},
        // diff file1.txt file2.txt
        {"diff", (const char *[]){"file1.txt", "file2.txt", NULL}, 2, NULL, NULL, {{TOK_WORD, .word = "diff"}, {TOK_WORD, .word = "file1.txt"}, {TOK_WORD, .word = "file2.txt"}, {TOK_END}}},
        // cut -d : -f 1 file.txt
        {"cut", (const char *[]){"-d", ":", "-f", "1", "file.txt", NULL}, 5, NULL, NULL, {{TOK_WORD, .word = "cut"}, {TOK_WORD, .word = "-d"}, {TOK_WORD, .word = ":"}, {TOK_WORD, .word = "-f"}, {TOK_WORD, .word = "1"}, {TOK_WORD, .word = "file.txt"}, {TOK_END}}},
        // env
        {"env", NULL, 0, NULL, NULL, {{TOK_WORD, .word = "env"}, {TOK_END}}},
        // head -n 10 log.txt
        {"head", (const char *[]){"-n", "10", "log.txt", NULL}, 3, NULL, NULL, {{TOK_WORD, .word = "head"}, {TOK_WORD, .word = "-n"}, {TOK_WORD, .word = "10"}, {TOK_WORD, .word = "log.txt"}, {TOK_END}}},
        // "grep pattern file.txt | sort | uniq"
        {NULL, NULL, 0, NULL, NULL, {{TOK_WORD, .word = "grep"}, {TOK_WORD, .word = "pattern"}, {TOK_WORD, .word = "file.txt"}, {TOK_PIPE}, {TOK_WORD, .word = "sort"}, {TOK_PIPE}, {TOK_WORD, .word = "uniq"}, {TOK_END}}},
        // cat input.txt | sort | uniq > sorted.txt
        {NULL, NULL, 0, NULL, NULL, {{TOK_WORD, .word = "cat"}, {TOK_WORD, .word = "input.txt"}, {TOK_PIPE}, {TOK_WORD, .word = "sort"}, {TOK_PIPE}, {TOK_WORD, .word = "uniq"}, {TOK_GREATERTHAN}, {TOK_WORD, .word = "sorted.txt"}, {TOK_END}}},
        // "awk '{print $1}' input.txt | grep -v error | sort > output.txt
        {NULL, NULL, 0, NULL, NULL, {{TOK_WORD, .word = "awk"}, {TOK_QUOTED_WORD, .word = "'{print $1}'"}, {TOK_WORD, .word = "input.txt"}, {TOK_PIPE}, {TOK_WORD, .word = "grep"}, {TOK_WORD, .word = "-v"}, {TOK_WORD, .word = "error"}, {TOK_PIPE}, {TOK_WORD, .word = "sort"}, {TOK_GREATERTHAN}, {TOK_WORD, .word = "output.txt"}, {TOK_END}}}

    };

    const int num_tests = sizeof(test_cases) / sizeof(PTest);
    char errmsg[128];
    // run the tests
    for (int i = 0; i < num_tests; i++)
    {
        // get the tokens
        TList tokens = TL_new();

        for (int j = 0; test_cases[i].tokens[j].type != TOK_END; j++)
        {
            Token token;
            token.type = test_cases[i].tokens[j].type;
            if (token.type == TOK_WORD || token.type == TOK_QUOTED_WORD)
            {
                token.word = strdup(test_cases[i].tokens[j].word);
                assert(token.word);
            }
            TL_append(tokens, token);
        }

        PipeTree tree = Parse(tokens, errmsg, sizeof(errmsg));
        test_assert(test_pipeline(tree, test_cases[i].command, test_cases[i].args, test_cases[i].arg_sz, test_cases[i].input_file, test_cases[i].output_file));

        TOK_free(tokens);
        PT_free(tree);
    }

    // Test erroneous parse cases
    TList tokens;
    PipeTree tree;

    // No file name after redirection >
    tokens = TOK_tokenize_input("cat >", errmsg, sizeof(errmsg));
    tree = Parse(tokens, errmsg, sizeof(errmsg));
    test_assert(tree == NULL);
    test_assert(strcmp(errmsg, "Expect filename after redirection") == 0);
    TOK_free(tokens);
    PT_free(tree);

    // No file name after redirection <
    tokens = TOK_tokenize_input("less < ", errmsg, sizeof(errmsg));
    tree = Parse(tokens, errmsg, sizeof(errmsg));
    test_assert(tree == NULL);
    test_assert(strcmp(errmsg, "Expect filename after redirection") == 0);
    TOK_free(tokens);
    PT_free(tree);

    // No command specified |
    tokens = TOK_tokenize_input("|", errmsg, sizeof(errmsg));
    tree = Parse(tokens, errmsg, sizeof(errmsg));
    test_assert(tree == NULL);
    test_assert(strcmp(errmsg, "No command specified") == 0);
    TOK_free(tokens);
    PT_free(tree);

    // No command specified >
    tokens = TOK_tokenize_input(">", errmsg, sizeof(errmsg));
    tree = Parse(tokens, errmsg, sizeof(errmsg));
    test_assert(tree == NULL);
    test_assert(strcmp(errmsg, "No command specified") == 0);
    TOK_free(tokens);
    PT_free(tree);

    // No command specified <
    tokens = TOK_tokenize_input("|", errmsg, sizeof(errmsg));
    tree = Parse(tokens, errmsg, sizeof(errmsg));
    test_assert(tree == NULL);
    test_assert(strcmp(errmsg, "No command specified") == 0);
    TOK_free(tokens);
    PT_free(tree);

    // Multiple redirections cat > cat >
    tokens = TOK_tokenize_input("cat > cat >", errmsg, sizeof(errmsg));
    tree = Parse(tokens, errmsg, sizeof(errmsg));
    test_assert(tree == NULL);
    test_assert(strcmp(errmsg, "Multiple redirection") == 0);
    TOK_free(tokens);
    PT_free(tree);

    // Multiple redirections echo < echo <
    tokens = TOK_tokenize_input("cat > cat >", errmsg, sizeof(errmsg));
    tree = Parse(tokens, errmsg, sizeof(errmsg));
    test_assert(tree == NULL);
    test_assert(strcmp(errmsg, "Multiple redirection") == 0);
    TOK_free(tokens);
    PT_free(tree);

    // No command specified sed |
    tokens = TOK_tokenize_input("sed |", errmsg, sizeof(errmsg));
    tree = Parse(tokens, errmsg, sizeof(errmsg));
    test_assert(tree == NULL);
    test_assert(strcmp(errmsg, "No command specified") == 0);
    TOK_free(tokens);
    PT_free(tree);

    // No command specified | sed
    tokens = TOK_tokenize_input("| sed", errmsg, sizeof(errmsg));
    tree = Parse(tokens, errmsg, sizeof(errmsg));
    test_assert(tree == NULL);
    test_assert(strcmp(errmsg, "No command specified") == 0);
    TOK_free(tokens);
    PT_free(tree);

    // No command specified  echo || real_file.txt
    tokens = TOK_tokenize_input(" echo || real_file.txt", errmsg, sizeof(errmsg));
    tree = Parse(tokens, errmsg, sizeof(errmsg));
    test_assert(tree == NULL);
    test_assert(strcmp(errmsg, "No command specified") == 0);
    TOK_free(tokens);
    PT_free(tree);

    return 1;

test_error:
    return 0;
}

int main()
{
    int passed = 0;
    int num_tests = 0;

    num_tests++;
    passed += test_tokenization();
    num_tests++;
    passed += test_parsing();

    printf("Passed all test cases for \e[01;35mTokenizing\e[01;39m and \e[01;33mParsing\e[01;39m %d/%d\n", passed, num_tests);

    fflush(stdout);
    return 0;
}