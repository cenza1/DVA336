#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <cassert>

struct bstnode {
    bstnode(int key) : key(key) { left = right = nullptr; }
    ~bstnode() {
        delete left;
        delete right;
    }
    int key;
    bstnode *left, *right;
};

inline int max(int a, int b) { return a > b ? a : b; }

// returns the height of the tree
int height(bstnode *root) {
    if (!root)
        return 0;
    int lefth, righth;
    lefth = height(root->left);
    righth = height(root->right);
    return max(lefth, righth) + 1;
}

// returns the height of the tree
int efficient_height(bstnode *root) {
    if (!root)
        return 0;
    int lefth, righth;
#pragma omp task shared(lefth)
    lefth = efficient_height(root->left);
#pragma omp task shared(righth)
    righth = efficient_height(root->right);
#pragma omp taskwait
    // printf("\n Height returned: %d", max(lefth, righth) + 1);
    return max(lefth, righth) + 1;
}

// returns true if the tree is height-balanced; false otherwise
bool isbalanced(bstnode *root) {
    if (!root)
        return true;
    int lefth, righth;
    lefth = height(root->left);
    righth = height(root->right);
    return abs(lefth - righth) < 2 && isbalanced(root->left) && isbalanced(root->right);
}

// returns true if the tree is height-balanced; false otherwise
bool efficient_isbalanced(bstnode *root) {
    if (!root)
        return true;
    int lefth, righth;
#pragma omp task shared(lefth)
    lefth = efficient_height(root->left);
#pragma omp task shared(righth)
    righth = efficient_height(root->right);
#pragma omp taskwait
    // printf("Left: %d, Right: %d\n", lefth, righth);
    return abs(lefth - righth) < 2 && efficient_isbalanced(root->left) && efficient_isbalanced(root->right);
}

#define NDF -1

int lcg(int seed = NDF) {
    constexpr int m = 134456;
    constexpr int a = 8121;
    constexpr int c = 28411;
    static int x = 0;
    return x = seed == NDF ? (a * x + c) % m : seed;
}

// return a pointer to a random BST of which keys are in [lower,upper]
bstnode *buildbalanced(int lower, int upper, const int threshold, int &node_counter) {
    if (upper < lower)
        return nullptr;
    int key = (upper + lower) / 2;
    bstnode *root = new bstnode(key);
    node_counter = node_counter + 1;
    if (lcg() % 100 < threshold)
        root->left = buildbalanced(lower, key - 1, threshold, node_counter);
    if (lcg() % 100 < threshold)
        root->right = buildbalanced(key + 1, upper, threshold, node_counter);
    return root;
}

void compare_seq_and_parallel(bstnode *root, int nnodes) {
        double ts = omp_get_wtime();
        bool seqIsBalanced = isbalanced(root);
        ts = omp_get_wtime() - ts;
        printf("T#0, %d nodes, isbalanced = %c\n", nnodes, seqIsBalanced ? 'Y' : 'N');

        double tp = omp_get_wtime();
        bool parIsBalanced = efficient_isbalanced(root);
        tp = omp_get_wtime() - tp;
        printf("T#0, %d nodes, effecient_isbalanced = %c\n", nnodes, parIsBalanced ? 'Y' : 'N');

        // Make sure that parallel version is working as expected
        assert(parIsBalanced == seqIsBalanced);
        printf("Speedup from parallel: %.6fx\n", ts / tp);
} 

int main(int argc, char *argv[]) {
#pragma omp parallel
#pragma omp single
    {
        const int height = atoi(argv[1]);
        const int threshold = atoi(argv[2]);
        int nnodes = 0;
        lcg(264433);
        bstnode *root = buildbalanced(1, (1 << height) - 1, threshold, nnodes);
        compare_seq_and_parallel(root, nnodes);
        delete root;
    }
#pragma omp parallel
#pragma omp single
    {
        int nnodes = 0;
        lcg(264433);
        bstnode *root = buildbalanced(1, 511, 90, nnodes);
        compare_seq_and_parallel(root, nnodes);
        delete root;
    }
#pragma omp parallel
#pragma omp single
    {
        int nnodes = 0;
        lcg(123);
        bstnode *root = buildbalanced(1, 32767, 90, nnodes);
        compare_seq_and_parallel(root, nnodes);
        delete root;
    }

    return EXIT_SUCCESS;
}