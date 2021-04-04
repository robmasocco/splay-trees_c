/*
 * @brief Splay Tree data structure library source code.
 *
 * @author Roberto Masocco
 *
 * @date April 4, 2021
 */
/* 
 * This code is released under the MIT license.
 * See the attached LICENSE file.
 */

#include <stdlib.h>
#include <limits.h>
#include "splay-trees_int-keys.h"

/* Macro to find the maximum between two integers. */
#define MAX(X, Y) ((X) <= (Y) ? (Y) : (X))

/* Internal library subroutines declarations. */
SplayIntNode *_create_splay_int_node(int new_key, void *new_data);
void _delete_splay_int_node(SplayIntNode *node);
SplayIntNode *_search_splay_int_node(SplayIntTree *tree, int key);
void _spli_insert_left_subtree(SplayIntNode *father, SplayIntNode *new_son);
void _spli_insert_right_subtree(SplayIntNode *father, SplayIntNode *new_son);
SplayIntNode *_spli_cut_left_subtree(SplayIntNode *father);
SplayIntNode *_spli_cut_right_subtree(SplayIntNode *father);
SplayIntNode *_spli_cut_subtree(SplayIntNode *node);
SplayIntNode *_spli_max_key_son(SplayIntNode *node);
SplayIntNode *_spli_cut_one_son_node(SplayIntNode *node);
void _spli_swap_info(SplayIntNode *node1, SplayIntNode *node2);
void _spli_right_rotation(SplayIntNode *node);
void _spli_left_rotation(SplayIntNode *node);
void _spli_rotate(SplayIntNode *node);  // TODO Deprecated?
void _spli_splay_insert(SplayIntNode *new_node);  // TODO See below.
void _spli_splay_delete(SplayIntNode *rem_father);  // TODO See below.
void _spli_inodfs(SplayIntNode *root_node, void ***int_ptr, int int_opt);
void _spli_preodfs(SplayIntNode *root_node, void ***int_ptr, int int_opt);
void _spli_postodfs(SplayIntNode *root_node, void ***int_ptr, int int_opt);

// TODO Add splay heuristic for search?

// USER FUNCTIONS //
/*
 * Creates a new AVL Tree in the heap.
 *
 * @return Pointer to the newly created tree.
 */
SplayIntTree *create_splay_int_tree(void) {
    SplayIntTree *newTree = (SplayIntTree *) malloc(sizeof(SplayIntTree));
    if (newTree == NULL) return NULL;
    newTree->_root = NULL;
    newTree->nodes_count = 0;
    newTree->max_nodes = ULONG_MAX;
    return newTree;
}

/* 
 * Frees a given AVL Tree from the heap. Using options defined in the header,
 * it's possible to specify whether also data has to be freed or not.
 *
 * @param tree Pointer to the tree to free.
 * @param opts Options to configure the deletion behaviour (see header).
 * @return 0 if all went well, or -1.
 */
int delete_splay_int_tree(SplayIntTree *tree, int opts) {
    // Sanity check on input arguments.
    if (tree == NULL) return -1;
    if (opts < 0) return -1;
    // If the tree is empty free it directly.
    if (tree->_root == NULL) {
        free(tree);
        return 0;
    }
    // Do a BFS to get all the nodes (less taxing on memory than a DFS).
    SplayIntNode **nodes =
        (SplayIntNode **)splay_int_bfs(tree, BFS_LEFT_FIRST, SEARCH_NODES);
    // Free the nodes and eventually their data.
    for (unsigned long int i = 0; i < tree->nodes_count; i++) {
        if (opts & DELETE_FREE_DATA) free((*(nodes[i]))._data);
        _delete_splay_int_node(nodes[i]);
    }
    // Free the nodes array and the tree, and that's it!
    free(nodes);
    free(tree);
    return 0;
}

