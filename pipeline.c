/*
 * pipeline.c
 *
 * A dynamically allocated tree to handle arbitrary shell
 * command expression
 *
 * Author: Nwankwo Chukwunonso Michael
 *
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include <errno.h>

#include "pipeline.h"
#include "clist.h"

static int handlePipe(PipeTree tree);
static int executeCommand(char *command, char *const *args, const char *in, const char *out);

struct _pipe_tree_node
{
  PipeNodeType type;
  char *command;
  char *input;
  char *output;
  CList args;
  PipeTree left;
  PipeTree right;
};

/*
 * Convert an PipeNodeType into a printable character
 *
 * Parameters:
 *   ent    The PipeNodeType to convert
 *
 * Returns: A single character representing the ent
 */
static char PipeNodeType_to_char(PipeNodeType ent)
{

  switch (ent)
  {
  case CMD_PIPE:
    return '|';
  case CMD_LESS:
    return '<';
  case CMD_GREAT:
    return '>';
  default:
    return '?';
  }
}

// Documented in .h file
int setInputFiles(PipeTree tree, const char *in)
{
  // handle NULL tree situation
  if (tree == NULL)
  {
    return -1;
  }

  // Copy input filename string
  tree->input = strdup(in);
  assert(tree->input); // assert not null

  return 0; // return 0 on SUCCESS
}

// Documented in .h file
int setOutputFiles(PipeTree tree, const char *out)
{
  // handle NULL tree situation
  if (tree == NULL)
  {
    return -1;
  }

  // Copy input filename string
  tree->output = strdup(out);
  assert(tree->output); // assert not null

  return 0; // return 0 on SUCCESS
}

// Documented in .h file
PipeTree PT_word(const char *command, const char *args[])
{

  // Use malloc to request for a valid block of memory
  PipeTree node = (PipeTree)malloc(sizeof(struct _pipe_tree_node));
  assert(node); // assert a valid block of memory was returned

  // set the type
  node->type = WORD;

  // set the input/output file to NULL
  node->input = NULL;
  node->output = NULL;

  // set the command
  node->command = strdup(command);
  assert(node->command); // assert a valid block of memory was returned

  // first set the args to NULL
  if (args == NULL || args[0] == NULL)
  {
    node->args = NULL;
  }
  else
  {
    // we need to add a linkedlist to the node;
    node->args = CL_new();

    size_t idx = 0;

    // loop through the strings and append to CList
    while (args[idx] != NULL)
    {
      CListElementType element = strdup(args[idx]);
      CL_append(node->args, element);
      idx++;
    }
  }

  // set the left and right to NULL
  node->left = NULL;
  node->right = NULL;

  // return the node
  return node;
}

// Documented in .h file
PipeTree PT_pipe(PipeTree left, PipeTree right)
{
  // Use malloc to request for a valid block of memory
  PipeTree new = (PipeTree)malloc(sizeof(struct _pipe_tree_node));
  assert(new); // assert a valid block of memory was returned

  // set the type, left and right child of the new node
  new->type = CMD_PIPE;

  // left and right child
  new->left = left;
  new->right = right;

  // NULL these attributes
  new->command = NULL;
  new->args = NULL;
  new->input = NULL;
  new->output = NULL;

  // return the node
  return new;
}

/**
 * Callback to free a command argument.
 *
 * Called by foreach used in PT_free to
 * free each argument string.
 * Parameters
 *    pos - Position in argument list
 *    arg - Argument string to free
 *    cb_data - Optional callback data (unused)
 */

void PT_free_args_callback(int pos, CListElementType arg, void *cb_data)
{
  free((void *)arg);
  arg = NULL;
}

// Documented in .h file
void PT_free(PipeTree tree)
{
  // Base Case: do nothing, just return
  if (tree == NULL)
  {
    return;
  }

  // if type is of type CMD_PIPE, make a recursive call
  // to free the LEFT child, then the RIGHT child
  if (tree->type == CMD_PIPE)
  {
    PT_free(tree->left);
    PT_free(tree->right);
  }
  else if (tree->type == WORD)
  {
    // free the files
    free((void *)tree->input);
    tree->input = NULL;

    free((void *)tree->output);
    tree->output = NULL;

    // free the memory for command
    free((void *)tree->command);
    tree->command = NULL;

    // free the linkedlist, if args is not NULL
    if (tree->args != NULL)
    {
      // free the memory allocated for the element in the AST
      CL_foreach(tree->args, PT_free_args_callback, NULL);

      CL_free(tree->args);
      tree->args = NULL;
    }
  }

  // Free the node itself, after the children have been freed.
  free(tree);
  tree = NULL;
  return;
}

