/* Roberto Masocco
 * Creation Date: 26/7/2019
 * Latest Version: 26/7/2019
 * ----------------------------------------------------------------------------
 * This is the main source file for the AVL Trees library.
 * See the comments above each function definition for information about what
 * each one does. See the library header file for a brief description of the
 * "AVL Tree" data type.
 */
/* This code is released under the MIT license.
 * See the attached LICENSE file.
 */

#include <stdlib.h>
#include <limits.h>
#include "AVLTree_IntegerKeys.h"

/* Macro to find the maximum between two integers. */
#define MAX(X, Y) ((X) <= (Y) ? (Y) : (X))

/* Internal library subroutines declarations. */
AVLIntNode *_createIntNode(int newKey, void *newData);
void _deleteIntNode(AVLIntNode *node);
AVLIntNode *_searchIntNode(AVLIntTree *tree, int key);
void _intInsertAsLeftSubtree(AVLIntNode *father, AVLIntNode *newSon);
void _intInsertAsRightSubtree(AVLIntNode *father, AVLIntNode *newSon);
AVLIntNode *_intCutLeftSubtree(AVLIntNode *father);
AVLIntNode *_intCutRightSubtree(AVLIntNode *father);
AVLIntNode *_intCutSubtree(AVLIntNode *node);
AVLIntNode *_intMaxKeySon(AVLIntNode *node);
AVLIntNode *_intCutOneSonNode(AVLIntNode *node);
int _intHeight(AVLIntNode *node);
void _intSetHeight(AVLIntNode *node, int newHeight);
void _intSwapInfo(AVLIntNode *node1, AVLIntNode *node2);
int _intBalanceFactor(AVLIntNode *node);
void _intUpdateHeight(AVLIntNode *node);
void _intRightRotation(AVLIntNode *node);
void _intLeftRotation(AVLIntNode *node);
void _intRotate(AVLIntNode *node);
void _intBalanceInsert(AVLIntNode *newNode);
void _intBalanceDelete(AVLIntNode *remFather);
void _intInODFS(AVLIntNode *rootNode, void ***intPtr, int intOpt);
void _intPreODFS(AVLIntNode *rootNode, void ***intPtr, int intOpt);
void _intPostODFS(AVLIntNode *rootNode, void ***intPtr, int intOpt);

// USER FUNCTIONS //
/* Creates a new AVL Tree in the heap. */
AVLIntTree *createIntTree(void) {
    AVLIntTree *newTree = (AVLIntTree *) malloc(sizeof(AVLIntTree));
    if (newTree == NULL) return NULL;
    newTree->_root = NULL;
    newTree->nodesCount = 0;
    newTree->maxNodes = ULONG_MAX;
    return newTree;
}

/* Frees a given AVL Tree from the heap. Using options defined in the header,
 * it's possible to specify whether also data has to be freed or not.
 */
int deleteIntTree(AVLIntTree *tree, int opts) {
    // Sanity check on input arguments.
    if (tree == NULL) return -1;
    if (opts < 0) return -1;
    // If the tree is empty free it directly.
    if (tree->_root == NULL) {
        free(tree);
        return 0;
    }
    // Do a BFS to get all the nodes (less taxing on memory than a DFS).
    AVLIntNode **nodes = (AVLIntNode **) intBFS(tree, BFS_LEFT_FIRST,
                                                SEARCH_NODES);
    // Free the nodes and eventually their data.
    for (unsigned long int i = 0; i < tree->nodesCount; i++) {
        if (opts & DELETE_FREE_DATA) free((*(nodes[i]))._data);
        _deleteIntNode(nodes[i]);
    }
    // Free the nodes array and the tree, and that's it!
    free(nodes);
    free(tree);
    return 0;
}

/* Searches for an entry with the specified key in the tree. */
void *intSearch(AVLIntTree *tree, int key, int opts) {
    if ((opts <= 0) || (tree == NULL)) return NULL;  // Sanity check.
    AVLIntNode *searchedNode = _searchIntNode(tree, key);
    if (searchedNode != NULL) {
        if (opts & SEARCH_DATA) return searchedNode->_data;
        if (opts & SEARCH_NODES) return searchedNode;
        return NULL;
    }
    return NULL;
}

