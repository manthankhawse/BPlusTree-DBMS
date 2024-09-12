#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define MIN_DEG 3

typedef struct Node{
    int* keys;
    int t;
    struct Node** children;

    int n;

    bool leaf;
    struct Node* next;
} Node;

typedef struct Btree{
    Node* root;
    int t;
} Btree;

Node* createNode(int t, bool leaf){
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->t = t;
    newNode->leaf = leaf;
    newNode->keys = (int*)malloc((2*t-1)*sizeof(int));
    newNode->children = (Node**)malloc((2*t)*sizeof(Node*));
    newNode->n = 0;
    newNode->next = NULL;
    return newNode;
}

Btree* createBtree(int t){
    Btree* btree = (Btree*)malloc(sizeof(Btree));
    btree->t = t;
    btree->root = createNode(t, true);
    return btree;
}

void display(Node* node){

    int i;

    if(node==NULL){
        return;
    }

    for(i = 0; i<node->n; i++){
        if(!node->leaf){
            display(node->children[i]);
        }
        printf("%d ", node->keys[i]);
    }

    if(!node->leaf){
        display(node->children[i]);
    }
}

bool search(Node* node, int key){
    int i = 0;
    while(i<node->n && key>node->keys[i]){
        i++;
    }

    if(i<node->n && key==node->keys[i]){
        return true;
    }

    if(node->leaf){
        return false;
    }

    return search(node->children[i], key);
} 


void splitChild(Node* parent, int i, Node* child){
    int t = child->t;
    Node* newChild = createNode(t, child->leaf);
    newChild->n = t-1;

    for(int j = 0; j<t-1; j++){
        newChild->keys[j] = child->keys[j+t];
    }

    if(!child->leaf){
        for(int j = 0; i<t; j++){
            newChild->children[j] = child->children[j+t];
        }
    }

    child->n = t-1;

    for(int j = parent->n; j>=i+1; j--){
        parent->children[j+1] = parent->children[j];
    }

    parent->children[i+1] = newChild;

    for(int j = parent->n-1; j>=i; j--){
        parent->keys[j+1] = parent->keys[j];
    }

    parent->keys[i] = child->keys[t-1];
    parent->n += 1;
}

void insertNonFull(Node* node, int key){
    int i = node->n-1;

    if(node->leaf){
        while(i>=0 && node->keys[i]>key){
            node->keys[i+1] = node->keys[i];
            i--;
        }

        node->keys[i+1] = key;
        node->n += 1;
    }
    else{
        while(i>=0 && node->keys[i] > key){
            i--;
        }

        i++;
        if(node->children[i]->n==2*node->t-1){
            splitChild(node, i, node->children[i]);
            if(node->keys[i]<key){
                i++;
            }
        }
        insertNonFull(node->children[i], key);
    }
}

void insert(Btree* btree, int key){
    Node* root = btree->root;
    if(root->n==2*btree->t-1){
        Node* newRoot = createNode(btree->t, false);
        newRoot->children[0] = root;
        insertNonFull(newRoot, key);
        btree->root = newRoot;
    }else{
        insertNonFull(root, key);
    }
}

void deleteKeyHelper(Node* node, int key);
int findKey(Node* node, int key);
void removeFromLeaf(Node* node, int idx);
int getPredecessor(Node* node, int idx);
void fill(Node* node, int idx);
void borrowFromPrev(Node* node, int idx);
void borrowFromNext(Node* node, int idx);
void merge(Node* node, int idx);

void deleteKey(Btree* btree, int key)
{
    Node* root = btree->root;

    // Call a helper function to delete the key recursively
    deleteKeyHelper(root, key);

    // If root has no keys left and it has a child, make its
    // first child the new root
    if (root->n == 0 && !root->leaf) {
        btree->root = root->children[0];
        free(root);
    }
}

void deleteKeyHelper(Node* node, int key){
    int idx = findKey(node, key);

    if(idx<node->n && node->keys[idx]==key){
        if(node->leaf){
            removeFromLeaf(node,idx);
        }else{
            int predecessor = getPredecessor(node, idx);
            node->keys[idx] = predecessor;

            deleteKeyHelper(node->children[idx], predecessor);
        }
    }else{
        if(node->leaf){
            printf("Key not found\n");
            return;
        }

        bool isLastChild = (idx==node->n);

        if(node->children[idx]->n < node->t){
            fill(node, idx);
        }

        if(isLastChild && idx>node->n){
            deleteKeyHelper(node->children[idx-1], key);
        }else{
            deleteKeyHelper(node->children[idx], key);
        }
    }
}

int findKey(Node* node, int key){
    int idx = 0;
    while(idx<node->n && key>node->keys[idx]){
        idx++;
    }
    return idx;
}

void removeFromLeaf(Node* node, int idx){
    for(int i = idx+1; i<node->n; ++i){
        node->keys[i-1] = node->keys[i];
    }

    node->n--;
}

