#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
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


typedef enum{
    INSERT,
    FIND,
    SELECT,
    UPDATE,
    DELETE
}statement_type;

typedef enum{
    PARSE_SUCCESSFUL,
    NEGATIVE_ID_ERROR,
    INVALID_INPUT,
    UNRECOGNIZED,
}parse_res;

typedef enum{
    EXIT,
    SAVE,
    HELP,
    SCHEMA,
    OPENDB,
    UNRECOGNIZED_META,
}meta_command_type;

typedef struct{
    char type[100];
    int id;
    char username[255];
    char password[255];
}tokens;

typedef struct{
    statement_type type;
    UserData row_to_insert;
}statement;

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

void print_prompt(){
    printf("table > ");
}

tokens* tokenize(char* input){
    char* type = strtok(input, " ");
    char* id_str = strtok(NULL, " ");
    char* username = strtok(NULL, " ");
    char* password = strtok(NULL, " ");

    tokens* token = (tokens*)malloc(sizeof(tokens));
    if (type != NULL) strcpy(token->type, type);
    token->id = id_str != NULL ? atoi(id_str) : -1;
    username != NULL ? strcpy(token->username, username) : strcpy(token->username, "");
    password != NULL ? strcpy(token->password, password):  strcpy(token->password, "") ;

    return token;
}

void print_tokens(tokens* token){
    printf("tokens\n%s %d %s %s\n", token->type, token->id, token->username, token->password);
}

parse_res parse(tokens* token, statement** s){
    char* type = token->type;
    if(strcmp(type, "insert")==0){
        (*s)->type = INSERT;
        if(token->id < 0){
            // error
            return NEGATIVE_ID_ERROR;
        }
        if(strlen(token->username)==0){
            // error
            return INVALID_INPUT;
        }
        if(strlen(token->password)==0){
            // error
            return INVALID_INPUT;
        }
    }else if(strcmp(type, "select")==0){
        (*s)->type = SELECT;
        if(token->id!=-1){
            // error
            return INVALID_INPUT;
        }
        if(strlen(token->username)>0){
            // error
            return INVALID_INPUT;
        }
        if(strlen(token->password)>0){
            // error
            return INVALID_INPUT;
        }
    }else if(strcmp(type, "update")==0){
        (*s)->type = UPDATE;
        if(token->id<0){
            // error
            return NEGATIVE_ID_ERROR;
        }
        if(strlen(token->username)==0){
            // error
            return INVALID_INPUT;
        }
        if(strlen(token->password)==0){
            // error
            return INVALID_INPUT;
        }
    }else if(strcmp(type, "delete")==0){
        (*s)->type = DELETE;
        if(token->id<0){
            // error
            return NEGATIVE_ID_ERROR;
        }
        if(strlen(token->username)!=0){
            // error
            return INVALID_INPUT;
        }
        if(strlen(token->password)!=0){
            // error
            return INVALID_INPUT;
        }
    }else if(strcmp(type, "find")==0){
        (*s)->type = FIND;
        if(token->id<0){
            // error
            return NEGATIVE_ID_ERROR;
        }
        if(strlen(token->username)!=0){
            // error
            return INVALID_INPUT;
        }
        if(strlen(token->password)!=0){
            // error
            return INVALID_INPUT;
        }
    }else{
        return UNRECOGNIZED;
    }

    (*s)->row_to_insert.id = token->id;
    strcpy((*s)->row_to_insert.username, token->username);
    strcpy((*s)->row_to_insert.password, token->password);

    return PARSE_SUCCESSFUL;
}

void execute(statement* s, BTree** btree){
    UserData result;
    bool userFound;
    switch(s->type){
        case INSERT:
            insert(*btree, &(s->row_to_insert));
            break;
        case FIND:
            userFound = search((*btree)->root, s->row_to_insert.id, &result);
            printf("%d %s %s", result.id, result.username, result.password);
            break;
        case UPDATE:
            userFound = update((*btree)->root, s->row_to_insert.id, &result, s->row_to_insert.username, s->row_to_insert.password);
            printf("%d %s %s", result.id, result.username, result.password);
            break;
        case SELECT:
            display((*btree)->root);
            break;
        case DELETE:
            deleteUserData(*btree, s->row_to_insert.id);
            break;
        default:
            printf("Something went wrong, query could not be processed\n");
            return;
            break;
    }
    printf("Executed.\n");
    return; 
}