// Documented in .h file
int PT_count(PipeTree tree)
{
  if (tree == NULL)
    return 0;

  if (tree->type == WORD)
    return 1;

  return 1 + PT_count(tree->left) + PT_count(tree->right);
}

// Documented in .h file
int PT_depth(PipeTree tree)
{
  // Base case 1: If NULL, return 0 as a NULL node contributes nothing to the depth
  if (tree == NULL)
    return 0;

  // Base case 2: If of type VALUE or SYMBOL, return 1
  if (tree->type == WORD)
    return 1;

  //
  // Recursive Operation
  //

  int left = PT_depth(tree->left);   // get the depth of the left child
  int right = PT_depth(tree->right); // get the depth of the right child

  // return 1 plus the greater of left and right, or any if left is equal to right
  return 1 + (left > right ? left : right);
}

// Documented in .h file
int PT_evaluate(PipeTree tree)
{

  // check if the node to be evaluated is a command
  if (tree->type == WORD)
  {

    // data preprocessing, get the arguments and command ready for execution
    size_t size = CL_length(tree->args);
    const char *args[size + 2];

    args[0] = tree->command;

    for (size_t i = 0; i < size; i++)
    {
      args[i + 1] = CL_nth(tree->args, i);
    }

    args[size + 1] = NULL;
    return executeCommand(tree->command, (char *const *)args, tree->input, tree->output);
  }

  // handle the pipe
  return handlePipe(tree);
}

static int redirectSTDOUT(int *ofd, int *original_stdout, const char *filePath)
{
  const int mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
  *ofd = open(filePath, O_WRONLY | O_CREAT | O_TRUNC, mode);
  if (*ofd < 0)
  {
    if (errno == EACCES)
    {
      // Permission denied
      fprintf(stderr, "%s: Permission denied", filePath);
      return -1;
    }
    else
    {
      // Other errors
      fprintf(stderr, "%s: Error opening file: %s\n", filePath, strerror(errno));
      return -1;
    }
    return -1;
  }

  // Redirect stdout
  *original_stdout = dup(STDOUT_FILENO);
  dup2(*ofd, STDOUT_FILENO);

  return 0;
}

/**
 * Execute a command with arguments.
 * Handles built-in commands and external programs.
 *
 * Parameters
 *    command - a char * that represents the command
 *    args - an array of char * that represents the arguments for the command
 *    in - a char * representing the input filename, if any
 *    out - a char * representing the output filename, if any
 * Returns 0 on success and -1 otherwise
 *
 */
