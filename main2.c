#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_KEYS 3
#define MIN_KEYS MAX_KEYS / 2

typedef struct BTreeNode {
    int keys[MAX_KEYS];
    struct BTreeNode* children[MAX_KEYS + 1];
    int num_keys;
    bool is_leaf;
} BTreeNode;

BTreeNode* create_node(bool is_leaf) {
    BTreeNode* node = (BTreeNode*)malloc(sizeof(BTreeNode));
    node->is_leaf = is_leaf;
    node->num_keys = 0;
    for (int i = 0; i < MAX_KEYS + 1; i++) {
        node->children[i] = NULL;
    }
    return node;
}

void split_child(BTreeNode* parent, int index, BTreeNode* child) {
    BTreeNode* new_node = create_node(child->is_leaf);
    new_node->num_keys = MIN_KEYS - 1;

    for (int i = 0; i < MIN_KEYS - 1; i++) {
        new_node->keys[i] = child->keys[i + MIN_KEYS];
    }

    if (!child->is_leaf) {
        for (int i = 0; i < MIN_KEYS; i++) {
            new_node->children[i] = child->children[i + MIN_KEYS];
        }
    }

    child->num_keys = MIN_KEYS - 1;

    for (int i = parent->num_keys; i > index; i--) {
        parent->children[i + 1] = parent->children[i];
    }

    parent->children[index + 1] = new_node;

    for (int i = parent->num_keys - 1; i >= index; i--) {
        parent->keys[i + 1] = parent->keys[i];
    }

    parent->keys[index] = child->keys[MIN_KEYS - 1];
    parent->num_keys++;
}

void insert_non_full(BTreeNode* node, int key) {
    int i = node->num_keys - 1;

    if (node->is_leaf) {
        while (i >= 0 && key < node->keys[i]) {
            node->keys[i + 1] = node->keys[i];
            i--;
        }
        node->keys[i + 1] = key;
        node->num_keys++;
    } else {
        while (i >= 0 && key < node->keys[i]) {
            i--;
        }
        i++;
        if (node->children[i]->num_keys == MAX_KEYS) {
            split_child(node, i, node->children[i]);
            if (key > node->keys[i]) {
                i++;
            }
        }
        insert_non_full(node->children[i], key);
    }
}

void insert(BTreeNode** root, int key) {
    if ((*root)->num_keys == MAX_KEYS) {
        BTreeNode* new_root = create_node(false);
        new_root->children[0] = *root;
        split_child(new_root, 0, *root);
        *root = new_root;
    }
    insert_non_full(*root, key);
}

bool search(BTreeNode* node, int key) {
    int i = 0;
    while (i < node->num_keys && key > node->keys[i]) {
        i++;
    }
    if (i < node->num_keys && key == node->keys[i]) {
        return true;
    }
    if (node->is_leaf) {
        return false;
    }
    return search(node->children[i], key);
}

int find_key(BTreeNode* node, int key) {
    int idx = 0;
    while (idx < node->num_keys && node->keys[idx] < key) {
        ++idx;
    }
    return idx;
}

void remove_from_leaf(BTreeNode* node, int idx) {
    for (int i = idx + 1; i < node->num_keys; ++i) {
        node->keys[i - 1] = node->keys[i];
    }
    node->num_keys--;
}

void remove_from_nonleaf(BTreeNode* node, int idx) {
    int k = node->keys[idx];
    if (node->children[idx]->num_keys >= MIN_KEYS) {
        int pred = get_pred(node, idx);
        node->keys[idx] = pred;
        delete(node->children[idx], pred);
    }
    else if (node->children[idx + 1]->num_keys >= MIN_KEYS) {
        int succ = get_succ(node, idx);
        node->keys[idx] = succ;
        delete(node->children[idx + 1], succ);
    }
    else {
        merge(node, idx);
        delete(node->children[idx], k);
    }
}

int get_pred(BTreeNode* node, int idx) {
    BTreeNode* curr = node->children[idx];
    while (!curr->is_leaf) {
        curr = curr->children[curr->num_keys];
    }
    return curr->keys[curr->num_keys - 1];
}

int get_succ(BTreeNode* node, int idx) {
    BTreeNode* curr = node->children[idx + 1];
    while (!curr->is_leaf) {
        curr = curr->children[0];
    }
    return curr->keys[0];
}

