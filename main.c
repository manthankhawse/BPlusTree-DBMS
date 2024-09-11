#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_KEYS 3
#define MIN_KEYS (MAX_KEYS / 2)
#define ORDER (MAX_KEYS + 1)

typedef struct {
    int id;
    char name[100];
    char username[100];
    char password[100];
    char email[100];
    int age;
} User;

typedef struct BPlusNode {
    int is_leaf;
    int num_keys;
    User* keys[MAX_KEYS];
    struct BPlusNode* children[ORDER];
    struct BPlusNode* parent;
} BPlusNode;

BPlusNode* create_node(int is_leaf);
void insert(BPlusNode** root, User* user);
void traverse(BPlusNode* root);
BPlusNode* search(BPlusNode* root, int id);
void split_child(BPlusNode* parent, int index);
void insert_non_full(BPlusNode* node, User* user);
void print_tree(BPlusNode* root, int level);
void delete_user(BPlusNode** root, int id);
void merge_nodes(BPlusNode* node, int index);
void remove_from_leaf(BPlusNode* node, int index);
void remove_from_internal(BPlusNode* node, int index);
void fill(BPlusNode* node, int index);
void borrow_from_prev(BPlusNode* node, int index);
void borrow_from_next(BPlusNode* node, int index);
User* get_predecessor(BPlusNode* node, int index);
User* get_successor(BPlusNode* node, int index);
void update(BPlusNode* root, int id, char* name, char* username, char* email, int age);
User* create_user(int id, char* name, char* username, char* email, int age);

BPlusNode* create_node(int is_leaf) {
    BPlusNode* node = (BPlusNode*)malloc(sizeof(BPlusNode));
    node->is_leaf = is_leaf;
    node->num_keys = 0;
    node->parent = NULL;
    for (int i = 0; i < ORDER; i++) {
        node->children[i] = NULL;
    }
    return node;
}

void insert(BPlusNode** root, User* user) {
    BPlusNode* r = *root;
    if (r->num_keys == MAX_KEYS) {
        BPlusNode* s = create_node(0);
        *root = s;
        s->children[0] = r;
        r->parent = s;
        split_child(s, 0);
        insert_non_full(s, user);
    } else {
        insert_non_full(r, user);
    }
}

void insert_non_full(BPlusNode* node, User* user) {
    int i = node->num_keys - 1;
    if (node->is_leaf) {
        while (i >= 0 && node->keys[i]->id > user->id) {
            node->keys[i + 1] = node->keys[i];
            i--;
        }
        node->keys[i + 1] = user;
        node->num_keys++;
    } else {
        while (i >= 0 && node->keys[i]->id > user->id) {
            i--;
        }
        i++;
        if (node->children[i]->num_keys == MAX_KEYS) {
            split_child(node, i);
            if (node->keys[i]->id < user->id) {
                i++;
            }
        }
        insert_non_full(node->children[i], user);
    }
}

void split_child(BPlusNode* parent, int index) {
    BPlusNode* full_child = parent->children[index];
    BPlusNode* new_child = create_node(full_child->is_leaf);
    parent->children[index + 1] = new_child;
    new_child->parent = parent;
    parent->num_keys++;
    new_child->num_keys = MIN_KEYS;
    full_child->num_keys = MIN_KEYS;

    for (int i = 0; i < MIN_KEYS; i++) {
        new_child->keys[i] = full_child->keys[i + MIN_KEYS + 1];
    }
    if (!full_child->is_leaf) {
        for (int i = 0; i <= MIN_KEYS; i++) {
            new_child->children[i] = full_child->children[i + MIN_KEYS + 1];
        }
    }

    for (int i = parent->num_keys - 1; i >= index; i--) {
        parent->keys[i + 1] = parent->keys[i];
        parent->children[i + 2] = parent->children[i + 1];
    }
    parent->keys[index] = full_child->keys[MIN_KEYS];
    parent->children[index + 1] = new_child;
}

void traverse(BPlusNode* root) {
    if (root) {
        if (root->is_leaf) {
            for (int i = 0; i < root->num_keys; i++) {
                User* user = root->keys[i];
                printf("ID: %d, Name: %s, Username: %s, Email: %s, Age: %d\n",
                    user->id, user->name, user->username, user->email, user->age);
            }
        } else {
            for (int i = 0; i < root->num_keys; i++) {
                traverse(root->children[i]);
                User* user = root->keys[i];
                printf("ID: %d, Name: %s, Username: %s, Email: %s, Age: %d\n",
                    user->id, user->name, user->username, user->email, user->age);
            }
            traverse(root->children[root->num_keys]);
        }
    }
}

BPlusNode* search(BPlusNode* root, int id) {
    BPlusNode* node = root;
    while (node) {
        int i = 0;
        while (i < node->num_keys && id > node->keys[i]->id) {
            i++;
        }
        if (i < node->num_keys && node->keys[i]->id == id) {
            return node;
        }
        if (node->is_leaf) {
            return NULL;
        } else {
            node = node->children[i];
        }
    }
    return NULL;
}

