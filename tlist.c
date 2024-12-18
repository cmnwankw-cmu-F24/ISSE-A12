/*
 *
 * tlist.c
 * 
 * Linked list implementation
 * 
 * Author: Nwankwo Chukwunonso Michael
 * 
 */

#include <stdio.h>  //included the stdio header file
#include <stdlib.h> //included the stdlib header file
#include <assert.h> //include the assert header file
#include <string.h> //included the string header file
 
#include "tlist.h" //included the user-defined header file clist.h 

#define DEBUG //defined DEBUG for conditional execution


/*
 *
 * Defined a struct represent the structure of a node in 
 * the LinkedList
 * 
 * Each node has:
 *  1- a CListElementType, which the element
 *  2- a _cl_node pointer which is the pointer the next node 
 * 
 */
struct _tl_node {
  TListElementType element;
  struct _tl_node *next;
};

/*
 *
 * Defined a struct represent the metadata of the linkedlist
 * 
 * The metadata for the linkedlist has:
 * 1- a _cl_node pointer to the head of the linkedlist
 * 2- an int variable containing the length of the list
 * 
 */

struct _tlist {
  struct _tl_node *head;
  int length;
};



/*
 * Create (malloc) a new _cl_node and populate it with the supplied
 * values
 *
 * Parameters:
 *   element, next  the values for the node to be created
 * 
 * Returns: The newly-malloc'd node, or NULL in case of error
 */
static struct _tl_node* _TL_new_node(TListElementType element, struct _tl_node *next)
{
  //allocate memory with the right size to store the new node
  struct _tl_node* new = (struct _tl_node*) malloc(sizeof(struct _tl_node));

  assert(new); //assert that the memory was allocated

  new->element = element; //initialize the element for the node 
  new->next = next; //initialize the next pointer for the node

  return new; //return the newly created node
}



// Documented in .h file
TList TL_new()
{
  //allocate memory with the right size to store the metadata for the list
  TList list = (TList) malloc(sizeof(struct _tlist));
  assert(list); //assert that the memory was allocated

  list->head = NULL; //initialize the head for the metadata to NULL
  list->length = 0; //initialize the length for the metadata to 0

  return list; //return the newly created list
}



// Documented in .h file
void TL_free(TList list)
{
  //if list is NULL return
  if(list == NULL){
    return;
  }

  //Get a reference to the first node
  struct _tl_node *free_node = list -> head;

  //loop through till free_node is NULL
  while(free_node != NULL){
    list -> head = free_node -> next; //point the head to the next node

    free(free_node); //free the current node
    free_node = list -> head; //update the free_node with the value of the head pointer.
  }
  free(list); //free the list the contains the metadata for the list
  list = NULL; //set list pointer to NULL

  return; 

}



// Documented in .h file
int TL_length(TList list)
{
  // assert(list); //assert that the list is valid
  if(list == NULL){
    return 0;
  }
#ifdef DEBUG
  // In production code, we simply return the stored value for
  // length. However, as a defensive programming method to prevent
  // bugs in our code, in DEBUG mode we walk the list and ensure the
  // number of elements on the list is equal to the stored length.

  int len = 0; //declare an int variable and initialize it to 0
  
  //loop through each element in the linkedlist 
  for (struct _tl_node *node = list->head; node != NULL; node = node->next)
    len++; //increment len in each iteration

  //assert that the node counted equals the length in the metadata for the list
  assert(len == list->length);  
#endif // DEBUG

  return list->length; //return the value in length
}



// Documented in .h file
void TL_push(TList list, TListElementType element)
{
  // assert(list); //assert that the list is valid
    if(list == NULL){
    return;
  }

  //Create a new node that has the its next pointer as the current head
  //and update the head to point to the newly created node
  list->head = _TL_new_node(element, list->head); 
  list->length++; //increment the length by 1
}



