/*
 * expr_tree.h
 *
 * A dynamically allocated tree to handle arbitrary expressions
 *
 * Author: Nwankwo Chukwunonso Michael
 */

#ifndef _PIPE_TREE_H_
#define _PIPE_TREE_H_

#include <stdio.h>

typedef struct _pipe_tree_node *PipeTree;

// Program logic, relies on the order of the enum,
typedef enum
{
  WORD,
  CMD_LESS,
  CMD_GREAT,
  CMD_PIPE
} PipeNodeType;

/*
 * Create a value node on the tree. A value node is always a leaf.
 *
 * Parameters:
 *   value    The value for the leaf node
 *
 * Returns:
 *   The new tree, which will consist of a single leaf node
 *
 * It is the responsibility of the caller to call ET_free on a tree
 * that contains this leaf.
 */
PipeTree PT_word(const char* command, const char* args[]);


/*
 * Create an interior node on tree. An interior node always represents
 * an arithmetic operation.
 *
 * Parameters:
 *   op       The operator
 *   left     Left side of the operator
 *   right    Right side of the operator
 *
 * Returns: The new tree, which will consist of an interior node with
 *   two children.
 *
 * It is the responsibility of the caller to call ET_free on a tree
 * that contains this leaf
 */
PipeTree PT_node(PipeNodeType type, PipeTree left, PipeTree right);

/*
 * Destroy an ExprTree, calling free() on all malloc'd memory
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
 *     vars A dictionary containing the variables known so far, which
 *     may be modified by this function
 *     errmsg Return space for an error message, filled in in case of error
 *     errmsg_sz The size of errmsg
 *
 * Returns: The computed value on success. If a syntax error is
 * encountered, copies an error message into errmsg and returns NaN.
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

#endif /* _PIPE_TREE_H_ */
