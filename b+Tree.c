#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIN_DEGREE 3 

typedef struct UserData {
    int id;
    char username[100];
    char password[100];
} UserData;

typedef struct Node {
    UserData* records;
    int t;
    struct Node** children;
    int n;
    bool leaf;
    struct Node* next;
} Node;

typedef struct BTree {
    Node* root;
    int t;
} BTree;

UserData* createUser(int id, const char* username, const char* password) {
    UserData* newUser = (UserData*) malloc(sizeof(UserData));
    newUser->id = id;
    strcpy(newUser->username, username);
    strcpy(newUser->password, password);
    return newUser;
}

Node* createNode(int t, bool leaf)
{
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->t = t;
    newNode->leaf = leaf;
    newNode->records = (UserData*)malloc((2 * t - 1) * sizeof(UserData));
    newNode->children
        = (Node**)malloc((2 * t) * sizeof(Node*));
    newNode->n = 0;
    newNode->next = NULL;
    return newNode;
}

BTree* createBTree(int t)
{
    BTree* btree = (BTree*)malloc(sizeof(BTree));
    btree->t = t;
    btree->root = createNode(t, true);
    return btree;
}

void display(Node* node)
{
    if (node == NULL)
        return;
    int i;
    for (i = 0; i < node->n; i++) {
        if (!node->leaf) {
            display(node->children[i]);
        }
        printf("[ID: %d, Username: %s, Password: %s] \n", node->records[i].id, node->records[i].username, node->records[i].password);
    }
    if (!node->leaf) {
        display(node->children[i]);
    }
}

bool search(Node* node, int id, UserData* result)
{
    int i = 0;
    while (i < node->n && id > node->records[i].id) {
        i++;
    }
    if (i < node->n && id == node->records[i].id) {
        if (result != NULL) *result = node->records[i];
        return true;
    }
    if (node->leaf) {
        return false;
    }
    return search(node->children[i], id, result);
}

void splitChild(Node* parent, int i, Node* child)
{
    int t = child->t;
    Node* newChild = createNode(t, child->leaf);
    newChild->n = t - 1;

    for (int j = 0; j < t - 1; j++) {
        newChild->records[j] = child->records[j + t];
    }

    if (!child->leaf) {
        for (int j = 0; j < t; j++) {
            newChild->children[j] = child->children[j + t];
        }
    }

    child->n = t - 1;

    for (int j = parent->n; j >= i + 1; j--) {
        parent->children[j + 1] = parent->children[j];
    }
    parent->children[i + 1] = newChild;

    for (int j = parent->n - 1; j >= i; j--) {
        parent->records[j + 1] = parent->records[j];
    }
    parent->records[i] = child->records[t - 1];
    parent->n += 1;
}

void insertNonFull(Node* node, UserData* row)
{
    int i = node->n - 1;

    if (node->leaf) {
        while (i >= 0 && node->records[i].id > row->id) {
            node->records[i + 1] = node->records[i];
            i--;
        }
        node->records[i + 1] = *row;
        node->n += 1;
    }
    else {
        while (i >= 0 && node->records[i].id > row->id) {
            i--;
        }
        i++;
        if (node->children[i]->n == 2 * node->t - 1) {
            splitChild(node, i, node->children[i]);
            if (node->records[i].id < row->id) {
                i++;
            }
        }
        insertNonFull(node->children[i], row);
    }
}

void insert(BTree* btree, UserData* row)
{
    Node* root = btree->root;
    if (root->n == 2 * btree->t - 1) {
        Node* newRoot = createNode(btree->t, false);
        newRoot->children[0] = root;
        splitChild(newRoot, 0, root);
        insertNonFull(newRoot, row);
        btree->root = newRoot;
    }
    else {
        insertNonFull(root, row);
    }
}

void deleteUserDataHelper(Node* node, int key);
int findKey(Node* node, int key);
void removeFromLeaf(Node* node, int idx);
int getPredecessor(Node* node, int idx);
void fill(Node* node, int idx);
void borrowFromPrev(Node* node, int idx);
void borrowFromNext(Node* node, int idx);
void merge(Node* node, int idx);

int findKey(Node* node, int key)
{
    int idx = 0;
    while (idx < node->n && key > node->records[idx].id) {
        idx++;
    }
    return idx;
}

void removeFromLeaf(Node* node, int idx)
{
    for (int i = idx + 1; i < node->n; ++i) {
        node->records[i - 1] = node->records[i];
    }
    node->n--;
}


int getPredecessor(Node* node, int idx) {
    while (!node->leaf) {
        node = node->children[idx + 1];
    }
    return node->records[node->n - 1].id;
}

