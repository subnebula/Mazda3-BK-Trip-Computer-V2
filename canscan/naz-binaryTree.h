/* Nathaniel Roach */

#ifndef _NAZ_BINARYTREE_H_
#define _NAZ_BINARYTREE_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BinaryTreeNode {
    struct BinaryTreeNode *parent;
    uint16_t key;
    void *data;
    struct BinaryTreeNode *childLeft;
    struct BinaryTreeNode *childRight;
} BinaryTreeNode;

typedef struct BinaryTree {
    BinaryTreeNode *root;
    int size;
} BinaryTree;

BinaryTree *binaryTreeCreate();
BinaryTreeNode *binaryTreeFindR(BinaryTree *subjTree, int inKey, int retParent);
BinaryTreeNode *binaryTreeInsert(BinaryTree *subjTree, int inKey, void *inData);
void *binaryTreeDelete(BinaryTree *subjTree, int inKey);
BinaryTreeNode *getLeftmostRightSide(BinaryTreeNode *ParentNode, int retParent);

#ifdef __cplusplus
}
#endif

#endif