void merge(BTreeNode* node, int idx) {
    BTreeNode* child = node->children[idx];
    BTreeNode* sibling = node->children[idx + 1];

    child->keys[MIN_KEYS - 1] = node->keys[idx];

    for (int i = 0; i < sibling->num_keys; ++i) {
        child->keys[i + MIN_KEYS] = sibling->keys[i];
    }

    if (!child->is_leaf) {
        for (int i = 0; i <= sibling->num_keys; ++i) {
            child->children[i + MIN_KEYS] = sibling->children[i];
        }
    }

    for (int i = idx + 1; i < node->num_keys; ++i) {
        node->keys[i - 1] = node->keys[i];
    }

    for (int i = idx + 2; i <= node->num_keys; ++i) {
        node->children[i - 1] = node->children[i];
    }

    child->num_keys += sibling->num_keys + 1;
    node->num_keys--;

    free(sibling);
}

void delete(BTreeNode* node, int key) {
    int idx = find_key(node, key);

    if (idx < node->num_keys && node->keys[idx] == key) {
        if (node->is_leaf) {
            remove_from_leaf(node, idx);
        } else {
            remove_from_nonleaf(node, idx);
        }
    } else {
        if (node->is_leaf) {
            printf("The key %d does not exist in the tree\n", key);
            return;
        }

        bool flag = (idx == node->num_keys);

        if (node->children[idx]->num_keys < MIN_KEYS) {
            fill(node, idx);
        }

        if (flag && idx > node->num_keys) {
            delete(node->children[idx - 1], key);
        } else {
            delete(node->children[idx], key);
        }
    }
}

void fill(BTreeNode* node, int idx) {
    if (idx != 0 && node->children[idx - 1]->num_keys >= MIN_KEYS) {
        borrow_from_prev(node, idx);
    } else if (idx != node->num_keys && node->children[idx + 1]->num_keys >= MIN_KEYS) {
        borrow_from_next(node, idx);
    } else {
        if (idx != node->num_keys) {
            merge(node, idx);
        } else {
            merge(node, idx - 1);
        }
    }
}

void borrow_from_prev(BTreeNode* node, int idx) {
    BTreeNode* child = node->children[idx];
    BTreeNode* sibling = node->children[idx - 1];

    for (int i = child->num_keys - 1; i >= 0; --i) {
        child->keys[i + 1] = child->keys[i];
    }

    if (!child->is_leaf) {
        for (int i = child->num_keys; i >= 0; --i) {
            child->children[i + 1] = child->children[i];
        }
    }

    child->keys[0] = node->keys[idx - 1];

    if (!node->is_leaf) {
        child->children[0] = sibling->children[sibling->num_keys];
    }

    node->keys[idx - 1] = sibling->keys[sibling->num_keys - 1];

    child->num_keys++;
    sibling->num_keys--;
}

void borrow_from_next(BTreeNode* node, int idx) {
    BTreeNode* child = node->children[idx];
    BTreeNode* sibling = node->children[idx + 1];

    child->keys[child->num_keys] = node->keys[idx];

    if (!child->is_leaf) {
        child->children[child->num_keys + 1] = sibling->children[0];
    }

    node->keys[idx] = sibling->keys[0];

    for (int i = 1; i < sibling->num_keys; ++i) {
        sibling->keys[i - 1] = sibling->keys[i];
    }

    if (!sibling->is_leaf) {
        for (int i = 1; i <= sibling->num_keys; ++i) {
            sibling->children[i - 1] = sibling->children[i];
        }
    }

    child->num_keys++;
    sibling->num_keys--;
}

void print_tree(BTreeNode* root, int level) {
    if (root != NULL) {
        printf("Level %d: ", level);
        for (int i = 0; i < root->num_keys; i++) {
            printf("%d ", root->keys[i]);
        }
        printf("\n");
        if (!root->is_leaf) {
            for (int i = 0; i <= root->num_keys; i++) {
                print_tree(root->children[i], level + 1);
            }
        }
    }
}

int main() {
    BTreeNode* root = create_node(true);

    insert(&root, 10);
    insert(&root, 20);
    insert(&root, 5);
    insert(&root, 6);
    insert(&root, 12);
    insert(&root, 30);
    insert(&root, 7);
    insert(&root, 17);

    printf("B-tree after insertions:\n");
    print_tree(root, 0);

    printf("\nSearching for 6: %s\n", search(root, 6) ? "Found" : "Not Found");
    printf("Searching for 15: %s\n", search(root, 15) ? "Found" : "Not Found");

    delete(root, 6);
    printf("\nB-tree after deleting 6:\n");
    print_tree(root, 0);

    delete(root, 30);
    printf("\nB-tree after deleting 30:\n");
    print_tree(root, 0);

    return 0;
}