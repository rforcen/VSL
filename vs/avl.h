#ifndef AVL_H
#define AVL_H

// C++ program to delete a node from AVL Tree
#include <stdlib.h>
#include <iostream>

using namespace std;

// An AVL tree node
class Node {
 public:
  int key;
  Node *left = nullptr;
  Node *right = nullptr;
  int height = 1;

  Node() {}
  Node(int key) : key(key) {}

  int get_height(Node *node) { return node ? node->height : 0; }

  // A utility function to get maximum
  // of two integers
  int max(Node *a, Node *b) {
    int ha = (a != nullptr) ? a->height : 0,
        hb = (b != nullptr) ? b->height : 0;
    return (ha > hb) ? ha : hb;
  }

  // A utility function to right
  // rotate subtree rooted with y
  // See the diagram given above.
  Node *rightRotate(Node *y) {
    Node *x = y->left, *T2 = x->right;

    // Perform rotation
    x->right = y;
    y->left = T2;

    // Update heights
    y->height = max(y->left, y->right) + 1;
    x->height = max(x->left, x->right) + 1;

    // Return new root
    return x;
  }

  // A utility function to left
  // rotate subtree rooted with x
  // See the diagram given above.
  Node *leftRotate(Node *x) {
    Node *y = x->right, *T2 = y->left;

    // Perform rotation
    y->left = x;
    x->right = T2;

    // Update heights
    height = max(x->left, x->right) + 1;
    y->height = max(y->left, y->right) + 1;

    // Return new root
    return y;
  }

  // Get Balance factor of node N
  int getBalance() { return get_height(left) - get_height(right); }

  Node &operator=(const Node &n) {
    memcpy(this, &n, sizeof(Node));
    return *this;
  }
  Node(const Node *n) { memcpy(this, n, sizeof(Node)); }

  void insert(int key) {
    Node *n = insert(this, key);
    memcpy(this, n, sizeof(Node));
  }

  Node *insert(Node *node, int key) {
    /* 1. Perform the normal BST rotation */
    if (node == nullptr) return new Node(key);

    if (key < node->key)
      node->left = node->left->insert(node->left, key);
    else if (key > this->key)
      node->right = node->right->insert(node->right, key);
    else  // Equal keys not allowed
      return node;

    /* 2. Update height of this ancestor node */
    node->height = 1 + max(node->left, node->right);

    /* 3. Get the balance factor of this
            ancestor node to check whether
            this node became unbalanced */
    int balance = node->getBalance();

    // If this node becomes unbalanced,
    // then there are 4 cases

    // Left Left Case
    if (balance > 1 && key < node->left->key) return rightRotate(node);

    // Right Right Case
    if (balance < -1 && key > node->right->key) return leftRotate(node);

    // Left Right Case
    if (balance > 1 && key > node->left->key) {
      node->left = leftRotate(node->left);
      return rightRotate(node);
    }

    // Right Left Case
    if (balance < -1 && key < right->key) {
      node->right = rightRotate(node->right);
      return leftRotate(node);
    }

    return node;
  }

  /* Given a non-empty binary search tree,
  return the node with minimum key value
  found in that tree. Note that the entire
  tree does not need to be searched. */
  Node *minValueNode(Node *node) {
    Node *current = node;

    /* loop down to find the leftmost leaf */
    while (current->left != nullptr) current = current->left;

    return current;
  }

  // Recursive function to delete a node
  // with given key from subtree with
  // given root. It returns root of the
  // modified subtree.
  Node *deleteNode(int key) {
    // STEP 1: PERFORM STANDARD BST DELETE

    // If the key to be deleted is smaller
    // than the root's key, then it lies
    // in left subtree
    if (key < this->key) left = left->deleteNode(key);

    // If the key to be deleted is greater
    // than the this's key, then it lies
    // in right subtree
    else if (key > this->key)
      right = right->deleteNode(key);

    // if key is same as this's key, then
    // This is the node to be deleted
    else {
      // node with only one child or no child
      if ((left == nullptr) || (right == nullptr)) {
        Node *temp = left ? left : right;

        // No child case
        if (temp == nullptr) {
          temp = this;
        } else            // One child case
          *this = *temp;  // Copy the contents of
                          // the non-empty child
        delete temp;
      } else {
        // node with two children: Get the inorder
        // successor (smallest in the right subtree)
        Node *temp = minValueNode(right);

        // Copy the inorder successor's
        // data to this node
        key = temp->key;

        // Delete the inorder successor
        right = right->deleteNode(temp->key);
      }
    }

    // If the tree had only one node
    // then return
    //    if (this == nullptr) return this;

    // STEP 2: UPDATE HEIGHT OF THE CURRENT NODE
    height = 1 + max(left, right);

    // STEP 3: GET THE BALANCE FACTOR OF
    // THIS NODE (to check whether this
    // node became unbalanced)
    int balance = getBalance();

    // If this node becomes unbalanced,
    // then there are 4 cases

    // Left Left Case
    if (balance > 1 && left->getBalance() >= 0) return rightRotate(this);

    // Left Right Case
    if (balance > 1 && left->getBalance() < 0) {
      left = leftRotate(left);
      return rightRotate(this);
    }

    // Right Right Case
    if (balance < -1 && right->getBalance() <= 0) return leftRotate(this);

    // Right Left Case
    if (balance < -1 && right->getBalance() > 0) {
      right = rightRotate(right);
      return leftRotate(this);
    }

    return this;
  }

  // A utility function to print preorder
  // traversal of the tree.
  // The function also prints height
  // of every node
  void preOrder() { preOrder(this); }
  void preOrder(Node *node) {
    if (node) {
      cout << key << " ";
      preOrder(node->left);
      preOrder(node->right);
    }
  }
};

// Driver Code
static int test_avl() {
  Node root;

  /* Constructing tree given in
  the above figure */
  root.insert(9);
  root.insert(5);
  root.insert(10);
  root.insert(0);
  root.insert(6);
  root.insert(11);
  root.insert(-1);
  root.insert(1);
  root.insert(2);

  /* The constructed AVL Tree would be
                  9
          / \
          1 10
          / \ \
  0 5 11
  / / \
  -1 2 6
  */

  cout << "Preorder traversal of the "
          "constructed AVL tree is \n";
  root.preOrder();

  root.deleteNode(10);

  /* The AVL Tree after deletion of 10
                  1
          / \
          0 9
          / / \
  -1 5	 11
          / \
          2 6
  */

  cout << "\nPreorder traversal after"
       << " deletion of 10 \n";
  root.preOrder();

  return 0;
}

// This code is contributed by rathbhupendra

#endif  // AVL_H