meta_command_type findMetaType(char* input){
    if(strcmp(input, ".exit")==0){
        return EXIT;
    }else if(strcmp(input, ".help")==0){
        return HELP;
    }else if(strcmp(input, ".save")==0){
        return SAVE;
    }else if(strcmp(input, ".schema")==0){
        return SCHEMA;
    }else if(strcmp(input, ".opendb")==0){
        return OPENDB;
    }

    return UNRECOGNIZED_META;
}

void print_help() {
    printf("-------- SQL REPL HELP --------\n");
    printf("Below are the available commands for interacting with the database:\n\n");

    // SQL Queries
    printf("1. insert <id> <username> <password>\n");
    printf("   - Inserts a new row into the table with the specified ID, username, and password.\n");
    printf("   Example: insert 1 alice secret123\n\n");

    printf("2. update <id> <newusername> <newpassword>\n");
    printf("   - Updates the username and password for the given ID.\n");
    printf("   Example: update 1 bob newpass\n\n");

    printf("3. find <id>\n");
    printf("   - Finds and displays the row corresponding to the specified ID.\n");
    printf("   Example: find 1\n\n");

    printf("4. select\n");
    printf("   - Selects and displays all rows in the current table.\n");
    printf("   Example: select\n\n");

    printf("5. delete <id>\n");
    printf("   - Deletes the row with the specified ID from the table.\n");
    printf("   Example: delete 1\n\n");

    // Meta Commands
    printf("6. .help\n");
    printf("   - Displays this help message with detailed information about all queries and commands.\n\n");

    printf("7. .exit\n");
    printf("   - Exits the SQL REPL (Read-Eval-Print-Loop).\n\n");

    printf("8. .save <filename>\n");
    printf("   - Saves the current table to the specified file.\n");
    printf("   Example: .save mydb.txt\n\n");

    printf("9. .opendb <filename>\n");
    printf("   - Opens and loads the table from the specified file.\n");
    printf("   Example: .opendb mydb.txt\n\n");

    printf("10. .schema\n");
    printf("    - Prints the schema of the current table.\n");
    printf("    Example: .schema\n\n");

    printf("--------------------------------\n");
}


void print_schema() {
    printf("-------- DATABASE SCHEMA --------\n");
    printf("Table: users\n\n");

    printf("| %-10s | %-20s | %-20s |\n", "id", "username", "password");
    printf("----------------------------------------------\n");
    printf("| %-10s | %-20s | %-20s |\n", "INTEGER", "VARCHAR(255)", "VARCHAR(255)");

    printf("\nSchema of the table is as follows:\n");
    printf("id: INTEGER (Primary Key)\n");
    printf("username: VARCHAR(255)\n");
    printf("password: VARCHAR(255)\n");
    printf("--------------------------------\n");
}


int main() {
    char input[100];
    BTree* btree = createBTree(MIN_DEGREE);
    while (true) {
        print_prompt();
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = 0;  

        if(input[0]=='.'){
            meta_command_type meta_type = findMetaType(input);
            switch(meta_type){
                case HELP:
                    print_help();
                    break;
                case SCHEMA:
                    print_schema();
                    break;
                case EXIT:
                    exit(EXIT_SUCCESS);
                    break;
                case OPENDB:
                    printf("Todo\n");
                    break;
                case SAVE:
                    printf("Todo\n");
                    break;
                case UNRECOGNIZED_META:
                    printf("Unrecognized Meta command\n");
                    break;
            }
            continue;
        }

        tokens* tokenizedStatement = tokenize(input);
        statement* s;
        s = (statement*)malloc(sizeof(statement));

        switch(parse(tokenizedStatement, &s)){
            case PARSE_SUCCESSFUL:
                execute(s, &btree);
                break;
            case INVALID_INPUT:
                printf("Input is invalid\n");
                break;
            case NEGATIVE_ID_ERROR:
                printf("Input ID cannot be negative\n");
                break;
            case UNRECOGNIZED:
                printf("Unrecognized query\n");
                break;
        }

        free(tokenizedStatement);
        free(s);
    }
  
    return 0;
}
