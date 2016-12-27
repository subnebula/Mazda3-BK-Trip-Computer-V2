/* Nathaniel Roach*/

#include <stdlib.h>
#include <stdio.h>
#include "naz-binaryTree.h"

#ifdef __cplusplus
extern "C" {
#endif

BinaryTree *binaryTreeCreate(){
    BinaryTree *subjTree;

    subjTree = calloc(1, sizeof(BinaryTree));
    if (subjTree == NULL){ // Error in calloc, bail
        return NULL;
    }
    (*subjTree).root = NULL;
    (*subjTree).size = 0;

    return subjTree;
}

static BinaryTreeNode *binaryTreeFindRecurs(BinaryTreeNode *subjNode, int inKey){
    int currentKey;
    BinaryTreeNode *retval = NULL;

    if (subjNode == NULL){
        return NULL; // Exit here to make the logic a bit simpler for below
    }

    currentKey = (*subjNode).key;

    if (currentKey == inKey){
        retval = subjNode;
    } else if (inKey < currentKey){
        retval = binaryTreeFindRecurs((*subjNode).childLeft, inKey);
    } else if (inKey > currentKey){
        retval = binaryTreeFindRecurs((*subjNode).childRight, inKey);
    }

    if (retval == NULL){ // If we passed if a child that doesn't exist, return parent
        retval = subjNode;
    }

    return retval;
}

// Find a node in the binary tree - if retParent is non-zero it will return
// the parent of the location instead, for insertion.
BinaryTreeNode *binaryTreeFindR(BinaryTree *subjTree, int inKey, int retParent){
    BinaryTreeNode *retval;

    if (subjTree == NULL){
        return NULL; // Can't do anything here
    }

    retval = binaryTreeFindRecurs((*subjTree).root, inKey);

    if (((*retval).key != inKey) && !retParent){
        retval = NULL;
    }

    return retval;
}

BinaryTreeNode *binaryTreeInsert(BinaryTree *subjTree, int inKey, void *inData){
    int parentKey;
    BinaryTreeNode *subjNode, *subjParent;
    if (subjTree == NULL){
        return NULL; // Can't do anything here
    }

    if ((*subjTree).size > 0){
        subjParent = binaryTreeFindR(subjTree, inKey, 1);
        parentKey = (*subjParent).key;

        if (parentKey == inKey){
            subjNode = NULL;
        } else if (inKey < parentKey){
            if ((*subjParent).childLeft == NULL){
                (*subjParent).childLeft = subjNode = calloc(1, sizeof(BinaryTreeNode));
                if (subjNode == NULL){
                    return NULL;
                }
                (*subjTree).size++;
                (*subjNode).parent = subjParent;
                (*subjNode).key = inKey;
                (*subjNode).data = inData;
                (*subjNode).childLeft = NULL;
                (*subjNode).childRight = NULL;
            }
        } else if (inKey > parentKey){
            if ((*subjParent).childRight == NULL){
                (*subjParent).childRight = subjNode = calloc(1, sizeof(BinaryTreeNode));
                if (subjNode == NULL){
                    return NULL;
                }
                (*subjTree).size++;
                (*subjNode).parent = subjParent;
                (*subjNode).key = inKey;
                (*subjNode).data = inData;
                (*subjNode).childLeft = NULL;
                (*subjNode).childRight = NULL;
            }
        }
    } else if ((*subjTree).size == 0){ // Inserting into empty tree
        (*subjTree).root = subjNode =  calloc(1, sizeof(BinaryTreeNode));
        if (subjNode == NULL){
            return NULL;
        }
        (*subjTree).size++;
        (*subjNode).key = inKey;
        (*subjNode).data = inData;
        (*subjNode).parent = NULL;
        (*subjNode).childLeft = NULL;
        (*subjNode).childRight = NULL;
    }


    return subjNode;
}

void *binaryTreeDelete(BinaryTree *subjTree, int inKey){
  BinaryTreeNode *subjNode;
  BinaryTreeNode *successor;
  int nodeIsRoot = 0;
  void *subjData;
  subjNode = binaryTreeFindR(subjTree, inKey, 0);

  if ((*subjNode).childLeft == NULL && (*subjNode).childRight == NULL){ // If Neither
    subjData = (*subjNode).data;
    free(subjNode);
  } else if ((*subjNode).childLeft != NULL && (*subjNode).childRight != NULL){ // If Both
    successor = getLeftmostRightSide(subjNode, 0);

    // Tell the new grandparent
    if ((*subjNode).parent == NULL){
      nodeIsRoot = 1;
    } else if ((*(*subjNode).parent).key > (*subjNode).key){
      (*(*subjNode).parent).childLeft = successor;
    } else if ((*(*subjNode).parent).key < (*subjNode).key){
      (*(*subjNode).parent).childRight = successor;
    } else {
      exit(22); //banana
    }

    // Clear the original grandparent's memory
    if ((*(*successor).parent).key > (*successor).key){
      (*(*successor).parent).childLeft = NULL;
    } else if ((*(*successor).parent).key < (*successor).key){
      (*(*successor).parent).childRight = NULL;
    } else {
      exit(23); //banana
    }

    // Tell the new children
    if (nodeIsRoot){
      (*subjTree).root = successor;
    } else {
      (*(*subjNode).childLeft).parent = successor;
      (*(*subjNode).childRight).parent = successor;
    }

    (*successor).childLeft = (*subjNode).childLeft;
    (*successor).childRight = (*subjNode).childRight;

    subjData = (*subjNode).data;
    free(subjNode);

  } else { // One Child

    // Find the 'prodigy' child
    if ((*subjNode).childLeft != NULL){
      successor = (*subjNode).childLeft;
    } else if ((*subjNode).childRight != NULL){
      successor = (*subjNode).childRight;
    }

    // Find the parent's slot that we are re-using
    if ((*(*subjNode).parent).key > (*subjNode).key){
      (*(*subjNode).parent).childLeft = successor;
    } else if ((*(*subjNode).parent).key < (*subjNode).key){
      (*(*subjNode).parent).childRight = successor;
    } else {
      exit(24); //banana
    }

    subjData = (*subjNode).data;
    free(subjNode);
  }

  return(subjData);
}

BinaryTreeNode *getLeftmostRightSide(BinaryTreeNode *ParentNode, int retParent){
  BinaryTreeNode *subjNode;
  int looping = 1;

  if (ParentNode == NULL) return NULL;

  subjNode = (*ParentNode).childRight;

  while (looping){
    if ((*subjNode).childLeft != NULL){
      subjNode = (*subjNode).childLeft;
    } else looping = 0;
  }

  if (retParent){
    subjNode = (*subjNode).parent;
  }

  return(subjNode);
}

#ifdef __cplusplus
}
#endif