void delete_user(BPlusNode** root, int id) {
    BPlusNode* node = *root;
    int i = 0;
    while (i < node->num_keys && node->keys[i]->id < id) {
        i++;
    }
    if (i < node->num_keys && node->keys[i]->id == id) {
        if (node->is_leaf) {
            remove_from_leaf(node, i);
        } else {
            remove_from_internal(node, i);
        }
    } else {
        if (node->is_leaf) {
            printf("User with ID %d not found.\n", id);
            return;
        }
        int flag = (i == node->num_keys);
        if (node->children[i]->num_keys < MIN_KEYS) {
            fill(node, i);
        }
        if (flag && i > node->num_keys) {
            delete_user(&(node->children[i - 1]), id);
        } else {
            delete_user(&(node->children[i]), id);
        }
    }
}

void remove_from_leaf(BPlusNode* node, int index) {
    for (int i = index + 1; i < node->num_keys; i++) {
        node->keys[i - 1] = node->keys[i];
    }
    node->num_keys--;
}

void remove_from_internal(BPlusNode* node, int index) {
    User* user = node->keys[index];
    if (node->children[index]->num_keys >= MIN_KEYS) {
        User* predecessor = get_predecessor(node, index);
        node->keys[index] = predecessor;
        delete_user(&(node->children[index]), predecessor->id);
    } else if (node->children[index + 1]->num_keys >= MIN_KEYS) {
        User* successor = get_successor(node, index);
        node->keys[index] = successor;
        delete_user(&(node->children[index + 1]), successor->id);
    } else {
        merge_nodes(node, index);
        delete_user(&(node->children[index]), user->id);
    }
}

void merge_nodes(BPlusNode* node, int index) {
    BPlusNode* child = node->children[index];
    BPlusNode* sibling = node->children[index + 1];
    child->keys[MIN_KEYS] = node->keys[index];
    for (int i = 0; i < sibling->num_keys; i++) {
        child->keys[i + MIN_KEYS + 1] = sibling->keys[i];
    }
    if (!child->is_leaf) {
        for (int i = 0; i <= sibling->num_keys; i++) {
            child->children[i + MIN_KEYS + 1] = sibling->children[i];
        }
    }
    for (int i = index + 1; i < node->num_keys; i++) {
        node->keys[i - 1] = node->keys[i];
        node->children[i] = node->children[i + 1];
    }
    node->num_keys--;
    child->num_keys += sibling->num_keys + 1;
    free(sibling);
}

void fill(BPlusNode* node, int index) {
    if (index != 0 && node->children[index - 1]->num_keys >= MIN_KEYS) {
        borrow_from_prev(node, index);
    } else if (index != node->num_keys && node->children[index + 1]->num_keys >= MIN_KEYS) {
        borrow_from_next(node, index);
    } else {
        if (index != node->num_keys) {
            merge_nodes(node, index);
        } else {
            merge_nodes(node, index - 1);
        }
    }
}

void borrow_from_prev(BPlusNode* node, int index) {
    BPlusNode* child = node->children[index];
    BPlusNode* sibling = node->children[index - 1];
    for (int i = child->num_keys - 1; i >= 0; i--) {
        child->keys[i + 1] = child->keys[i];
    }
    if (!child->is_leaf) {
        for (int i = child->num_keys; i >= 0; i--) {
            child->children[i + 1] = child->children[i];
        }
    }
    child->keys[0] = node->keys[index - 1];
    if (!child->is_leaf) {
        child->children[0] = sibling->children[sibling->num_keys];
    }
    node->keys[index - 1] = sibling->keys[sibling->num_keys - 1];
    child->num_keys++;
    sibling->num_keys--;
}

void borrow_from_next(BPlusNode* node, int index) {
    BPlusNode* child = node->children[index];
    BPlusNode* sibling = node->children[index + 1];
    child->keys[child->num_keys] = node->keys[index];
    if (!child->is_leaf) {
        child->children[child->num_keys + 1] = sibling->children[0];
    }
    node->keys[index] = sibling->keys[0];
    for (int i = 1; i < sibling->num_keys; i++) {
        sibling->keys[i - 1] = sibling->keys[i];
    }
    if (!sibling->is_leaf) {
        for (int i = 1; i <= sibling->num_keys; i++) {
            sibling->children[i - 1] = sibling->children[i];
        }
    }
    child->num_keys++;
    sibling->num_keys--;
}

User* get_predecessor(BPlusNode* node, int index) {
    BPlusNode* cur = node->children[index];
    while (!cur->is_leaf) {
        cur = cur->children[cur->num_keys];
    }
    return cur->keys[cur->num_keys - 1];
}

User* get_successor(BPlusNode* node, int index) {
    BPlusNode* cur = node->children[index + 1];
    while (!cur->is_leaf) {
        cur = cur->children[0];
    }
    return cur->keys[0];
}