// Documented in .h file
TListElementType TL_pop(TList list)
{
  // assert(list); //assert that the list is valid
  if(list == NULL){
    return INVALID;
  }
  struct _tl_node *popped_node = list->head; //get a reference to the head of the linkedlist

  //if the head is NULL return INVALID_RETURN
  if (popped_node == NULL)
    return INVALID;

  TListElementType ret = popped_node->element; //get a reference to the element in the head 

  // unlink previous head node, then free it
  list->head = popped_node->next;
  free(popped_node);
  // we cannot refer to popped node any longer

  popped_node = NULL; //set popped_node to NULL
  list->length--; //decrement the length of the list

  return ret; //return the string that is popped
}



// Documented in .h file
void TL_append(TList list, TListElementType element)
{
  // assert(list); //assert that the list is valid

  if(list == NULL){
    return;
  }
  struct _tl_node *tail_node= list->head; // get a reference to the head node

  
  if(tail_node == NULL){
  // if the head node is NULL, create the new node with a next pointer of NULL
    list->head = _TL_new_node(element, NULL); // update the list's head to the newly created node.
    list -> length++; //increment the length by 1
    return;
  }


  for ( ; tail_node->next != NULL; tail_node = tail_node->next)
    ;//loop till we hit the tail node

  //Set the next pointer of the tail node to the newly created node.
  tail_node -> next = _TL_new_node(element, NULL);

  list-> length++; //increment the length by 1
  return;

}




// Documented in .h file
TListElementType TL_nth(TList list, int pos)
{
  // assert(list); // assert that the list is valid
  if(list == NULL){
    return INVALID;
  }

  // Validate that pos is within the range of the list
  if(!(pos >= -(list->length) && pos <= (list->length-1))){
    return INVALID;
  }


  // if pos is negative, get the positive equivalent
  if(pos < 0){
    pos = list -> length + pos;
  }

  int num = 0; // declare and initialize an int variable to 0

  //loop till the node at the index 'pos' and return the element at the node
  for (struct _tl_node *node = list->head; node != NULL; node = node->next){
    if(num == pos){
        return node -> element;
    }
    num++;
  }

  // if the node is not found return, INVALID_RETURN 
  return INVALID;
}



// Documented in .h file
bool TL_insert(TList list, TListElementType element, int pos)
{
  // assert(list); // assert that the list is valid
  if(list == NULL){
    return false;
  }

  int num = 0; // declare and initialize an int variable to 0

  
  // Validate that pos is within the range of the list
  if(!(pos >= (-list->length -1) && pos <= list->length)){
    return false;
  }

  // if pos is negative, get the positive equivalent
  if(pos < 0){
    pos = list -> length + pos + 1;
  }

  //if pos is 0. it's a push
  if(pos == 0){
    TL_push(list, element);
    return true;
  }

  
  
   
  for (struct _tl_node *node = list->head; node != NULL; node = node->next){
    if(num == (pos-1)){
    //loop till the 'pos - 1'th node
    //create a new node with a next pointer of the 'pos'th node
    //point the pos - 1 th node next pointer to the newly created node
        node->next = _TL_new_node(element, node->next);
        list->length++; //increment the length of the list by 1
        return true;
    }
    num++;
  }

  return false;
}


    
// Documented in .h file
TListElementType TL_remove(TList list, int pos)
{
  // assert(list); // assert that the list is valid
  if(list == NULL){
    return INVALID;
  }

  //if not within the valid range for insertion, return INVALID_RETURN
  if(!(pos >= -(list->length) && pos <= (list->length-1))){
    return INVALID;
  }

  // if pos is negative, get the positive equivalent
  if(pos < 0){
    pos = list -> length + pos;
  }

  //if pos is 0, it's a pop
  if(pos == 0){
    return TL_pop(list); 
  }

  int num=0;  // declare and initialize an int variable to 0
  for(struct _tl_node *node = list-> head; node != NULL; node = node->next){
    if(num == (pos - 1)){
    //loop till the 'pos - 1'th node
      struct _tl_node *remove_node = node -> next; //get a reference to the node to be removed

      TListElementType ret = remove_node -> element; //get the string value of the node to be removed

      //Update the 'pos - 1'th node's next pointer to the 'pos + 1'th node
      node -> next = remove_node -> next; 

      free(remove_node); //destroy the memory of the node to be destroyed
      remove_node = NULL; //set the pointer to NULL

      list -> length--; //decrement the length of the list

      return ret; //return the string value of the removed node

    }
    num++;
  }
 
  return INVALID;
}