/* Deletes an entry from the tree. */
int intDelete(AVLIntTree *tree, int key, int opts) {
    // Sanity check on input arguments.
    if ((opts < 0) || (tree == NULL)) return 0;
    AVLIntNode *toDelete = _searchIntNode(tree, key);
    AVLIntNode *toFree;
    if (toDelete != NULL) {
        // Check whether the node has no sons or even one.
        if ((toDelete->_leftSon == NULL) || (toDelete->_rightSon == NULL)) {
            toFree = _intCutOneSonNode(toDelete);
        } else {
            // Find the node's predecessor and swap the content.
            AVLIntNode *maxLeft = _intMaxKeySon(toDelete->_leftSon);
            _intSwapInfo(toDelete, maxLeft);
            // Remove the original predecessor.
            toFree = _intCutOneSonNode(maxLeft);
        }
        // Apply eventual options to free keys and data, then free the node.
        if (opts & DELETE_FREE_DATA) free(toFree->_data);
        free(toFree);
        tree->nodesCount--;
        // Check if the tree is now empty and update root pointer.
        if (tree->nodesCount == 0) tree->_root = NULL;
        return 1;  // Found and deleted.
    }
    return 0;  // Not found.
}

/* Creates and inserts a new node in the tree. */
unsigned long int intInsert(AVLIntTree *tree, int newKey, void *newData) {
    if (tree == NULL) return 0;  // Sanity check.
    if (tree->nodesCount == tree->maxNodes) return 0;  // The tree is full.
    AVLIntNode *newNode = _createIntNode(newKey, newData);
    if (tree->_root == NULL) {
        // The tree is empty.
        tree->_root = newNode;
        tree->nodesCount++;
    } else {
        // Look for the correct position and place it there.
        AVLIntNode *curr = tree->_root;
        AVLIntNode *pred = NULL;
        int comp;
        while (curr != NULL) {
            pred = curr;
            comp = curr->_key - newKey;
            if (comp >= 0) {
                // Equals are kept in the left subtree.
                curr = curr->_leftSon;
            } else {
                curr = curr->_rightSon;
            }
        }
        comp = pred->_key - newKey;
        if (comp >= 0) {
            _intInsertAsLeftSubtree(pred, newNode);
        } else {
            _intInsertAsRightSubtree(pred, newNode);
        }
        _intBalanceInsert(newNode);
        tree->nodesCount++;
    }
    return tree->nodesCount;  // Return the result of the insertion.
}

/* Performs a depth-first search of the tree, the type of which can be
 * specified using the options defined in the header.
 * Depending on the option specified, returns an array of:
 * - Pointers to the nodes.
 * - Keys.
 * - Data.
 * See the header for the definitions of such options.
 * Remember to free the returned array afterwards!
 */
void **intDFS(AVLIntTree *tree, int type, int opts) {
    // Sanity check for the input arguments.
    if ((type <= 0) || (opts <= 0)) return NULL;
    if ((tree == NULL) || (tree->_root == NULL)) return NULL;
    // Allocate memory according to options.
    void **dfsRes;
    int intOpt;
    if (opts & SEARCH_DATA) {
        intOpt = SEARCH_DATA;
        dfsRes = calloc(tree->nodesCount, sizeof(void *));
    } else if (opts & SEARCH_KEYS) {
        intOpt = SEARCH_KEYS;
        dfsRes = calloc(tree->nodesCount, sizeof(int));
    } else if (opts & SEARCH_NODES) {
        intOpt = SEARCH_NODES;
        dfsRes = calloc(tree->nodesCount, sizeof(AVLIntNode *));
    } else return NULL;  // Invalid option.
    if (dfsRes == NULL) return NULL;  // calloc failed.
    // Launch the requested DFS according to type.
    void **intPtr = dfsRes;
    if (type & DFS_PRE_ORDER) {
        _intPreODFS(tree->_root, &intPtr, intOpt);
    } else if (type & DFS_IN_ORDER) {
        _intInODFS(tree->_root, &intPtr, intOpt);
    } else if (type & DFS_POST_ORDER) {
        _intPostODFS(tree->_root, &intPtr, intOpt);
    } else {
        // Invalid type.
        free(dfsRes);
        return NULL;
    }
    // The array is now filled with the requested data.
    return dfsRes;
}

