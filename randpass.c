#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "hudatabase.h"

huRoot huTheRoot;

// Create a new node.
static inline huNode huLeafNodeCreate(
    utSym name,
    uint64 count)
{
    huNode node = huNodeAlloc();

    huNodeSetCount(node, count);
    huNodeSetNegCount(node, -count);
    huNodeSetSym(node, name);
    huRootPushHeapNode(huTheRoot, node);
    huRootAppendNode(huTheRoot, node);
    return node;
}

// Create a new node.
static inline huNode huInternalNodeCreate(
    huNode leftNode,
    huNode rightNode)
{
    huNode node = huNodeAlloc();
    uint64 count = huNodeGetCount(leftNode) + huNodeGetCount(rightNode);

    huNodeSetCount(node, count);
    huNodeSetNegCount(node, -count);
    huNodeInsertLeftNode(node, leftNode);
    huNodeInsertRightNode(node, rightNode);
    huRootPushHeapNode(huTheRoot, node);
    return node;
}

// Huffman enchode words in words.txt.
static void encodeWords(void)
{
    FILE *file = fopen("words.txt", "r");
    char line[100], word[100];
    uint64 count;
    huNode node, leftNode, rightNode;

    while(fgets(line, 100, file)) {
        sscanf(line, "%s %llu", word, &count);
        huLeafNodeCreate(utSymCreate(word), count);
    }
    while(huRootGetUsedHeapNode(huTheRoot) != 1) {
        leftNode = huRootPopHeapNode(huTheRoot);
        rightNode = huRootPopHeapNode(huTheRoot);
        node = huInternalNodeCreate(leftNode, rightNode);
    }
    node = huRootPopHeapNode(huTheRoot);
    huRootSetTopNode(huTheRoot, node);
}

// Compute node paths.
static void setNodePaths(
    huNode node,
    uint32 path,
    uint32 length)
{
    huNode left, right;
    uint32 leftPath, rightPath;

    huNodeSetPath(node, path);
    huNodeSetPathLength(node, length);
    length += 1;
    leftPath = path << 1;
    rightPath = leftPath | 1;
    left = huNodeGetLeftNode(node);
    right = huNodeGetRightNode(node);
    if(left != huNodeNull) {
        setNodePaths(left, leftPath, length);
    }
    if(right != huNodeNull) {
        setNodePaths(right, rightPath, length);
    }
}

// Print a node.
static void printNode(
    huNode node)
{
    uint32 path = huNodeGetPath(node);
    uint32 length = huNodeGetPathLength(node);

    printf("%s ", huNodeGetName(node));
    while(length--) {
        if((1 << length) & path) {
            putchar('1');
        } else {
            putchar('0');
        }
    }
    putchar('\n');
}

// Print all nodes.
static void printNodes(void)
{
    huNode node;

    huForeachRootNode(huTheRoot, node) {
        printNode(node);
    } huEndRootNode;
}

static uint64 huCounter;
volatile static bool huStop;

// Run a free-running counter until the huStop flag goes high.
static void *countUntilStopped(
    void *data)
{
    while(!huStop) {
        huCounter++;
    }
    fflush(stdout);
    return NULL;
}

// Use the enter key to generate random bits.
static bool randBit(void)
{
    pthread_t thread;
    static uint32 numBits = 0;
    static uint64 bits = 0;
    bool value;

    while(numBits == 0) {
        huStop = false;
        pthread_create(&thread, NULL, countUntilStopped, NULL);
        getchar();
        huStop = true;
        pthread_join(thread, NULL);
        while(huCounter > (1 << 20)) {
            bits <<= 1;
            if(huCounter & 1) {
                bits |= 1;
                putchar('1');
                fflush(stdout);
            } else {
                putchar('0');
                fflush(stdout);
            }
            huCounter >>= 1;
            numBits++;
        }
    }
    numBits--;
    value = bits & 1;
    bits >>= 1;
    return value;
}

// Print a random pass phrase of the number of bits
static void printRandPasswd(
    uint32 bits)
{
    huNode node = huRootGetTopNode(huTheRoot);
    huNode leftNode, rightNode;
    uint32 numBits = 0;

    printf("Please press the enter repeatedly key to generate random bits\n");
    while(numBits < bits) {
        leftNode = huNodeGetLeftNode(node);
        rightNode = huNodeGetRightNode(node);
        if(leftNode == huNodeNull) {
            printf(" %s\n", huNodeGetName(node));
            numBits += huNodeGetPathLength(node);
            node = huRootGetTopNode(huTheRoot);
        } else {
            node = randBit()? rightNode : leftNode;
        }
    }
    printf("Total bits: %u\n", numBits);
}

int main(int argc, char **argv)
{
    uint32 bits;

    utStart();
    huDatabaseStart();
    huTheRoot = huRootAlloc();
    encodeWords();
    setNodePaths(huRootGetTopNode(huTheRoot), 0, 0);
    if(argc == 1) {
        printNodes();
    } else {
        bits = atoi(argv[1]);
        printRandPasswd(bits);
    }
    huDatabaseStop();
    utStop(false);
    return 0;
}