// Documented in .h file
TList TL_copy(TList list)
{
  // assert(list); // assert that the list is valid
  if(list == NULL){
    return NULL;
  }

  TList copy_list = TL_new(); // create a new list

  if(list->head == NULL){
  //if the list is empty, return the newly created list
    return copy_list;
  }

  //create a copy of the head node
  copy_list -> head = _TL_new_node(list->head->element, NULL);
  struct _tl_node  *copy_node = copy_list->head; //get a reference to the copy of the head node
  
  if(copy_node != NULL){
    copy_list -> length = 1; //update the length to 1, if the copy_node was created successfully.
  }


  //set 'node' pointer one position ahead of the 'copy_node' pointer
  //loop through till 'node' pointer is NULL
  //on each iteration, make a copy of node and update the copy list
  for(struct _tl_node *node=list->head->next ; node != NULL; node=node->next){

    copy_node -> next = _TL_new_node(node->element, NULL);
    copy_node = copy_node -> next;
    copy_list -> length++;

  }

  copy_node = NULL; //set the copy_pointer to NULL

  return copy_list; //return the copied the list
}


// Documented in .h file
void TL_join(TList list1, TList list2){
  
  // assert(list1); // assert that the list1 is valid
  // assert(list2); // assert that the list2 is valid

  if(list1 == NULL || list2 == NULL){
    return;
  }

  if(list2->head == NULL){
  //if the list2 is empty, return
    return;
  }else if(list1->head == NULL){
  //if list1 is empty, but list2 is not empty
    list1->head = list2->head; //attached the head of list2 to the head of list1
    list1->length = list2->length; //update the length
    list2->head = NULL; //set the head of list2 to NULL
    list2->length = 0; //set the length of list2 to 0
    return; //return
  }else{
  //if list1 is not empty, and list2 is not empty

    //get the tail node
    struct _tl_node *tail_node = list1->head;
    
    while(tail_node->next != NULL){
      tail_node = tail_node->next;
    }

    tail_node->next = list2->head; //attached the head of list2 to tail of list1
    list1->length = list1->length+ list2->length;  //update the length
    list2->head = NULL;  //set the head of list2 to NULL
    list2->length = 0; //set the length of list2 to 0

    return; //return
  }



}


// Documented in .h file
void TL_reverse(TList list)
{
  // assert(list); // assert that the list is valid
  if(list == NULL){
    return;
  }

  struct _tl_node *prev = NULL; //initialize a _cl_node pointer to NULL
  struct _tl_node *curr = list->head; //initialize a _cl_node pointer to head node of the list
  struct _tl_node *next = NULL;  //initialize a _cl_node pointer to NULL


  //loop through till curr is NULL
  while(curr != NULL){
    next = curr -> next; //get a reference to the next
    curr-> next = prev; //update the curr's next pointer to prev

    prev = curr; //update prev with curr
    curr = next; //update curr with next
  }

  list -> head = prev; //set the head of the list to prev

}


// Documented in .h file
void TL_foreach(TList list, TL_foreach_callback callback, void *cb_data)
{
  // assert(list); // assert that the list is valid
  if(list == NULL){
    return;
  }

    int pos = 0; // declare and initialize an int variable to 0

    //loop through each node in the list
    for(struct _tl_node *node = list -> head; node != NULL; node = node -> next){
        callback(pos, node->element, cb_data); //on each iteration, invoke the callback function
        pos++; //increment pos by 1
    }
}
