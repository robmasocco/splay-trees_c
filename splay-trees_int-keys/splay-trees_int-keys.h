/* Roberto Masocco
 * Creation Date: 26/7/2019
 * Latest Version: 26/7/2019
 * ----------------------------------------------------------------------------
 * This file contains type definitions and declarations for the AVL Tree data
 * structure. See the source file for brief descriptions of what each function
 * does. Note that functions which names start with "_" are meant for internal
 * use only, and only those without it should be used by the actual programmer.
 * Many functions require dynamic memory allocation in the heap, and many
 * exposed methods require pointers or return some which refer to the heap: see
 * the source file to understand what needs to be freed after use.
 */
/* This code is released under the MIT license.
 * See the attached LICENSE file.
 */

#ifndef _AVLTREES_INTEGERKEYS_H
#define _AVLTREES_INTEGERKEYS_H

/* These options can be OR'd in a call to the delete functions to specify
 * if also the keys and/or the data in the nodes must be freed in the heap.
 * If nothing is specified, only the nodes are freed.
 */
#define DELETE_FREE_DATA 0x1

/* These options can be specified to tell the search functions what data to
 * return from the trees.
 * Only one at a time is allowed.
 */
#define SEARCH_DATA 0x4
#define SEARCH_KEYS 0x8
#define SEARCH_NODES 0x10

/* These options can be used to specify the desired kind of depth-first search.
 * Only one at a time is allowed.
 */
#define DFS_PRE_ORDER 0x20
#define DFS_IN_ORDER 0x40
#define DFS_POST_ORDER 0x80

/* These options can be used to specify the desired kind of breadth-first
 * search. Only one at a time is allowed.
 */
#define BFS_LEFT_FIRST 0x100
#define BFS_RIGHT_FIRST 0x200

/* An AVL Tree's node stores pointers to its "father" node and to its sons.
 * To calculate the balance factor, the height of the node is also stored.
 * In this implementation, integers are used as keys in the dictionary.
 * The data kept inside the node can be everything, as long as it's at most
 * 64-bits wide. These can be pointers, too.
 * Note that, as per the deletion options, is not possible to have only SOME
 * data in the heap: either all or none, so think about the data you're
 * providing to these functions.
 */
typedef struct _avlIntNode {
    struct _avlIntNode *_father;
    struct _avlIntNode *_leftSon;
    struct _avlIntNode *_rightSon;
    int _height;
    int _key;
    void *_data;
} AVLIntNode;

/* An AVL Tree stores a pointer to its root node and a counter which keeps
 * track of the number of nodes in the structure, to get an idea of its "size"
 * and be able to efficiently perform searches.
 * AVL trees implemented like this have a size limit set by the maximum
 * amount representable with an unsigned long integer, automatically set (as
 * long as you compile this code on the same machine you're going to use it on).
 */
typedef struct {
    AVLIntNode *_root;
    unsigned long int nodesCount;
    unsigned long int maxNodes;
} AVLIntTree;

/* Library functions. */
AVLIntTree *createIntTree(void);
int deleteIntTree(AVLIntTree *tree, int opts);
void *intSearch(AVLIntTree *tree, int key, int opts);
unsigned long int intInsert(AVLIntTree *tree, int newKey, void *newData);
int intDelete(AVLIntTree *tree, int key, int opts);
void **intDFS(AVLIntTree *tree, int type, int opts);
void **intBFS(AVLIntTree *tree, int type, int opts);

#endif