int getPredecessor(Node* node, int idx){
    Node* curr = node->children[idx];
    while(!curr->leaf){
        curr = curr->children[curr->n-1];
    }

    return curr->keys[curr->n-1];
}

void fill(Node* node, int idx){
    if(idx!=0 && node->children[idx-1]->n>=node->t){
        borrowFromPrev(node, idx);
    }else if(idx!=node->n && node->children[idx+1]->n>=node->t){
        borrowFromNext(node, idx);
    }else{
        if(idx!=node->n){
            merge(node, idx);
        }else{
            merge(node, idx-1);
        }
    }
}

void borrowFromPrev(Node* node, int idx){
    Node* child = node->children[idx];
    Node* sibling = node->children[idx-1];

    for(int i = child->n-1; i>=0; --i){
        child->keys[i+1] = child->keys[i];
    }

    if(!child->leaf){
        for(int i = child->n; i>=0; --i){
            child->children[i+1] = child->children[i];
        }
    }

    child->keys[0] = node->keys[idx-1];

    if(!child->leaf){
        child->children[0] = sibling->children[sibling->n];
    }

    node->keys[idx-1] = sibling->keys[sibling->n-1];

    child->n+=1;
    sibling->n+=1;
}


void borrowFromNext(Node* node, int idx)
{
    Node* child = node->children[idx];
    Node* sibling = node->children[idx + 1];

    // Setting child's (t - 1)th key equal to node's
    // key[idx]
    child->keys[(child->n)] = node->keys[idx];

    // If child is not a leaf, move its child pointers one
    // step ahead
    if (!child->leaf) {
        child->children[(child->n) + 1]
            = sibling->children[0];
    }

    // Setting node's idx-th key equal to sibling's first
    // key
    node->keys[idx] = sibling->keys[0];

    // Moving all keys in sibling one step behind
    for (int i = 1; i < sibling->n; ++i) {
        sibling->keys[i - 1] = sibling->keys[i];
    }

    // If sibling is not a leaf, move its child pointers one
    // step behind
    if (!sibling->leaf) {
        for (int i = 1; i <= sibling->n; ++i) {
            sibling->children[i - 1] = sibling->children[i];
        }
    }

    // Incrementing and decrementing the key counts of child
    // and sibling respectively
    child->n += 1;
    sibling->n -= 1;
}

void merge(Node* node, int idx)
{
    Node* child = node->children[idx];
    Node* sibling = node->children[idx + 1];

    // Pulling a key from the current node and inserting it
    // into (t-1)th position of child
    child->keys[child->n] = node->keys[idx];

    // If child is not a leaf, move its child pointers one
    // step ahead
    if (!child->leaf) {
        child->children[child->n + 1]
            = sibling->children[0];
    }

    // Copying the keys from sibling to child
    for (int i = 0; i < sibling->n; ++i) {
        child->keys[i + child->n + 1] = sibling->keys[i];
    }

    // If child is not a leaf, copy the children pointers as
    // well
    if (!child->leaf) {
        for (int i = 0; i <= sibling->n; ++i) {
            child->children[i + child->n + 1]
                = sibling->children[i];
        }
    }

    // Move all keys after idx in the current node one step
    // before, so as to fill the gap created by moving
    // keys[idx] to child
    for (int i = idx + 1; i < node->n; ++i) {
        node->keys[i - 1] = node->keys[i];
    }

    // Move the child pointers after (idx + 1) in the
    // current node one step before
    for (int i = idx + 2; i <= node->n; ++i) {
        node->children[i - 1] = node->children[i];
    }

    // Update the key count of child and current node
    child->n += sibling->n + 1;
    node->n--;

    // Free the memory occupied by sibling
    free(sibling);
}

int main()
{
    Btree* btree = createBtree(MIN_DEG);

    // Insert elements into the B+ tree
    insert(btree, 2);
    insert(btree, 4);
    insert(btree, 7);
    insert(btree, 10);
    insert(btree, 17);
    insert(btree, 21);
    insert(btree, 28);

    // Print the B+ tree
    printf("B+ Tree after insertion: ");
    display(btree->root);
    printf("\n");

    // Search for a key
    int key_to_search = 17;
    bool found = search(btree->root, key_to_search);

    if (found) {
        printf("Key %d found in the B+ tree.\n",
               key_to_search);
    }
    else {
        printf("Key %d not found in the B+ tree.\n",
               key_to_search);
    }

    // Delete element from the B+ tree
    deleteKey(btree, 17);

    // Print the B+ tree after deletion
    printf("B+ Tree after deletion: ");
    display(btree->root);
    printf("\n");

    found = search(btree->root, key_to_search);

    if (found) {
        printf("Key %d found in the B+ tree.\n",
               key_to_search);
    }
    else {
        printf("Key %d not found in the B+ tree.\n",
               key_to_search);
    }

    return 0;
}