/* Performs a breadth-first search of the tree, the type of which can be
 * specified using the options defined in the header (left or right son
 * visited first).
 * Depending on the option specified, returns an array of:
 * - Pointers to the nodes.
 * - Keys.
 * - Data.
 * See the header for the definitions of such options.
 * Remember to free the returned array afterwards!
 */
void **intBFS(AVLIntTree *tree, int type, int opts) {
    // Sanity check on input arguments.
    if ((tree == NULL) || (tree->_root == NULL) ||
        (type <= 0) || (opts <= 0) ||
        !((type & BFS_LEFT_FIRST) || (type & BFS_RIGHT_FIRST)) ||
        !((opts & SEARCH_KEYS) || (opts & SEARCH_DATA) ||
        (opts & SEARCH_NODES))) return NULL;
    // Allocate memory in the heap.
    void **bfsRes = NULL;
    void **intPtr;
    int *keyPtr;  // Used only if keys are searched.
    if (opts & SEARCH_DATA) {
        bfsRes = calloc(tree->nodesCount, sizeof(void *));
    } else if (opts & SEARCH_KEYS) {
        bfsRes = calloc(tree->nodesCount, sizeof(AVLIntNode *));
        keyPtr = (int *) bfsRes;
    } else if (opts & SEARCH_NODES) {
        bfsRes = calloc(tree->nodesCount, sizeof(AVLIntNode *));
    } else return NULL;  // Invalid option.
    if (bfsRes == NULL) return NULL;  // Calloc failed.
    intPtr = bfsRes + 1;
    *bfsRes = (void *) (tree->_root);
    AVLIntNode *curr;
    // Start the visit, using the same array to return as a temporary queue
    // for the nodes.
    for (unsigned long int i = 0; i < tree->nodesCount; i++) {
        curr = (AVLIntNode *) bfsRes[i];
        // Visit the current node.
        if (opts & SEARCH_DATA) {
            bfsRes[i] = curr->_data;
        } else if (opts & SEARCH_KEYS) {
            *keyPtr = curr->_key;
            keyPtr++;
        } else if (opts & SEARCH_NODES) {
            bfsRes[i] = curr;
        }
        // Eventually add the sons to the array, to be visited afterwards.
        if (type & BFS_LEFT_FIRST) {
            if (curr->_leftSon != NULL) {
                *intPtr = (void *) (curr->_leftSon);
                intPtr++;
            }
            if (curr->_rightSon != NULL) {
                *intPtr = (void *) (curr->_rightSon);
                intPtr++;
            }
        } else if (type & BFS_RIGHT_FIRST) {
            if (curr->_rightSon != NULL) {
                *intPtr = (void *) (curr->_rightSon);
                intPtr++;
            }
            if (curr->_leftSon != NULL) {
                *intPtr = (void *) (curr->_leftSon);
                intPtr++;
            }
        }
    }
    if (opts & SEARCH_KEYS) {
        // If keys were searched, part of the array (half of it on x86-64)
        // is totally unneeded, so we can release it.
        // reallocarray is used instead of realloc to account for possible size
        // computation overflows (see man).
        if ((bfsRes = reallocarray(bfsRes, (size_t) (tree->nodesCount),
                sizeof(int))) == NULL) {
            free(bfsRes);
            return NULL;
        }
    }
    return bfsRes;
}

// INTERNAL LIBRARY SUBROUTINES //
/* Creates a new AVL node in the heap. Requires an integer key and some data. */
AVLIntNode *_createIntNode(int newKey, void *newData) {
    AVLIntNode *newNode = (AVLIntNode *) malloc(sizeof(AVLIntNode));
    if (newNode == NULL) return NULL;
    newNode->_father = NULL;
    newNode->_leftSon = NULL;
    newNode->_rightSon = NULL;
    newNode->_key = newKey;
    newNode->_data = newData;
    newNode->_height = 0;
    return newNode;
}

/* Frees memory occupied by a node. */
void _deleteIntNode(AVLIntNode *node) {
    free(node);
}

/* Inserts a subtree rooted in a given node as the left subtree of a given
 * node.
 */
void _intInsertAsLeftSubtree(AVLIntNode *father, AVLIntNode *newSon) {
    if (newSon != NULL) newSon->_father = father;
    father->_leftSon = newSon;
}

