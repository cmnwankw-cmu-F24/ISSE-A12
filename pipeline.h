/*
 * pipeline.h
 *
 * A dynamically allocated tree to handle arbitrary shell command expressions
 *
 * Author: Nwankwo Chukwunonso Michael
 */

#ifndef _PIPE_TREE_H_
#define _PIPE_TREE_H_

#include <stdio.h>
#include <stdbool.h>

#include "clist.h"

typedef struct _pipe_tree_node *PipeTree;

// Program logic, relies on the order of the enum,
typedef enum
{
  WORD,
  CMD_LESS,
  CMD_GREAT,
  CMD_PIPE
} PipeNodeType;

/**
 * Create a new Pipetree node for a shell command.
 *
 * Allocates a new Pipetree node, sets its type as WORD and sets the
 * command and args of the nodeusing the given command and arguments,
 * and returns the node.
 *
 * Parameters
 *    command - Command string for this node
 *    args - Array of argument strings
 * Returns
 *    New PipeTree node, NULL on failure
 *
 * It is the responsibility of the caller to call PT_free on a tree
 * that contains this node.
 */
PipeTree PT_word(const char *command, const char *args[]);

/*
 * Create an interior node on tree of CMD_PIPE type. An interior node always represents
 * a pipeline.
 *
 * Parameters:
 *   left     Left side of the pipeline
 *   right    Right side of the pipeline
 *
 * Returns: The new tree, which will consist of an interior node with
 *   two children.
 *
 * It is the responsibility of the caller to call PT_free on a tree
 * that contains this leaf
 */
PipeTree PT_pipe(PipeTree left, PipeTree right);

/*
 * Destroy a PipeTree, calling free() on all malloc'd memory
 *
 * Parameters:
 *   tree     The tree
 *
 * Returns: None
 */
void PT_free(PipeTree tree);

/*
 * Return the number of nodes in the tree, including both leaf and
 * interior nodes in the count.
 *
 * Parameters:
 *   tree     The tree
 *
 * Returns: The number of nodes
 */
int PT_count(PipeTree tree);

/*
 * Return the maximum depth for the tree. A tree that contains just a
 * single leaf node has a depth of 1.
 *
 * Parameters:
 *   tree     The tree
 *
 * Returns: The maximum depth.
 */
int PT_depth(PipeTree tree);

/*
 * Evaluate an ExprTree and return the resulting value
 *
 * Parameters:
 *     tree The tree to compute
 *     errmsg Return space for an error message, filled in in case of error
 *     errmsg_sz The size of errmsg
 *
 * Returns: 0 on success and -1 otherwise
 */
int PT_evaluate(PipeTree tree);

/*
 * Convert an ExprTree into a printable ASCII string stored in buf
 *
 * Parameters:
 *   tree     The tree
 *   buf      The buffer
 *   buf_sz   Size of buffer, in b
 *
 * Returns: The number of characters written to buf, not counting the
 * \0 terminator.
 */
size_t PT_tree2string(PipeTree tree, char *buf, size_t buf_sz);

/**
 * Add new argument to a pipeline tree node's arguments.
 *
 * Allocates an argument list if not already present,
 * then appends a copy of the given argument string.
 *
 * Parameters
 *    tree - Pipeline node to add arg to
 *    arg - Argument string to append
 * Return 0 on success, non-zero on failure
 */

int PT_set_args(PipeTree tree, const char *arg);

/**
 * Set output file for a pipeline tree node.
 *
 * Copies the given filename to the output file
 * pointer in the pipeline node.
 *
 * Parameters
 *    tree - Pipeline tree node
 *    out - Input filename
 *
 * Returns 0 on success, -1 on failure
 */

int setOutputFiles(PipeTree tree, const char *out);

/**
 * Set input file for a pipeline tree node.
 *
 * Copies the given filename to the input file
 * pointer in the pipeline node.
 *
 * Parameters
 *    tree - Pipeline tree node
 *    in - Input filename
 *
 * Returns 0 on success, -1 on failure
 */
int setInputFiles(PipeTree tree, const char *in);

/**
 * Tests a pipeline represented by a PipeTree against expected values.
 *
 * This function checks if the provided PipeTree node matches the expected command,
 * arguments, input file, and output file. It is used to validate that a PipeTree
 * structure correctly represents a given command pipeline.
 *
 * Parameter
 *    tree The PipeTree structure representing the command pipeline to be tested.
 *    expected_command A string representing the expected command.
 *    expected_args A CList of expected arguments. Each element in the list
 *        should correspond to an argument of the command.
 *    expected_input_file A string representing the expected input file for
 *        redirection, or NULL if no input redirection is expected.
 *    expected_output_file A string representing the expected output file for
 *        redirection, or NULL if no output redirection is expected.
 *
 * Returns true if the PipeTree matches the expected values for the command, its
 *         arguments, and any input/output redirection; false otherwise.
 *
 * NB: The function assumes that the PipeTree, expected_command, and the elements
 *       within expected_args are properly initialized. The comparison is done only
 *       for the first (topmost) command in the PipeTree. If the PipeTree represents
 *       a more complex structure (like a series of piped commands), only the first
 *       command is checked.
 */
bool test_pipeline(
    PipeTree tree,
    const char *expected_command,
    const char ** expected_args,
    int args_sz,
    const char *expected_input_file,
    const char *expected_output_file);

#endif /* _PIPE_TREE_H_ */