/*
 * Searches for an entry with the specified key in the tree.
 *
 * @param tree Tree to search into.
 * @param key Key to look for.
 * @param opts Looking for data stored in a node or the entire node?
 * @return Data stored in a node (if any) or pointer to the node (if any).
 */
void *splay_int_search(SplayIntTree *tree, int key, int opts) {
    if ((opts <= 0) || (tree == NULL)) return NULL;  // Sanity check.
    SplayIntNode *searchedNode = _search_splay_int_node(tree, key);
    if (searchedNode != NULL) {
        if (opts & SEARCH_DATA) return searchedNode->_data;
        if (opts & SEARCH_NODES) return searchedNode;
        return NULL;
    }
    return NULL;
}

/*
 * Deletes an entry from the tree.
 *
 * @param tree Pointer to the tree to delete from.
 * @param key Key to delete from the dictionary.
 * @param opts Also willing to free the stored data?
 * @return 1 if found and deleted, 0 if not found.
 */
int splay_int_delete(SplayIntTree *tree, int key, int opts) {
    // Sanity check on input arguments.
    if ((opts < 0) || (tree == NULL)) return 0;
    SplayIntNode *toDelete = _search_splay_int_node(tree, key);
    SplayIntNode *toFree;
    if (toDelete != NULL) {
        // Check whether the node has no sons or even one.
        if ((toDelete->_left_son == NULL) || (toDelete->_right_son == NULL)) {
            toFree = _spli_cut_one_son_node(toDelete);
        } else {
            // Find the node's predecessor and swap the content.
            SplayIntNode *maxLeft = _spli_max_key_son(toDelete->_left_son);
            _spli_swap_info(toDelete, maxLeft);
            // Remove the original predecessor.
            toFree = _spli_cut_one_son_node(maxLeft);
        }
        // Apply eventual options to free keys and data, then free the node.
        if (opts & DELETE_FREE_DATA) free(toFree->_data);
        free(toFree);
        tree->nodes_count--;
        // Check if the tree is now empty and update root pointer.
        if (tree->nodes_count == 0) tree->_root = NULL;
        return 1;  // Found and deleted.
    }
    return 0;  // Not found.
}

/*
 * Creates and inserts a new node in the tree.
 *
 * @param tree Pointer to the tree to insert into.
 * @param new_key New key to add to the dictionary.
 * @param new_data New data to store into the dictionary.
 * @return Internal nodes counter after the insertion.
 */
ulong splay_int_insert(SplayIntTree *tree, int new_key, void *new_data) {
    if (tree == NULL) return 0;  // Sanity check.
    if (tree->nodes_count == tree->max_nodes) return 0;  // The tree is full.
    SplayIntNode *new_node = _create_splay_int_node(new_key, new_data);
    if (tree->_root == NULL) {
        // The tree is empty.
        tree->_root = new_node;
        tree->nodes_count++;
    } else {
        // Look for the correct position and place it there.
        SplayIntNode *curr = tree->_root;
        SplayIntNode *pred = NULL;
        int comp;
        while (curr != NULL) {
            pred = curr;
            comp = curr->_key - new_key;
            if (comp >= 0) {
                // Equals are kept in the left subtree.
                curr = curr->_left_son;
            } else {
                curr = curr->_right_son;
            }
        }
        comp = pred->_key - new_key;
        if (comp >= 0) {
            _spli_insert_left_subtree(pred, new_node);
        } else {
            _spli_insert_right_subtree(pred, new_node);
        }
        _spli_splay_insert(new_node);
        tree->nodes_count++;
    }
    return tree->nodes_count;  // Return the result of the insertion.
}

/* 
 * Performs a depth-first search of the tree, the type of which can be
 * specified using the options defined in the header.
 * Depending on the option specified, returns an array of:
 * - Pointers to the nodes.
 * - Keys.
 * - Data.
 * See the header for the definitions of such options.
 * Remember to free the returned array afterwards!
 *
 * @param tree Pointer to the tree to operate on.
 * @param type Type of DFS to perform (see header).
 * @param opts Type of data to return (see header).
 * @return Pointer to an array with the result of the search correctly ordered.
 */