/* Inserts a subtree rooted in a given node as the right subtree of a given
 * node.
 */
void _intInsertAsRightSubtree(AVLIntNode *father, AVLIntNode *newSon) {
    if (newSon != NULL) newSon->_father = father;
    father->_rightSon = newSon;
}

/* Cuts and returns the left subtree of a given node. */
AVLIntNode *_intCutLeftSubtree(AVLIntNode *father) {
    AVLIntNode *son = father->_leftSon;
    if (son == NULL) return NULL;  // Sanity check.
    son->_father = NULL;
    father->_leftSon = NULL;
    return son;
}

/* Cuts and returns the right subtree of a given node. */
AVLIntNode *_intCutRightSubtree(AVLIntNode *father) {
    AVLIntNode *son = father->_rightSon;
    if (son == NULL) return NULL;  // Sanity check.
    son->_father = NULL;
    father->_rightSon = NULL;
    return son;
}

/* Cuts and returns the subtree nested in a given node. */
AVLIntNode *_intCutSubtree(AVLIntNode *node) {
    if (node == NULL) return NULL;  // Sanity check.
    if (node->_father == NULL) return node;  // Asked to cut at root.
    AVLIntNode *father = node->_father;
    if ((node->_leftSon == NULL) && (node->_rightSon == NULL)) {
        // The node is a leaf: distinguish between the node being a left or
        // right son and cut accordingly.
        if (father->_rightSon == node) {
            father->_rightSon = NULL;
        } else father->_leftSon = NULL;
        node->_father = NULL;
        return node;
    } else if (father->_leftSon == node) return _intCutLeftSubtree(father);
    return _intCutRightSubtree(father);
}

/* Returns the descendant of a given node with the greatest key. */
AVLIntNode *_intMaxKeySon(AVLIntNode *node) {
    AVLIntNode *curr = node;
    while (curr->_rightSon != NULL) curr = curr->_rightSon;
    return curr;
}

/* Returns a pointer to the node with the specified key, or NULL. */
AVLIntNode *_searchIntNode(AVLIntTree *tree, int key) {
    if (tree->_root == NULL) return NULL;
    AVLIntNode *curr = tree->_root;
    int comp;
    while (curr != NULL) {
        comp = curr->_key - key;
        if (comp > 0) {
            curr = curr->_leftSon;
        } else if (comp < 0) {
            curr = curr->_rightSon;
        } else return curr;
    }
    return NULL;
}

/* Returns the height of a given node. */
int _intHeight(AVLIntNode *node) {
    if (node == NULL) {
        return -1;  // Useful when computing balance factors.
    }
    return node->_height;
}

/* Sets the height of the specified node to the given value. */
void _intSetHeight(AVLIntNode *node, int newHeight) {
    if (node != NULL) node->_height = newHeight;
}

/* Returns the balance factor of a given node. */
int _intBalanceFactor(AVLIntNode *node) {
    if (node == NULL) return 0;  // Consistency check.
    return _intHeight(node->_leftSon) - _intHeight(node->_rightSon);
}

/* Updates the height of a given node. */
void _intUpdateHeight(AVLIntNode *node) {
    if (node != NULL) {
        _intSetHeight(node, MAX(_intHeight(node->_leftSon),
                                _intHeight(node->_rightSon)) + 1);
    }
}

/* Swaps contents between two nodes. */
void _intSwapInfo(AVLIntNode *node1, AVLIntNode *node2) {
    int key1 = node1->_key;
    void *data1 = node1->_data;
    int key2 = node2->_key;
    void *data2 = node2->_data;
    node1->_key = key2;
    node2->_key = key1;
    node1->_data = data2;
    node2->_data = data1;
}