static int executeCommand(char *command, char *const *args, const char *in, const char *out)
{

  // Built-in commands exit, quit
  if (strcmp(command, "exit") == 0 || strcmp(command, "quit") == 0)
  {

    // Terminate the shell
    exit(0);
  }
  else if (strcmp(command, "author") == 0)
  {
    int ofd;
    int original_stdout;

    // check if we have to redirect the stdout
    if (out != NULL)
    {
      int status = redirectSTDOUT(&ofd, &original_stdout, out);
      if (status == -1)
        return -1;
    }

    // Print the username of the author of this shell
    fprintf(stdout, "Michael C. Nwankwo");

    if (out != NULL)
    {
      // Revert stdout back
      fflush(stdout); // flush buffered content
      dup2(original_stdout, STDOUT_FILENO);
      close(original_stdout);
    }
    return 0;
  }
  else if (strcmp(command, "cd") == 0)
  {
    // Change directory
    if (args[1] == NULL || strcmp(args[1], "~") == 0)
    {
      chdir(getenv("HOME"));
    }
    else
    {
      if (chdir(args[1]) != 0)
      {
        perror("cd failed");
        return -1;
      }
    }
    return 0;
  }
  else if (strcmp(command, "pwd") == 0)
  {

    int ofd;
    int original_stdout;
    // check if we have to redirect the stdout
    if (out != NULL)
    {
      // check if we have to redirect the stdout
      int status = redirectSTDOUT(&ofd, &original_stdout, out);
      if (status == -1)
        return -1;
    }

    // Print working directory
    char cwd[4096];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
      printf("%s\n", cwd);
    }
    else
    {
      perror("pwd failed");
      return -1;
    }

    if (out != NULL)
    {
      // Revert stdout back
      fflush(stdout); // flush buffered content
      dup2(original_stdout, STDOUT_FILENO);
      close(original_stdout);
    }

    return 0;
  }
  else
  {
    int ifd;
    int ofd;

    if (in != NULL)
    {

      // Open the input filePath
      const int mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
      ifd = open(in, O_RDONLY, mode);
      if (ifd == -1)
      {
        perror("plaidsh: Error opening file");
        return -1;
      }
    }

    if (out != NULL)
    {
      const int mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
      ofd = open(out, O_WRONLY | O_CREAT | O_TRUNC, mode);
      if (ofd < 0)
      {
        if (errno == EACCES)
        {
          // Permission denied
          fprintf(stderr, "%s: Permission denied", out);
        }
        else
        {
          // Other errors
          fprintf(stderr, "%s: Error opening file: %s\n", out, strerror(errno));
        }
        return -1;
      }
    }

    // Handle external commands
    pid_t pid = fork();
    if (pid == -1)
    {
      // Fork failed
      perror("fork failed");
      return -1;
    }
    else if (pid == 0)
    {

      if (in != NULL)
      {
        dup2(ifd, STDIN_FILENO);
        close(ifd);
      }

      if (out != NULL)
      {
        dup2(ofd, STDOUT_FILENO);
        close(ofd);
      }
      // Child process
      execvp(command, args);
      // If execvp returns, it must have failed

      exit(EXIT_FAILURE);
    }
    else
    {
      // Parent process
      int status;
      waitpid(pid, &status, 0);

      // error handling
      if (status != 0)
      {
        fprintf(stderr, "%s: Command not found\n", command);
        fprintf(stderr, "Child %u exited with status 2\n", pid);
      }

      // handle file descriptors in the parent
      if (in != NULL)
        close(ifd);
      if (out != NULL)
        close(ofd);

      return WEXITSTATUS(status);
    }
  }
}

static int handlePipe(PipeTree tree)
{
  int pipefd[2];
  pid_t leftPid, rightPid;

  if (pipe(pipefd) == -1)
  {
    perror("plaidsh: Error creating pipe");
    return -1;
  }

  leftPid = fork();
  if (leftPid == -1)
  {
    perror("plaidsh: Error forking the child");
    return -1;
  }
  else if (leftPid == 0)
  {
    // Left child writes to pipe

    close(pipefd[0]); // close the unused read end

    // Redirect the standardout to the write end of the pipe
    if (dup2(pipefd[1], STDOUT_FILENO) == -1)
    {
      perror("plaidsh: Error redirecting pipe write end to standard out");
      return -1;
    }

    // Close the write end
    close(pipefd[1]);

    PT_evaluate(tree->left);
    exit(EXIT_SUCCESS);
  }

  rightPid = fork();
  if (rightPid == -1)
  {
    perror("plaidsh: Error forking the child");
    return -1;
  }
  else if (rightPid == 0)
  {
    // Right child reads from the pipe
    close(pipefd[1]);
    if (dup2(pipefd[0], STDIN_FILENO) == -1)
    {
      perror("plaidsh: Error forking the child");
      return -1;
    }

    close(pipefd[0]); // close the read end of the pipe

    PT_evaluate(tree->right);
    exit(EXIT_SUCCESS);
  }

  // Parent process
  close(pipefd[0]);
  close(pipefd[1]);

  int status1;
  int status2;

  // wait for the child processes
  waitpid(leftPid, &status1, 0);
  waitpid(rightPid, &status2, 0);

  // check for exit status and handle error
  if (status1 != 0)
  {
    fprintf(stderr, "%s: Command not found\n", tree->left->command);
    fprintf(stderr, "Child %u exited with status 2", leftPid);
    return -1;
  }

  // check for exit status and handle error
  if (status2 != 0)
  {
    fprintf(stderr, "%s: Command not found\n", tree->right->command);
    fprintf(stderr, "Child %u exited with status 2", rightPid);
    return -1;
  }

  return 0; // return 0 on success
}

