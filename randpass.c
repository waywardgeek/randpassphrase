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

volatile static uint64 huCounter;
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
        huCounter = 0;
        pthread_create(&thread, NULL, countUntilStopped, NULL);
        do {
            getchar();
        } while(huCounter <= (1 << 20));
        huStop = true;
        pthread_join(thread, NULL);
        while(huCounter > (1 << 20)) {
            bits <<= 1;
            if(huCounter & 1) {
                bits |= 1;
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

// Find 2^selectBits nodes.
static void findRandNodes(
    huNode *nodes,
    uint8 selectBits)
{
    huNode node;
    uint32 numNodes = 1 << selectBits;
    uint32 xNode, xBit;
    bool bit, done;
 
    for(xNode = 0; xNode < numNodes; xNode++) {
        node = huRootGetTopNode(huTheRoot);
        for(xBit = 0; xBit < selectBits; xBit++) {
            if(xNode & (1 << xBit)) {
                node = huNodeGetRightNode(node);
            } else {
                node = huNodeGetLeftNode(node);
            }
        }
        nodes[xNode] = node;
    }
    do {
        done = true;
        bit = randBit();
        for(xNode = 0; xNode < numNodes; xNode++) {
            node = nodes[xNode];
            if(huNodeGetLeftNode(node) != huNodeNull) {
                node = bit? huNodeGetLeftNode(node) : huNodeGetRightNode(node);
                nodes[xNode] = node;
                done = false;
            }
        }
    } while(!done);
}

// Print the words and ask the user to select one.
static huNode selectNode(
    huNode *nodes,
    uint32 numBits,
    uint8 selectBits)
{
    uint32 numNodes = 1 << selectBits;
    huNode node;
    uint32 xNode, bits;
    char c;

    for(xNode = 0; xNode < numNodes; xNode++) {
        node = nodes[xNode];
        bits = huNodeGetPathLength(node) - selectBits;
        printf("%u) %s - %u bits (%0.2f/letter)\n", xNode + 1, huNodeGetName(node),
            bits, (float)bits/(strlen(huNodeGetName(node)) + 1));
    }
    printf("%u bits: Please select the word to be in the passphrase by typing 1-%u.\n",
        numBits, numNodes);
    do {
        c = getchar();
    } while(c < '1' || c > '0' + numNodes);
    return nodes[c - '1'];
}

// Select passphrase words until we've met our strength goal.  If there are 4 choices, we have
// lose 2 bits per selection.
static void selectPassPhrase(
    uint32 bits,
    uint8 selectBits)
{
    huNode node, randNodes[100], nodes[100];
    uint32 xNode = 0;
    uint32 numNodes;
    uint32 numBits = 0;

    do {
        findRandNodes(randNodes, selectBits);
        node = selectNode(randNodes, numBits, selectBits);
        nodes[xNode++] = node;
        numBits += huNodeGetPathLength(node) - selectBits;
    } while(numBits < bits);
    numNodes = xNode;
    for(xNode = 0; xNode < numNodes; xNode++) {
        if(xNode != 0) {
            putchar(' ');
        }
        printf("%s", huNodeGetName(nodes[xNode]));
    }
    printf("\nTotal bits: %u\n", numBits);
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
        printf("Please press the enter key repeatedly to generate random bits\n");
        selectPassPhrase(bits, 1);
    }
    huDatabaseStop();
    utStop(false);
    return 0;
}