/* Performs a simple right rotation at the specified node. */
void _intRightRotation(AVLIntNode *node) {
    AVLIntNode *leftSon = node->_leftSon;
    // Swap the node and its son's contents to make it climb.
    _intSwapInfo(node, leftSon);
    // Shrink the tree portion in subtrees.
    AVLIntNode *rTree = _intCutRightSubtree(node);
    AVLIntNode *lTree = _intCutLeftSubtree(node);
    AVLIntNode *lTree_l = _intCutLeftSubtree(leftSon);
    AVLIntNode *lTree_r = _intCutRightSubtree(leftSon);
    // Recombine portions to respect the search property.
    _intInsertAsRightSubtree(lTree, rTree);
    _intInsertAsLeftSubtree(lTree, lTree_r);
    _intInsertAsRightSubtree(node, lTree);
    _intInsertAsLeftSubtree(node, lTree_l);
    // Update the height of the involved nodes.
    _intUpdateHeight(node->_rightSon);
    _intUpdateHeight(node);
}

/* Performs a simple left rotation at the specified node. */
void _intLeftRotation(AVLIntNode *node) {
    AVLIntNode *rightSon = node->_rightSon;
    // Swap the node and its son's contents to make it climb.
    _intSwapInfo(node, rightSon);
    // Shrink the tree portion in subtrees.
    AVLIntNode *rTree = _intCutRightSubtree(node);
    AVLIntNode *lTree = _intCutLeftSubtree(node);
    AVLIntNode *rTree_l = _intCutLeftSubtree(rightSon);
    AVLIntNode *rTree_r = _intCutRightSubtree(rightSon);
    // Recombine portions to respect the search property.
    _intInsertAsLeftSubtree(rTree, lTree);
    _intInsertAsRightSubtree(rTree, rTree_l);
    _intInsertAsLeftSubtree(node, rTree);
    _intInsertAsRightSubtree(node, rTree_r);
    // Update the height of the involved nodes.
    _intUpdateHeight(node->_leftSon);
    _intUpdateHeight(node);
}

/* Examines the balance factor of a given node and eventually rotates. */
void _intRotate(AVLIntNode *node) {
    int balFactor = _intBalanceFactor(node);
    if (balFactor == 2) {
        if (_intBalanceFactor(node->_leftSon) >= 0) {
            // LL displacement: rotate right.
            _intRightRotation(node);
        } else {
            // LR displacement: apply double rotation.
            _intLeftRotation(node->_leftSon);
            _intRightRotation(node);
        }
    } else if (balFactor == -2) {
        if (_intBalanceFactor(node->_rightSon) <= 0) {
            // RR displacement: rotate left.
            _intLeftRotation(node);
        } else {
            // RL displacement: apply double rotation.
            _intRightRotation(node->_rightSon);
            _intLeftRotation(node);
        }
    }
}

/* Updates heights and looks for displacements following an insertion. */
void _intBalanceInsert(AVLIntNode *newNode) {
    AVLIntNode *curr = newNode->_father;
    while (curr != NULL) {
        if (abs(_intBalanceFactor(curr)) >= 2) {
            // Unbalanced node found.
            break;
        } else {
            _intUpdateHeight(curr);
            curr = curr->_father;
        }
    }
    if (curr != NULL) _intRotate(curr);
}

/* Updates heights and looks for displacements following a deletion. */
void _intBalanceDelete(AVLIntNode *remFather) {
    AVLIntNode *curr = remFather;
    while (curr != NULL) {
        if (abs(_intBalanceFactor(curr)) >= 2) {
            // There may be more than one unbalanced node.
            _intRotate(curr);
        } else _intUpdateHeight(curr);
        curr = curr->_father;
    }
}

/* Cuts a node with a single son. */
AVLIntNode *_intCutOneSonNode(AVLIntNode *node) {
    AVLIntNode *son = NULL;
    AVLIntNode *father = node->_father;
    if (node->_leftSon != NULL) {
        son = node->_leftSon;
    } else if (node->_rightSon != NULL) {
        son = node->_rightSon;
    }
    if (son == NULL) {  // The node is a leaf.
        son = _intCutSubtree(node);  // Will be returned later.
    } else {
        // Swap the content from the son to the father.
        _intSwapInfo(node, son);
        // Cut the node and balance the deletion.
        _intCutSubtree(son);
        _intInsertAsRightSubtree(node, _intCutSubtree(son->_rightSon));
        _intInsertAsLeftSubtree(node, _intCutSubtree(son->_leftSon));
    }
    _intBalanceDelete(father);
    return son;  // Return the node to free, now totally disconnected.
}