void **splay_int_dfs(SplayIntTree *tree, int type, int opts) {
    // Sanity check for the input arguments.
    if ((type <= 0) || (opts <= 0)) return NULL;
    if ((tree == NULL) || (tree->_root == NULL)) return NULL;
    // Allocate memory according to options.
    void **dfsRes;
    int int_opt;
    if (opts & SEARCH_DATA) {
        int_opt = SEARCH_DATA;
        dfsRes = calloc(tree->nodes_count, sizeof(void *));
    } else if (opts & SEARCH_KEYS) {
        int_opt = SEARCH_KEYS;
        dfsRes = calloc(tree->nodes_count, sizeof(int));
    } else if (opts & SEARCH_NODES) {
        int_opt = SEARCH_NODES;
        dfsRes = calloc(tree->nodes_count, sizeof(SplayIntNode *));
    } else return NULL;  // Invalid option.
    if (dfsRes == NULL) return NULL;  // calloc failed.
    // Launch the requested DFS according to type.
    void **int_ptr = dfsRes;
    if (type & DFS_PRE_ORDER) {
        _spli_preodfs(tree->_root, &int_ptr, int_opt);
    } else if (type & DFS_IN_ORDER) {
        _spli_inodfs(tree->_root, &int_ptr, int_opt);
    } else if (type & DFS_POST_ORDER) {
        _spli_postodfs(tree->_root, &int_ptr, int_opt);
    } else {
        // Invalid type.
        free(dfsRes);
        return NULL;
    }
    // The array is now filled with the requested data.
    return dfsRes;
}

/*
 * Performs a breadth-first search of the tree, the type of which can be
 * specified using the options defined in the header (left or right son
 * visited first).
 * Depending on the option specified, returns an array of:
 * - Pointers to the nodes.
 * - Keys.
 * - Data.
 * See the header for the definitions of such options.
 * Remember to free the returned array afterwards!
 * 
 * @param tree Pointer to the tree to operate on.
 * @param type Type of BFS to perform (see header).
 * @param opts Type of data to return (see header).
 * @return Pointer to an array with the result of the search correctly ordered.
 */
void **splay_int_bfs(SplayIntTree *tree, int type, int opts) {
    // Sanity check on input arguments.
    if ((tree == NULL) || (tree->_root == NULL) ||
        (type <= 0) || (opts <= 0) ||
        !((type & BFS_LEFT_FIRST) || (type & BFS_RIGHT_FIRST)) ||
        !((opts & SEARCH_KEYS) || (opts & SEARCH_DATA) ||
        (opts & SEARCH_NODES))) return NULL;
    // Allocate memory in the heap.
    void **bfsRes = NULL;
    void **int_ptr;
    int *keyPtr;  // Used only if keys are searched.
    if (opts & SEARCH_DATA) {
        bfsRes = calloc(tree->nodes_count, sizeof(void *));
    } else if (opts & SEARCH_KEYS) {
        bfsRes = calloc(tree->nodes_count, sizeof(SplayIntNode *));
        keyPtr = (int *) bfsRes;
    } else if (opts & SEARCH_NODES) {
        bfsRes = calloc(tree->nodes_count, sizeof(SplayIntNode *));
    } else return NULL;  // Invalid option.
    if (bfsRes == NULL) return NULL;  // Calloc failed.
    int_ptr = bfsRes + 1;
    *bfsRes = (void *) (tree->_root);
    SplayIntNode *curr;
    // Start the visit, using the same array to return as a temporary queue
    // for the nodes.
    for (unsigned long int i = 0; i < tree->nodes_count; i++) {
        curr = (SplayIntNode *) bfsRes[i];
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
            if (curr->_left_son != NULL) {
                *int_ptr = (void *) (curr->_left_son);
                int_ptr++;
            }
            if (curr->_right_son != NULL) {
                *int_ptr = (void *) (curr->_right_son);
                int_ptr++;
            }
        } else if (type & BFS_RIGHT_FIRST) {
            if (curr->_right_son != NULL) {
                *int_ptr = (void *) (curr->_right_son);
                int_ptr++;
            }
            if (curr->_left_son != NULL) {
                *int_ptr = (void *) (curr->_left_son);
                int_ptr++;
            }
        }
    }
    if (opts & SEARCH_KEYS) {
        // If keys were searched, part of the array (half of it on x86-64)
        // is totally unneeded, so we can release it.
        // reallocarray is used instead of realloc to account for possible size
        // computation overflows (see man).
        if ((bfsRes = reallocarray(bfsRes, (size_t) (tree->nodes_count),
                sizeof(int))) == NULL) {
            free(bfsRes);
            return NULL;
        }
    }
    return bfsRes;
}