void fill(Node* node, int idx)
{
    if (idx != 0 && node->children[idx - 1]->n >= node->t) {
        borrowFromPrev(node, idx);
    }
    else if (idx != node->n && node->children[idx + 1]->n >= node->t) {
        borrowFromNext(node, idx);
    }
    else {
        if (idx != node->n) {
            merge(node, idx);
        }
        else {
            merge(node, idx - 1);
        }
    }
}

void borrowFromPrev(Node* node, int idx)
{
    Node* child = node->children[idx];
    Node* sibling = node->children[idx - 1];

    for (int i = child->n - 1; i >= 0; --i) {
        child->records[i + 1] = child->records[i];
    }

    if (!child->leaf) {
        for (int i = child->n; i >= 0; --i) {
            child->children[i + 1] = child->children[i];
        }
    }

    child->records[0] = node->records[idx - 1];

    if (!child->leaf) {
        child->children[0] = sibling->children[sibling->n];
    }

    node->records[idx - 1] = sibling->records[sibling->n - 1];

    child->n += 1;
    sibling->n -= 1;
}

void borrowFromNext(Node* node, int idx)
{
    Node* child = node->children[idx];
    Node* sibling = node->children[idx + 1];

    child->records[(child->n)] = node->records[idx];

    if (!child->leaf) {
        child->children[(child->n) + 1]
            = sibling->children[0];
    }

    node->records[idx] = sibling->records[0];

    for (int i = 1; i < sibling->n; ++i) {
        sibling->records[i - 1] = sibling->records[i];
    }

    if (!sibling->leaf) {
        for (int i = 1; i <= sibling->n; ++i) {
            sibling->children[i - 1] = sibling->children[i];
        }
    }

    child->n += 1;
    sibling->n -= 1;
}

void merge(Node* node, int idx)
{
    Node* child = node->children[idx];
    Node* sibling = node->children[idx + 1];

    child->records[child->n] = node->records[idx];

    if (!child->leaf) {
        child->children[child->n + 1]
            = sibling->children[0];
    }

    for (int i = 0; i < sibling->n; ++i) {
        child->records[i + child->n + 1] = sibling->records[i];
    }

    if (!child->leaf) {
        for (int i = 0; i <= sibling->n; ++i) {
            child->children[i + child->n + 1]
                = sibling->children[i];
        }
    }

    for (int i = idx + 1; i < node->n; ++i) {
        node->records[i - 1] = node->records[i];
    }

    for (int i = idx + 2; i <= node->n; ++i) {
        node->children[i - 1] = node->children[i];
    }

    child->n += sibling->n + 1;
    node->n--;

   
    free(sibling);
}

void deleteUserData(BTree* btree, int key) {
    deleteUserDataHelper(btree->root, key);
}

void deleteUserDataHelper(Node* node, int key) {
    int idx = findKey(node, key);

    if (idx < node->n && node->records[idx].id == key) {
        if (node->leaf) {
            removeFromLeaf(node, idx);
        } else {
            int pred = getPredecessor(node, idx);
            node->records[idx].id = pred;  
            deleteUserDataHelper(node->children[idx], pred);  
        }
    } else {
        if (node->leaf) {
            printf("Key %d not found in the tree.\n", key);
            return;
        }

        bool isLastChild = (idx == node->n);
        if (node->children[idx]->n < node->t) {
            fill(node, idx);
        }

        if (isLastChild && idx > node->n) {
            deleteUserDataHelper(node->children[idx - 1], key);
        } else {
            deleteUserDataHelper(node->children[idx], key);
        }
    }
}



int main() {
    BTree* btree = createBTree(MIN_DEGREE);

    insert(btree, createUser(1, "alice", "password1"));
    insert(btree, createUser(3, "bob", "password2"));
    insert(btree, createUser(7, "charlie", "password3"));
    insert(btree, createUser(10, "david", "password4"));
    insert(btree, createUser(5, "eve", "password5"));

    printf("B+ Tree contents before deletion:\n");
    display(btree->root);

    int idToDelete = 3;
    deleteUserData(btree, idToDelete);
    printf("\nDeleted user with ID: %d\n", idToDelete);

    printf("B+ Tree contents after deletion:\n");
    display(btree->root);

    UserData result;
    if (search(btree->root, idToDelete, &result)) {
        printf("\n\nUser found - ID: %d, Username: %s, Password: %s\n", result.id, result.username, result.password);
    } else {
        printf("\n\nUser with ID %d not found.\n", idToDelete);
    }


    return 0;
}