/* Performs an in-order, recursive DFS. */
void _intInODFS(AVLIntNode *rootNode, void ***intPtr, int intOpt) {
    // Recursion base step.
    if (rootNode == NULL) {
        // Correctly update the pointer.
        if (intOpt & SEARCH_KEYS) {
            *(intPtr) = (void **)((int *)*(intPtr) - 1);
        } else *(intPtr) = *(intPtr) - 1;
        return;
    }
    // Recursive step: visit the left son.
    _intInODFS(rootNode->_leftSon, intPtr, intOpt);
    // Correctly increment the internal pointer.
    if (intOpt & SEARCH_KEYS) {
        *(intPtr) = (void **)(int *)*(intPtr) + 1);
    } else *(intPtr) = *(intPtr) + 1;
    // Now visit the root node.
    if (intOpt & SEARCH_NODES) {
        **(intPtr) = rootNode;
    } else if (intOpt & SEARCH_KEYS) {
        *(int *)*(intPtr) = rootNode->_key;
    } else if (intOpt & SEARCH_DATA) {
        **(intPtr) = rootNode->_data;
    }
    // Correctly increment the internal pointer.
    if (intOpt & SEARCH_KEYS) {
        *(intPtr) = (void **)((int *)*(intPtr) + 1);
    } else *(intPtr) = *(intPtr) + 1;
    // Visit the right son and return.
    _intInODFS(rootNode->_rightSon, intPtr, intOpt);
}

/* Performs a pre-order, recursive DFS. */
void _intPreODFS(AVLIntNode *rootNode, void ***intPtr, int intOpt) {
    // Recursion base step.
    if (rootNode == NULL) {
        // Correctly update the pointer.
        if (intOpt & SEARCH_KEYS) {
            *(intPtr) = (void **)((int *)*(intPtr) - 1);
        } else *(intPtr) = *(intPtr) - 1;
        return;
    }
    // Recursive step.
    // Visit the root node.
    if (intOpt & SEARCH_NODES) {
        **(intPtr) = rootNode;
    } else if (intOpt & SEARCH_KEYS) {
        *(int *)*(intPtr) = rootNode->_key;
    } else if (intOpt & SEARCH_DATA) {
        **(intPtr) = rootNode->_data;
    }
    // Correctly increment the internal pointer.
    if (intOpt & SEARCH_KEYS) {
        *(intPtr) = (void **)((int *)*(intPtr) + 1);
    } else *(intPtr) = *(intPtr) + 1;
    // Now visit the left son.
    _intPreODFS(rootNode->_leftSon, intPtr, intOpt);
    // Correctly increment the internal pointer.
    if (intOpt & SEARCH_KEYS) {
        *(intPtr) = (void **)((int *)*(intPtr) + 1);
    } else *(intPtr) = *(intPtr) + 1;
    // Visit the right son and return.
    _intPreODFS(rootNode->_rightSon, intPtr, intOpt);
}

/* Performs a post-order, recursive DFS. */
void _intPostODFS(AVLIntNode *rootNode, void ***intPtr, int intOpt) {
    // Recursion base step.
    if (rootNode == NULL) {
        // Correctly update the pointer.
        if (intOpt & SEARCH_KEYS) {
            *(intPtr) = (void **)((int *)*(intPtr) - 1);
        } else *(intPtr) = *(intPtr) - 1;
        return;
    }
    // Recursive step.
    // Visit the left son.
    _intPostODFS(rootNode->_leftSon, intPtr, intOpt);
    // Correctly increment the internal pointer.
    if (intOpt & SEARCH_KEYS) {
        *(intPtr) = (void **)((int *)*(intPtr) + 1);
    } else *(intPtr) = *(intPtr) + 1;
    // Visit the right son.
    _intPostODFS(rootNode->_rightSon, intPtr, intOpt);
    // Correctly increment the internal pointer.
    if (intOpt & SEARCH_KEYS) {
        *(intPtr) = (void **)((int *)*(intPtr) + 1);
    } else *(intPtr) = *(intPtr) + 1;
    // Visit the root node and return.
    if (intOpt & SEARCH_NODES) {
        **(intPtr) = rootNode;
    } else if (intOpt & SEARCH_KEYS) {
        *(int *)*(intPtr) = rootNode->_key;
    } else if (intOpt & SEARCH_DATA) {
        **(intPtr) = rootNode->_data;
    }
}