// INTERNAL LIBRARY SUBROUTINES //
/*
 * Creates a new node in the heap. Requires an integer key and some data.
 *
 * @param new_key Key to add.
 * @param new_data Data to add.
 * @return Pointer to a new node.
 */
SplayIntNode *_create_splay_int_node(int new_key, void *new_data) {
    SplayIntNode *new_node = (SplayIntNode *)malloc(sizeof(SplayIntNode));
    if (new_node == NULL) return NULL;
    new_node->_father = NULL;
    new_node->_left_son = NULL;
    new_node->_right_son = NULL;
    new_node->_key = new_key;
    new_node->_data = new_data;
    return new_node;
}

/*
 * Frees memory occupied by a node.
 *
 * @param node Node to release.
 */
void _delete_splay_int_node(SplayIntNode *node) {
    free(node);
}

/*
 * Inserts a subtree rooted in a given node as the left subtree of a given
 * node.
 *
 * @param father Pointer to the node to root the subtree onto.
 * @param new_son Root of the subtree to add.
 */
void _spli_insert_left_subtree(SplayIntNode *father, SplayIntNode *new_son) {
    if (new_son != NULL) new_son->_father = father;
    father->_left_son = new_son;
}

/*
 * Inserts a subtree rooted in a given node as the right subtree of a given
 * node.
 *
 * @param father Pointer to the node to root the subtree onto.
 * @param new_son Root of the subtree to add.
 */
void _spli_insert_right_subtree(SplayIntNode *father, SplayIntNode *new_son) {
    if (new_son != NULL) new_son->_father = father;
    father->_right_son = new_son;
}

/*
 * Cuts and returns the left subtree of a given node.
 *
 * @param father Node to cut the subtree at.
 * @return Pointer to the cut subtree's root.
 */
SplayIntNode *_spli_cut_left_subtree(SplayIntNode *father) {
    SplayIntNode *son = father->_left_son;
    if (son == NULL) return NULL;  // Sanity check.
    son->_father = NULL;
    father->_left_son = NULL;
    return son;
}

/*
 * Cuts and returns the right subtree of a given node.
 *
 * @param father Node to cut the subtree at.
 * @return Pointer to the cut subtree's root.
 */
SplayIntNode *_spli_cut_right_subtree(SplayIntNode *father) {
    SplayIntNode *son = father->_right_son;
    if (son == NULL) return NULL;  // Sanity check.
    son->_father = NULL;
    father->_right_son = NULL;
    return son;
}

/*
 * Cuts and returns the subtree nested in a given node.
 *
 * @param father Node to cut the subtree at.
 * @return Pointer to the cut subtree's root.
 */
SplayIntNode *_spli_cut_subtree(SplayIntNode *node) {
    if (node == NULL) return NULL;  // Sanity check.
    if (node->_father == NULL) return node;  // Asked to cut at root.
    SplayIntNode *father = node->_father;
    if ((node->_left_son == NULL) && (node->_right_son == NULL)) {
        // The node is a leaf: distinguish between the node being a left or
        // right son and cut accordingly.
        if (father->_right_son == node) {
            father->_right_son = NULL;
        } else father->_left_son = NULL;
        node->_father = NULL;
        return node;
    } else if (father->_left_son == node) return _spli_cut_left_subtree(father);
    return _spli_cut_right_subtree(father);
}