/*
 * Helper function to append a string to the buffer
 * It ensures that the buffer size is not exceeded.
 */
static void safe_strcat(char *dest, const char *src, size_t dest_sz)
{
  size_t len = strlen(dest);
  if (len < dest_sz - 1)
  {
    strncat(dest, src, dest_sz - len - 1);
  }
}

/*
 * Helper function to append a node's details to the buffer.
 */
static void append_node_to_buf(PipeTree tree, char *buf, size_t buf_sz)
{

  if (tree->type != WORD)
  {
    char temp[2] = {PipeNodeType_to_char(tree->type), '\0'};
    safe_strcat(buf, temp, buf_sz);
  }
  else
  {
    // add space
    safe_strcat(buf, " ", buf_sz);

    // add the command
    safe_strcat(buf, tree->command, buf_sz);

    // add the command's argument
    size_t size = CL_length(tree->args);

    for (size_t i = 0; i < size; i++)
    {
      safe_strcat(buf, " ", buf_sz);
      safe_strcat(buf, (char *)CL_nth(tree->args, i), buf_sz);
    }

    // add the redirections
    if (tree->input != NULL)
    {
      safe_strcat(buf, " ", buf_sz);
      safe_strcat(buf, "<", buf_sz);
      safe_strcat(buf, tree->input, buf_sz);
    }

    // add the redirections
    if (tree->output != NULL)
    {
      safe_strcat(buf, " ", buf_sz);
      safe_strcat(buf, ">", buf_sz);
      safe_strcat(buf, tree->input, buf_sz);
    }
  }
}

/*
 * Function to convert the tree to a string.
 * Parameters:
 *   tree    The tree to convert
 *   buf     The buffer to write the string to
 *   buf_sz  The size of the buffer
 */
size_t PT_tree2stringHelper(PipeTree tree, char *buf, size_t buf_sz)
{

  // Base case
  if (tree == NULL)
  {
    return 0;
  }

  // Append left child
  if (tree->left != NULL)
  {
    PT_tree2stringHelper(tree->left, buf, buf_sz);
    safe_strcat(buf, " ", buf_sz);
  }

  // Append current node
  append_node_to_buf(tree, buf, buf_sz);

  // Append right child
  if (tree->right != NULL)
  {
    safe_strcat(buf, " ", buf_sz);
    PT_tree2stringHelper(tree->right, buf, buf_sz);
    safe_strcat(buf, " ", buf_sz);
  }

  return strlen(buf);
}

// Documented in .h file
size_t PT_tree2string(PipeTree tree, char *buf, size_t buf_sz)
{

  // Clear the buffer
  memset(buf, 0, buf_sz);

  // invoke the helper function to convert the ExprTree into a printable ASCII string
  size_t num_of_char_added = PT_tree2stringHelper(tree, buf, buf_sz);

  return num_of_char_added; // return the num of char added
}

// Documented in the .h file
int PT_set_args(PipeTree tree, const char *arg)
{
  if (tree->args == NULL)
  {
    tree->args = CL_new();
  }

  char *element = strdup(arg);
  assert(element);

  CL_append(tree->args, element);
  return 0;
}

// Safe string comparison
bool safe_strcmp(const char *s1, const char *s2)
{
  if (s1 == NULL || s2 == NULL)
  {
    return s1 == s2;
  }
  return strcmp(s1, s2) == 0;
};

// Documented in the .h file
bool test_pipeline(PipeTree tree, const char *expected_command, const char **expected_args, int args_sz, const char *expected_input_file, const char *expected_output_file)
{

  // Check the command, input/output files
  if (!safe_strcmp(tree->command, expected_command))
    return false;
  if (!safe_strcmp(tree->input, expected_input_file))
    return false;
  if (!safe_strcmp(tree->output, expected_output_file))
    return false;

  // Check the args
  size_t size = CL_length(tree->args);
  if (size != args_sz)
    return false;

  for (size_t i = 0; i < size; i++)
  {
    if (!safe_strcmp(CL_nth(tree->args, i), expected_args[i]))
      return false;
  }

  return true;
}
