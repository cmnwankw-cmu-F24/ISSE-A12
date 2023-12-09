/*
 * pipeline.c
 *
 * A dynamically allocated tree to handle arbitrary arithmetic
 * expressions
 *
 * Author: Howdy Pierce <howdy@sleepymoose.net>
 * Co-Author: Nwankwo Chukwunonso Michael
 *
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>

#include "pipeline.h"
#include "clist.h"

struct _pipe_tree_node
{
  PipeNodeType type;
  char *command;
  CList args;
  PipeTree left;
  PipeTree right;
};

/*
 * Convert an ExprNodeType into a printable character
 *
 * Parameters:
 *   ent    The ExprNodeType to convert
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
PipeTree PT_word(const char *command, const char *args[])
{
  // Use malloc to request for a valid block of memory
  PipeTree node = (PipeTree)malloc(sizeof(struct _pipe_tree_node));
  assert(node); // assert a valid block of memory was returned

  // set the type
  node->type = WORD;

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
PipeTree PT_node(PipeNodeType op, PipeTree left, PipeTree right)
{
  // Use malloc to request for a valid block of memory
  PipeTree new = (PipeTree)malloc(sizeof(struct _pipe_tree_node));
  assert(new); // assert a valid block of memory was returned

  // set the type, left and right child of the new node
  new->type = op;
  new->command = NULL;
  new->args = NULL;
  new->left = left;
  new->right = right;

  // return the node
  return new;
}

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

  // if type is not VALUE, make a recursive call
  // to free the LEFT child, then the RIGHT child
  if (tree->type >= CMD_LESS && tree->type <= CMD_PIPE)
  {
    PT_free(tree->left);
    PT_free(tree->right);
  }
  else if (tree->type == WORD)
  {
    // free the memory for command
    free(tree->command);
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

const char *getUserName()
{
  uid_t uid = getuid(); // Get the current user's UID
  struct passwd *pw = getpwuid(uid);
  if (pw)
  {
    return pw->pw_name;
  }

  return "Unknown"; // Fallback in case of an error
}

int executeCommand(char *command, char *const *args)
{
  if (strcmp(command, "exit") == 0 || strcmp(command, "quit") == 0)
  {
    // Terminate the shell
    exit(0);
  }
  else if (strcmp(command, "author") == 0)
  {
    // Print the username of the author of this shell—that is, the current user's name
    const char *username = getUserName();
    printf("%s\n", username);
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
    // Print working directory
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
      printf("%s\n", cwd);
    }
    else
    {
      perror("pwd failed");
      return -1;
    }
    return 0;
  }
  else
  {
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
      // Child process
      execvp(command, args);
      // If execvp returns, it must have failed
      perror("execvp failed");
      exit(EXIT_FAILURE);
    }
    else
    {
      // Parent process
      int status;
      waitpid(pid, &status, 0);
      return WEXITSTATUS(status);
    }
  }
}

int handleRedirectInput(PipeTree tree, char *filePath)
{
  int ifd;
  pid_t pid;
  int status;

  // Open the input filePath
  const int mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
  ifd = open(filePath, O_RDONLY, mode);
  if (ifd == -1)
  {
    perror("plaidsh: Error opening file");
    return -1;
  }

  // Create a child process
  pid = fork();
  if (pid == -1)
  {
    perror("plaidsh: Error forking the children.");
    return -1;
  }

  if (pid == 0)
  {
    // We are in the child process

    // Redirect the standard input to the file
    if (dup2(ifd, STDIN_FILENO) < 0)
    {
      perror("plaidsh: Error redirecting to standard input");
      close(ifd);
      exit(EXIT_FAILURE);
    }

    // close the file descriptor
    close(ifd);

    // Evaluate the left child
    return PT_evaluate(tree->left);
  }
  else
  {

    // We are in the Parent process

    // Close the output file in the parent as it's only needed in the child
    close(ifd);

    // Wait for the child process to finish
    waitpid(pid, &status, 0);
    return WEXITSTATUS(status);
  }
}

int handleRedirectOutput(PipeTree tree, char *filePath)
{
  int ofd;
  pid_t pid;
  int status;

  const int mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
  ofd = open(filePath, O_WRONLY | O_CREAT | O_TRUNC, mode);
  if (ofd == -1)
  {
    perror("plaidsh: Error opening file");
    return -1;
  }

  // Create a child process
  pid = fork();
  if (pid == -1)
  {
    perror("plaidsh: Error forking the child.");
    return -1;
  }

  if (pid == 0)
  {
    // We are in the child process

    // Redirect the standard input to the file
    if (dup2(ofd, STDOUT_FILENO) < 0)
    {
      perror("plaidsh: Error redirecting to standard input");
      close(ofd);
      exit(EXIT_FAILURE);
    }

    // close the file descriptor
    close(ofd);

    // Evaluate the left child
    return PT_evaluate(tree->left);
  }
  else
  {

    // We are in the Parent process

    // Close the output file in the parent as it's only needed in the child
    close(ofd);

    // Wait for the child process to finish
    waitpid(pid, &status, 0);
    return WEXITSTATUS(status);
  }
}

int handlePipe(PipeTree tree)
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
  waitpid(leftPid, NULL, 0);
  waitpid(rightPid, NULL, 0);

  return 0;
}

// Documented in .h file
int PT_evaluate(PipeTree tree)
{

  if (tree->type == WORD)
  {

    size_t size = CL_length(tree->args);
    const char *args[size + 2];

    args[0] = tree->command;

    for (size_t i = 0; i < size; i++)
    {
      args[i + 1] = CL_nth(tree->args, i);
    }

    args[size + 1] = NULL;
    return executeCommand(tree->command, (char *const *)args);
  }
  else if (tree->type == CMD_LESS || tree->type == CMD_GREAT)
  {
    char *file = tree->right->command;

    if (tree->type == CMD_LESS)
    {
      return handleRedirectInput(tree, file);
    }
    return handleRedirectOutput(tree, file);
  }
  else if (tree->type == CMD_PIPE)
  {
    return handlePipe(tree);
  }

  return -1;
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
    safe_strcat(buf, " ", buf_sz);
    safe_strcat(buf, tree->command, buf_sz);
    size_t size = CL_length(tree->args);

    for (size_t i = 0; i < size; i++)
    {
      safe_strcat(buf, " ", buf_sz);
      safe_strcat(buf, (char *)CL_nth(tree->args, i), buf_sz);
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
    // safe_strcat(buf, " ", buf_sz);
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

int PT_set_args(PipeTree tree, const char * arg){
  if(tree->args == NULL){
    tree->args = CL_new();
  }

  CL_append(tree->args, arg);
  return 0;
}

// int main()
// {

//   const char cmd[] = "cat";
//   const char *args1[] = {"maribu.txt", NULL};
//   char buf[128];

//   PipeTree pip = PT_word(cmd, args1);

  // pip = PT_node(CMD_LESS, pip, NULL);

  // int ret = PT_evaluate(pip);
  // PT_tree2string(pip, buf, 128);
  // printf("%s \n.", buf);

//   const char cmd2[] = "grep";
//   const char *args[] = {"love", NULL};

//   PipeTree pip2 = PT_word(cmd2, args);
//   // PT_tree2string(pip, buf, 128);
//   // printf("%s \n", buf);

//   // ret = PT_evaluate(pip);

//   pip = PT_node(CMD_PIPE, pip, pip2);
//   PT_tree2string(pip, buf, 128);
//   printf("%s \n", buf);
//   int ret = PT_evaluate(pip);
//   PT_free(pip);

//   return 0;
// }