/*
 * Returns the descendant of a given node with the greatest key.
 *
 * @param node Node for which to look for the descendant.
 * @return Pointer to the descendant node.
 */
SplayIntNode *_spli_max_key_son(SplayIntNode *node) {
    SplayIntNode *curr = node;
    while (curr->_right_son != NULL) curr = curr->_right_son;
    return curr;
}

/*
 * Returns a pointer to the node with the specified key, or NULL.
 *
 * @param tree Pointer to the tree to look into.
 * @param key Key to look for.
 * @return Pointer to the target node (if any).
 */
SplayIntNode *_search_splay_int_node(SplayIntTree *tree, int key) {
    if (tree->_root == NULL) return NULL;
    SplayIntNode *curr = tree->_root;
    int comp;
    while (curr != NULL) {
        comp = curr->_key - key;
        if (comp > 0) {
            curr = curr->_left_son;
        } else if (comp < 0) {
            curr = curr->_right_son;
        } else return curr;
    }
    return NULL;
}

/*
 * Swaps contents between two nodes.
 *
 * @param node1 First node.
 * @param node2 Second node.
 */
void _spli_swap_info(SplayIntNode *node1, SplayIntNode *node2) {
    int key1 = node1->_key;
    void *data1 = node1->_data;
    int key2 = node2->_key;
    void *data2 = node2->_data;
    node1->_key = key2;
    node2->_key = key1;
    node1->_data = data2;
    node2->_data = data1;
}

/*
 * Performs a simple right rotation at the specified node.
 *
 * @param node Node to rotate onto.
 */
void _spli_right_rotation(SplayIntNode *node) {
    SplayIntNode *leftSon = node->_left_son;
    // Swap the node and its son's contents to make it climb.
    _spli_swap_info(node, leftSon);
    // Shrink the tree portion in subtrees.
    SplayIntNode *rTree = _spli_cut_right_subtree(node);
    SplayIntNode *lTree = _spli_cut_left_subtree(node);
    SplayIntNode *lTree_l = _spli_cut_left_subtree(leftSon);
    SplayIntNode *lTree_r = _spli_cut_right_subtree(leftSon);
    // Recombine portions to respect the search property.
    _spli_insert_right_subtree(lTree, rTree);
    _spli_insert_left_subtree(lTree, lTree_r);
    _spli_insert_right_subtree(node, lTree);
    _spli_insert_left_subtree(node, lTree_l);
    // Update the height of the involved nodes.
    _intUpdateHeight(node->_right_son);
    _intUpdateHeight(node);
}

/*
 * Performs a simple left rotation at the specified node.
 *
 * @param node Node to rotate onto.
 */
void _spli_left_rotation(SplayIntNode *node) {
    SplayIntNode *rightSon = node->_right_son;
    // Swap the node and its son's contents to make it climb.
    _spli_swap_info(node, rightSon);
    // Shrink the tree portion in subtrees.
    SplayIntNode *rTree = _spli_cut_right_subtree(node);
    SplayIntNode *lTree = _spli_cut_left_subtree(node);
    SplayIntNode *rTree_l = _spli_cut_left_subtree(rightSon);
    SplayIntNode *rTree_r = _spli_cut_right_subtree(rightSon);
    // Recombine portions to respect the search property.
    _spli_insert_left_subtree(rTree, lTree);
    _spli_insert_right_subtree(rTree, rTree_l);
    _spli_insert_left_subtree(node, rTree);
    _spli_insert_right_subtree(node, rTree_r);
    // Update the height of the involved nodes.
    _intUpdateHeight(node->_left_son);
    _intUpdateHeight(node);
}

/*
 * Examines the balance factor of a given node and eventually rotates.
 */
