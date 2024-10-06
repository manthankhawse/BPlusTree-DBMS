#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
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

bool update(Node* node, int id, UserData* result, char* username, char* password)
{
    int i = 0;
    while (i < node->n && id > node->records[i].id) {
        i++;
    }
    if (i < node->n && id == node->records[i].id) {
        strcpy(node->records[i].username, username);
        strcpy(node->records[i].password, password);
        if (result != NULL) *result = node->records[i];
        return true;
    }
    if (node->leaf) {
        return false;
    }
    return update(node->children[i], id, result, username, password);
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


// Function to write the tree data to a file (recursive)
void writeTreeToFile(Node* node, FILE* file) {
    if (node == NULL) return;

    // Write number of keys and whether the node is a leaf
    fwrite(&node->n, sizeof(int), 1, file);
    fwrite(&node->leaf, sizeof(bool), 1, file);

    // Write the user data records
    for (int i = 0; i < node->n; i++) {
        fwrite(&node->records[i], sizeof(UserData), 1, file);
    }

    // Write child nodes recursively
    if (!node->leaf) {
        for (int i = 0; i <= node->n; i++) {
            writeTreeToFile(node->children[i], file);
        }
    }
}

// Function to read the tree data from a file (recursive)
Node* readTreeFromFile(FILE* file, int t) {
    int n;
    bool leaf;
    
    // Read number of keys and whether it's a leaf node
    if (fread(&n, sizeof(int), 1, file) != 1) return NULL;
    if (fread(&leaf, sizeof(bool), 1, file) != 1) return NULL;

    Node* node = createNode(t, leaf);
    node->n = n;
    
    // Read user data records
    for (int i = 0; i < n; i++) {
        fread(&node->records[i], sizeof(UserData), 1, file);
    }

    // Read child nodes recursively
    if (!leaf) {
        for (int i = 0; i <= n; i++) {
            node->children[i] = readTreeFromFile(file, t);
        }
    }

    return node;
}

// Save the BTree to file when closing
void saveBTreeToFile(BTree* btree, const char* filename) {
    FILE* file = fopen(filename, "wb");
    if (file != NULL) {
        writeTreeToFile(btree->root, file);
        fclose(file);
    } else {
        printf("Error opening file for writing.\n");
    }
}

// Load the BTree from file at the start
BTree* loadBTreeFromFile(const char* filename, int t) {
    FILE* file = fopen(filename, "rb");
    if (file != NULL) {
        BTree* btree = createBTree(t);
        btree->root = readTreeFromFile(file, t);
        fclose(file);
        return btree;
    } else {
        printf("Error opening file for reading or file not found. Creating a new tree.\n");
        return createBTree(t);
    }
}


int main() {
    const char* filename = "btree_data.dat";
    
    // Load the tree from the file (or create a new one if the file doesn't exist)
    BTree* btree = loadBTreeFromFile(filename, MIN_DEGREE);
    printf("Tree Loaded from file\n");
    if(btree==NULL){
        btree = (BTree*)malloc(sizeof(BTree));
    }else{
        display(btree->root);
    }

    char* newUsername = "newUsername";
    char* newPassword = "newPassword";
    UserData result;
    bool userFound = search(btree->root, 51, &result);
    printf("%d %s %s", result.id, result.username, result.password);

    printf("B+ Tree contents:\n");
    display(btree->root);

    saveBTreeToFile(btree, filename);
    
    return 0;
}