void update(BPlusNode* root, int id, char* name, char* username, char* email, int age) {
    BPlusNode* node = search(root, id);
    if (node) {
        for (int i = 0; i < node->num_keys; i++) {
            if (node->keys[i]->id == id) {
                if (name != NULL) strcpy(node->keys[i]->name, name);
                if (username != NULL) strcpy(node->keys[i]->username, username);
                if (email != NULL) strcpy(node->keys[i]->email, email);
                if (age > 0) node->keys[i]->age = age;
                printf("User updated successfully.\n");
                return;
            }
        }
    } else {
        printf("User with ID %d not found.\n", id);
    }
}

void save_node(FILE* file, BPlusNode* node) {
    fwrite(&node->is_leaf, sizeof(int), 1, file);
    fwrite(&node->num_keys, sizeof(int), 1, file);
    for (int i = 0; i < node->num_keys; i++) {
        fwrite(node->keys[i], sizeof(User), 1, file);  // Save user data
    }
    if (!node->is_leaf) {
        for (int i = 0; i <= node->num_keys; i++) {
            save_node(file, node->children[i]);
        }
    }
}

void save_tree(BPlusNode* root, const char* filename) {
    FILE* file = fopen(filename, "wb");
    if (file == NULL) {
        printf("Error opening file for saving.\n");
        return;
    }
    save_node(file, root);
    fclose(file);
    printf("Tree saved to %s\n", filename);
}

BPlusNode* load_node(FILE* file) {
    BPlusNode* node = (BPlusNode*)malloc(sizeof(BPlusNode));
    fread(&node->is_leaf, sizeof(int), 1, file);
    fread(&node->num_keys, sizeof(int), 1, file);
    for (int i = 0; i < node->num_keys; i++) {
        node->keys[i] = (User*)malloc(sizeof(User));
        fread(node->keys[i], sizeof(User), 1, file);
    }
    if (!node->is_leaf) {
        for (int i = 0; i <= node->num_keys; i++) {
            node->children[i] = load_node(file);
        }
    }
    return node;
}

BPlusNode* load_tree(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        printf("Error opening file for loading.\n");
        return NULL;
    }
    BPlusNode* root = load_node(file);
    fclose(file);
    printf("Tree loaded from %s\n", filename);
    return root;
}

void menu() {
    printf("\n--- B+ Tree Operations Menu ---\n");
    printf("1. Insert User\n");
    printf("2. Search User\n");
    printf("3. Delete User\n");
    printf("4. Update User\n");
    printf("5. Traverse Users\n");
    printf("6. Print Tree Structure\n");
    printf("0. Exit\n");
    printf("Choose an option: ");
}

int main() {
    BPlusNode* root = create_node(1);
    int choice, id, age;
    char name[100], username[100], email[100];

    while (1) {
        menu();
        scanf("%d", &choice);
        
        switch (choice) {
            case 1: // Insert User
                printf("Enter ID: ");
                scanf("%d", &id);
                printf("Enter Name: ");
                scanf("%s", name);
                printf("Enter Username: ");
                scanf("%s", username);
                printf("Enter Email: ");
                scanf("%s", email);
                printf("Enter Age: ");
                scanf("%d", &age);
                
                User* user = create_user(id, name, username, email, age);
                insert(&root, user);
                printf("User inserted successfully!\n");
                break;

            case 2: // Search User
                printf("Enter ID to search: ");
                scanf("%d", &id);
                BPlusNode* found = search(root, id);
                if (found) {
                    printf("User with ID %d found.\n", id);
                } else {
                    printf("User with ID %d not found.\n", id);
                }
                break;

            case 3: // Delete User
                printf("Enter ID to delete: ");
                scanf("%d", &id);
                delete_user(&root, id);
                break;

            case 4: // Update User
                printf("Enter ID to update: ");
                scanf("%d", &id);
                printf("Enter new name (or '-' to skip): ");
                scanf("%s", name);
                printf("Enter new username (or '-' to skip): ");
                scanf("%s", username);
                printf("Enter new email (or '-' to skip): ");
                scanf("%s", email);
                printf("Enter new age (or 0 to skip): ");
                scanf("%d", &age);
                
                update(root, id, strcmp(name, "-") ? name : NULL, strcmp(username, "-") ? username : NULL, strcmp(email, "-") ? email : NULL, age);
                break;

            case 5: // Traverse Users
                traverse(root);
                break;

            // case 6: // Print Tree Structure
            //     print_tree(root, 0);
            //     break;

            case 0: // Exit
                printf("Exiting...\n");
                exit(0);

            default:
                printf("Invalid choice. Please try again.\n");
        }
    }
}

User* create_user(int id, char* name, char* username, char* email, int age) {
    User* user = (User*)malloc(sizeof(User));
    user->id = id;
    strcpy(user->name, name);
    strcpy(user->username, username);
    strcpy(user->email, email);
    user->age = age;
    return user;
}