/* TODO Deprecated? */
void _spli_rotate(SplayIntNode *node) {
    int balFactor = _intBalanceFactor(node);
    if (balFactor == 2) {
        if (_intBalanceFactor(node->_left_son) >= 0) {
            // LL displacement: rotate right.
            _spli_right_rotation(node);
        } else {
            // LR displacement: apply double rotation.
            _spli_left_rotation(node->_left_son);
            _spli_right_rotation(node);
        }
    } else if (balFactor == -2) {
        if (_intBalanceFactor(node->_right_son) <= 0) {
            // RR displacement: rotate left.
            _spli_left_rotation(node);
        } else {
            // RL displacement: apply double rotation.
            _spli_right_rotation(node->_right_son);
            _spli_left_rotation(node);
        }
    }
}

// TODO Must be refactored into splay heuristic for insertion.
/* Updates heights and looks for displacements following an insertion. */
void _spli_splay_insert(SplayIntNode *new_node) {
    SplayIntNode *curr = new_node->_father;
    while (curr != NULL) {
        if (abs(_intBalanceFactor(curr)) >= 2) {
            // Unbalanced node found.
            break;
        } else {
            _intUpdateHeight(curr);
            curr = curr->_father;
        }
    }
    if (curr != NULL) _spli_rotate(curr);
}

// TODO Must be refactored into splay heuristic for deletion.
/* Updates heights and looks for displacements following a deletion. */
void _spli_splay_delete(SplayIntNode *rem_father) {
    SplayIntNode *curr = rem_father;
    while (curr != NULL) {
        if (abs(_intBalanceFactor(curr)) >= 2) {
            // There may be more than one unbalanced node.
            _spli_rotate(curr);
        } else _intUpdateHeight(curr);
        curr = curr->_father;
    }
}

// TODO Add splay heuristic for search?

/*
 * Cuts a node with a single son.
 *
 * @param node Node to cut.
 * @return Pointer to the disconnected node.
 */
SplayIntNode *_spli_cut_one_son_node(SplayIntNode *node) {
    SplayIntNode *son = NULL;
    SplayIntNode *father = node->_father;
    if (node->_left_son != NULL) {
        son = node->_left_son;
    } else if (node->_right_son != NULL) {
        son = node->_right_son;
    }
    if (son == NULL) {  // The node is a leaf.
        son = _spli_cut_subtree(node);  // Will be returned later.
    } else {
        // Swap the content from the son to the father.
        _spli_swap_info(node, son);
        // Cut the node and balance the deletion.
        _spli_cut_subtree(son);
        _spli_insert_right_subtree(node, _spli_cut_subtree(son->_right_son));
        _spli_insert_left_subtree(node, _spli_cut_subtree(son->_left_son));
    }
    _spli_splay_delete(father);
    return son;  // Return the node to free, now totally disconnected.
}

/*
 * Performs an in-order, recursive DFS.
 *
 * @param root_node Root of the subtree to start the search onto.
 * @param int_ptr Internal pointer to a pointer to the return array.
 * @param int_opt Internal options passed value.
 */
void _spli_inodfs(SplayIntNode *root_node, void ***int_ptr, int int_opt) {
    // Recursion base step.
    if (root_node == NULL) {
        // Correctly update the pointer.
        if (int_opt & SEARCH_KEYS) {
            *(int_ptr) = (void **)((int *)*(int_ptr) - 1);
        } else *(int_ptr) = *(int_ptr) - 1;
        return;
    }
    // Recursive step: visit the left son.
    _spli_inodfs(root_node->_left_son, int_ptr, int_opt);
    // Correctly increment the internal pointer.
    if (int_opt & SEARCH_KEYS) {
        *(int_ptr) = (void **)((int *)*(int_ptr) + 1);
    } else *(int_ptr) = *(int_ptr) + 1;
    // Now visit the root node.
    if (int_opt & SEARCH_NODES) {
        **(int_ptr) = root_node;
    } else if (int_opt & SEARCH_KEYS) {
        *(int *)*(int_ptr) = root_node->_key;
    } else if (int_opt & SEARCH_DATA) {
        **(int_ptr) = root_node->_data;
    }
    // Correctly increment the internal pointer.
    if (int_opt & SEARCH_KEYS) {
        *(int_ptr) = (void **)((int *)*(int_ptr) + 1);
    } else *(int_ptr) = *(int_ptr) + 1;
    // Visit the right son and return.
    _spli_inodfs(root_node->_right_son, int_ptr, int_opt);
}

/*
 * Performs a pre-order, recursive DFS.
 *
 * @param root_node Root of the subtree to start the search onto.
 * @param int_ptr Internal pointer to a pointer to the return array.
 * @param int_opt Internal options passed value.
 */
void _spli_preodfs(SplayIntNode *root_node, void ***int_ptr, int int_opt) {
    // Recursion base step.
    if (root_node == NULL) {
        // Correctly update the pointer.
        if (int_opt & SEARCH_KEYS) {
            *(int_ptr) = (void **)((int *)*(int_ptr) - 1);
        } else *(int_ptr) = *(int_ptr) - 1;
        return;
    }
    // Recursive step.
    // Visit the root node.
    if (int_opt & SEARCH_NODES) {
        **(int_ptr) = root_node;
    } else if (int_opt & SEARCH_KEYS) {
        *(int *)*(int_ptr) = root_node->_key;
    } else if (int_opt & SEARCH_DATA) {
        **(int_ptr) = root_node->_data;
    }
    // Correctly increment the internal pointer.
    if (int_opt & SEARCH_KEYS) {
        *(int_ptr) = (void **)((int *)*(int_ptr) + 1);
    } else *(int_ptr) = *(int_ptr) + 1;
    // Now visit the left son.
    _spli_preodfs(root_node->_left_son, int_ptr, int_opt);
    // Correctly increment the internal pointer.
    if (int_opt & SEARCH_KEYS) {
        *(int_ptr) = (void **)((int *)*(int_ptr) + 1);
    } else *(int_ptr) = *(int_ptr) + 1;
    // Visit the right son and return.
    _spli_preodfs(root_node->_right_son, int_ptr, int_opt);
}

/*
 * Performs a post-order, recursive DFS.
 *
 * @param root_node Root of the subtree to start the search onto.
 * @param int_ptr Internal pointer to a pointer to the return array.
 * @param int_opt Internal options passed value.
 */
void _spli_postodfs(SplayIntNode *root_node, void ***int_ptr, int int_opt) {
    // Recursion base step.
    if (root_node == NULL) {
        // Correctly update the pointer.
        if (int_opt & SEARCH_KEYS) {
            *(int_ptr) = (void **)((int *)*(int_ptr) - 1);
        } else *(int_ptr) = *(int_ptr) - 1;
        return;
    }
    // Recursive step.
    // Visit the left son.
    _spli_postodfs(root_node->_left_son, int_ptr, int_opt);
    // Correctly increment the internal pointer.
    if (int_opt & SEARCH_KEYS) {
        *(int_ptr) = (void **)((int *)*(int_ptr) + 1);
    } else *(int_ptr) = *(int_ptr) + 1;
    // Visit the right son.
    _spli_postodfs(root_node->_right_son, int_ptr, int_opt);
    // Correctly increment the internal pointer.
    if (int_opt & SEARCH_KEYS) {
        *(int_ptr) = (void **)((int *)*(int_ptr) + 1);
    } else *(int_ptr) = *(int_ptr) + 1;
    // Visit the root node and return.
    if (int_opt & SEARCH_NODES) {
        **(int_ptr) = root_node;
    } else if (int_opt & SEARCH_KEYS) {
        *(int *)*(int_ptr) = root_node->_key;
    } else if (int_opt & SEARCH_DATA) {
        **(int_ptr) = root_node->_data;
    }
}
