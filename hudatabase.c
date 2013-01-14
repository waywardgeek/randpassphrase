/*----------------------------------------------------------------------------------------
  Database hu
----------------------------------------------------------------------------------------*/

#include "hudatabase.h"

struct huRootType_ huRootData;
uint8 huModuleID;
struct huRootFields huRoots;
struct huNodeFields huNodes;

/*----------------------------------------------------------------------------------------
  Constructor/Destructor hooks.
----------------------------------------------------------------------------------------*/
huRootCallbackType huRootConstructorCallback;
huRootCallbackType huRootDestructorCallback;
huNodeCallbackType huNodeConstructorCallback;
huNodeCallbackType huNodeDestructorCallback;

/*----------------------------------------------------------------------------------------
  Destroy Root including everything in it. Remove from parents.
----------------------------------------------------------------------------------------*/
void huRootDestroy(
    huRoot Root)
{
    huNode HeapNode_;
    uint32 xHeapNode;

    if(huRootDestructorCallback != NULL) {
        huRootDestructorCallback(Root);
    }
    for(xHeapNode = 0; xHeapNode < huRootGetUsedHeapNode(Root); xHeapNode++) {
        HeapNode_ = huRootGetiHeapNode(Root, xHeapNode);
        if(HeapNode_ != huNodeNull) {
            huNodeSetRoot(HeapNode_, huRootNull);
        }
    }
    huRootFree(Root);
}

/*----------------------------------------------------------------------------------------
  Default constructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static uint64 allocRoot(void)
{
    huRoot Root = huRootAlloc();

    return huRoot2Index(Root);
}

/*----------------------------------------------------------------------------------------
  Destructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static void destroyRoot(
    uint64 objectIndex)
{
    huRootDestroy(huIndex2Root((uint32)objectIndex));
}

/*----------------------------------------------------------------------------------------
  Allocate the field arrays of Root.
----------------------------------------------------------------------------------------*/
static void allocRoots(void)
{
    huSetAllocatedRoot(2);
    huSetUsedRoot(1);
    huSetFirstFreeRoot(huRootNull);
    huRoots.HeapNodeIndex_ = utNewAInitFirst(uint32, (huAllocatedRoot()));
    huRoots.NumHeapNode = utNewAInitFirst(uint32, (huAllocatedRoot()));
    huSetUsedRootHeapNode(0);
    huSetAllocatedRootHeapNode(2);
    huSetFreeRootHeapNode(0);
    huRoots.HeapNode = utNewAInitFirst(huNode, huAllocatedRootHeapNode());
    huRoots.UsedHeapNode = utNewAInitFirst(uint32, (huAllocatedRoot()));
    huRoots.FirstNode = utNewAInitFirst(huNode, (huAllocatedRoot()));
    huRoots.LastNode = utNewAInitFirst(huNode, (huAllocatedRoot()));
    huRoots.NodeTableIndex_ = utNewAInitFirst(uint32, (huAllocatedRoot()));
    huRoots.NumNodeTable = utNewAInitFirst(uint32, (huAllocatedRoot()));
    huSetUsedRootNodeTable(0);
    huSetAllocatedRootNodeTable(2);
    huSetFreeRootNodeTable(0);
    huRoots.NodeTable = utNewAInitFirst(huNode, huAllocatedRootNodeTable());
    huRoots.NumNode = utNewAInitFirst(uint32, (huAllocatedRoot()));
    huRoots.TopNode = utNewAInitFirst(huNode, (huAllocatedRoot()));
}

/*----------------------------------------------------------------------------------------
  Realloc the arrays of properties for class Root.
----------------------------------------------------------------------------------------*/
static void reallocRoots(
    uint32 newSize)
{
    utResizeArray(huRoots.HeapNodeIndex_, (newSize));
    utResizeArray(huRoots.NumHeapNode, (newSize));
    utResizeArray(huRoots.UsedHeapNode, (newSize));
    utResizeArray(huRoots.FirstNode, (newSize));
    utResizeArray(huRoots.LastNode, (newSize));
    utResizeArray(huRoots.NodeTableIndex_, (newSize));
    utResizeArray(huRoots.NumNodeTable, (newSize));
    utResizeArray(huRoots.NumNode, (newSize));
    utResizeArray(huRoots.TopNode, (newSize));
    huSetAllocatedRoot(newSize);
}

/*----------------------------------------------------------------------------------------
  Allocate more Roots.
----------------------------------------------------------------------------------------*/
void huRootAllocMore(void)
{
    reallocRoots((uint32)(huAllocatedRoot() + (huAllocatedRoot() >> 1)));
}

/*----------------------------------------------------------------------------------------
  Compact the Root.HeapNode heap to free memory.
----------------------------------------------------------------------------------------*/
void huCompactRootHeapNodes(void)
{
    uint32 elementSize = sizeof(huNode);
    uint32 usedHeaderSize = (sizeof(huRoot) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(huRoot) + sizeof(uint32) + elementSize - 1)/elementSize;
    huNode *toPtr = huRoots.HeapNode;
    huNode *fromPtr = toPtr;
    huRoot Root;
    uint32 size;

    while(fromPtr < huRoots.HeapNode + huUsedRootHeapNode()) {
        Root = *(huRoot *)(void *)fromPtr;
        if(Root != huRootNull) {
            /* Need to move it to toPtr */
            size = utMax(huRootGetNumHeapNode(Root) + usedHeaderSize, freeHeaderSize);
            memmove((void *)toPtr, (void *)fromPtr, size*elementSize);
            huRootSetHeapNodeIndex_(Root, toPtr - huRoots.HeapNode + usedHeaderSize);
            toPtr += size;
        } else {
            /* Just skip it */
            size = utMax(*(uint32 *)(void *)(((huRoot *)(void *)fromPtr) + 1), freeHeaderSize);
        }
        fromPtr += size;
    }
    huSetUsedRootHeapNode(toPtr - huRoots.HeapNode);
    huSetFreeRootHeapNode(0);
}

/*----------------------------------------------------------------------------------------
  Allocate more memory for the Root.HeapNode heap.
----------------------------------------------------------------------------------------*/
static void allocMoreRootHeapNodes(
    uint32 spaceNeeded)
{
    uint32 freeSpace = huAllocatedRootHeapNode() - huUsedRootHeapNode();
    uint32 elementSize = sizeof(huNode);
    uint32 usedHeaderSize = (sizeof(huRoot) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(huRoot) + sizeof(uint32) + elementSize - 1)/elementSize;
    huNode *ptr = huRoots.HeapNode;
    huRoot Root;
    uint32 size;

    while(ptr < huRoots.HeapNode + huUsedRootHeapNode()) {
        Root = *(huRoot*)(void*)ptr;
        if(Root != huRootNull) {
            huValidRoot(Root);
            size = utMax(huRootGetNumHeapNode(Root) + usedHeaderSize, freeHeaderSize);
        } else {
            size = utMax(*(uint32 *)(void *)(((huRoot *)(void *)ptr) + 1), freeHeaderSize);
        }
        ptr += size;
    }
    if((huFreeRootHeapNode() << 2) > huUsedRootHeapNode()) {
        huCompactRootHeapNodes();
        freeSpace = huAllocatedRootHeapNode() - huUsedRootHeapNode();
    }
    if(freeSpace < spaceNeeded) {
        huSetAllocatedRootHeapNode(huAllocatedRootHeapNode() + spaceNeeded - freeSpace +
            (huAllocatedRootHeapNode() >> 1));
        utResizeArray(huRoots.HeapNode, huAllocatedRootHeapNode());
    }
}

/*----------------------------------------------------------------------------------------
  Allocate memory for a new Root.HeapNode array.
----------------------------------------------------------------------------------------*/
void huRootAllocHeapNodes(
    huRoot Root,
    uint32 numHeapNodes)
{
    uint32 freeSpace = huAllocatedRootHeapNode() - huUsedRootHeapNode();
    uint32 elementSize = sizeof(huNode);
    uint32 usedHeaderSize = (sizeof(huRoot) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(huRoot) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 spaceNeeded = utMax(numHeapNodes + usedHeaderSize, freeHeaderSize);

#if defined(DD_DEBUG)
    utAssert(huRootGetNumHeapNode(Root) == 0);
#endif
    if(numHeapNodes == 0) {
        return;
    }
    if(freeSpace < spaceNeeded) {
        allocMoreRootHeapNodes(spaceNeeded);
    }
    huRootSetHeapNodeIndex_(Root, huUsedRootHeapNode() + usedHeaderSize);
    huRootSetNumHeapNode(Root, numHeapNodes);
    *(huRoot *)(void *)(huRoots.HeapNode + huUsedRootHeapNode()) = Root;
    {
        uint32 xValue;
        for(xValue = (uint32)(huRootGetHeapNodeIndex_(Root)); xValue < huRootGetHeapNodeIndex_(Root) + numHeapNodes; xValue++) {
            huRoots.HeapNode[xValue] = huNodeNull;
        }
    }
    huSetUsedRootHeapNode(huUsedRootHeapNode() + spaceNeeded);
}

/*----------------------------------------------------------------------------------------
  Wrapper around huRootGetHeapNodes for the database manager.
----------------------------------------------------------------------------------------*/
static void *getRootHeapNodes(
    uint64 objectNumber,
    uint32 *numValues)
{
    huRoot Root = huIndex2Root((uint32)objectNumber);

    *numValues = huRootGetNumHeapNode(Root);
    return huRootGetHeapNodes(Root);
}

/*----------------------------------------------------------------------------------------
  Wrapper around huRootAllocHeapNodes for the database manager.
----------------------------------------------------------------------------------------*/
static void *allocRootHeapNodes(
    uint64 objectNumber,
    uint32 numValues)
{
    huRoot Root = huIndex2Root((uint32)objectNumber);

    huRootSetHeapNodeIndex_(Root, 0);
    huRootSetNumHeapNode(Root, 0);
    if(numValues == 0) {
        return NULL;
    }
    huRootAllocHeapNodes(Root, numValues);
    return huRootGetHeapNodes(Root);
}

/*----------------------------------------------------------------------------------------
  Free memory used by the Root.HeapNode array.
----------------------------------------------------------------------------------------*/
void huRootFreeHeapNodes(
    huRoot Root)
{
    uint32 elementSize = sizeof(huNode);
    uint32 usedHeaderSize = (sizeof(huRoot) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(huRoot) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 size = utMax(huRootGetNumHeapNode(Root) + usedHeaderSize, freeHeaderSize);
    huNode *dataPtr = huRootGetHeapNodes(Root) - usedHeaderSize;

    if(huRootGetNumHeapNode(Root) == 0) {
        return;
    }
    *(huRoot *)(void *)(dataPtr) = huRootNull;
    *(uint32 *)(void *)(((huRoot *)(void *)dataPtr) + 1) = size;
    huRootSetNumHeapNode(Root, 0);
    huSetFreeRootHeapNode(huFreeRootHeapNode() + size);
}

/*----------------------------------------------------------------------------------------
  Resize the Root.HeapNode array.
----------------------------------------------------------------------------------------*/
void huRootResizeHeapNodes(
    huRoot Root,
    uint32 numHeapNodes)
{
    uint32 freeSpace;
    uint32 elementSize = sizeof(huNode);
    uint32 usedHeaderSize = (sizeof(huRoot) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(huRoot) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 newSize = utMax(numHeapNodes + usedHeaderSize, freeHeaderSize);
    uint32 oldSize = utMax(huRootGetNumHeapNode(Root) + usedHeaderSize, freeHeaderSize);
    huNode *dataPtr;

    if(numHeapNodes == 0) {
        if(huRootGetNumHeapNode(Root) != 0) {
            huRootFreeHeapNodes(Root);
        }
        return;
    }
    if(huRootGetNumHeapNode(Root) == 0) {
        huRootAllocHeapNodes(Root, numHeapNodes);
        return;
    }
    freeSpace = huAllocatedRootHeapNode() - huUsedRootHeapNode();
    if(freeSpace < newSize) {
        allocMoreRootHeapNodes(newSize);
    }
    dataPtr = huRootGetHeapNodes(Root) - usedHeaderSize;
    memcpy((void *)(huRoots.HeapNode + huUsedRootHeapNode()), dataPtr,
        elementSize*utMin(oldSize, newSize));
    if(newSize > oldSize) {
        {
            uint32 xValue;
            for(xValue = (uint32)(huUsedRootHeapNode() + oldSize); xValue < huUsedRootHeapNode() + oldSize + newSize - oldSize; xValue++) {
                huRoots.HeapNode[xValue] = huNodeNull;
            }
        }
    }
    *(huRoot *)(void *)dataPtr = huRootNull;
    *(uint32 *)(void *)(((huRoot *)(void *)dataPtr) + 1) = oldSize;
    huSetFreeRootHeapNode(huFreeRootHeapNode() + oldSize);
    huRootSetHeapNodeIndex_(Root, huUsedRootHeapNode() + usedHeaderSize);
    huRootSetNumHeapNode(Root, numHeapNodes);
    huSetUsedRootHeapNode(huUsedRootHeapNode() + newSize);
}

/*----------------------------------------------------------------------------------------
  Compact the Root.NodeTable heap to free memory.
----------------------------------------------------------------------------------------*/
void huCompactRootNodeTables(void)
{
    uint32 elementSize = sizeof(huNode);
    uint32 usedHeaderSize = (sizeof(huRoot) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(huRoot) + sizeof(uint32) + elementSize - 1)/elementSize;
    huNode *toPtr = huRoots.NodeTable;
    huNode *fromPtr = toPtr;
    huRoot Root;
    uint32 size;

    while(fromPtr < huRoots.NodeTable + huUsedRootNodeTable()) {
        Root = *(huRoot *)(void *)fromPtr;
        if(Root != huRootNull) {
            /* Need to move it to toPtr */
            size = utMax(huRootGetNumNodeTable(Root) + usedHeaderSize, freeHeaderSize);
            memmove((void *)toPtr, (void *)fromPtr, size*elementSize);
            huRootSetNodeTableIndex_(Root, toPtr - huRoots.NodeTable + usedHeaderSize);
            toPtr += size;
        } else {
            /* Just skip it */
            size = utMax(*(uint32 *)(void *)(((huRoot *)(void *)fromPtr) + 1), freeHeaderSize);
        }
        fromPtr += size;
    }
    huSetUsedRootNodeTable(toPtr - huRoots.NodeTable);
    huSetFreeRootNodeTable(0);
}

/*----------------------------------------------------------------------------------------
  Allocate more memory for the Root.NodeTable heap.
----------------------------------------------------------------------------------------*/
static void allocMoreRootNodeTables(
    uint32 spaceNeeded)
{
    uint32 freeSpace = huAllocatedRootNodeTable() - huUsedRootNodeTable();
    uint32 elementSize = sizeof(huNode);
    uint32 usedHeaderSize = (sizeof(huRoot) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(huRoot) + sizeof(uint32) + elementSize - 1)/elementSize;
    huNode *ptr = huRoots.NodeTable;
    huRoot Root;
    uint32 size;

    while(ptr < huRoots.NodeTable + huUsedRootNodeTable()) {
        Root = *(huRoot*)(void*)ptr;
        if(Root != huRootNull) {
            huValidRoot(Root);
            size = utMax(huRootGetNumNodeTable(Root) + usedHeaderSize, freeHeaderSize);
        } else {
            size = utMax(*(uint32 *)(void *)(((huRoot *)(void *)ptr) + 1), freeHeaderSize);
        }
        ptr += size;
    }
    if((huFreeRootNodeTable() << 2) > huUsedRootNodeTable()) {
        huCompactRootNodeTables();
        freeSpace = huAllocatedRootNodeTable() - huUsedRootNodeTable();
    }
    if(freeSpace < spaceNeeded) {
        huSetAllocatedRootNodeTable(huAllocatedRootNodeTable() + spaceNeeded - freeSpace +
            (huAllocatedRootNodeTable() >> 1));
        utResizeArray(huRoots.NodeTable, huAllocatedRootNodeTable());
    }
}

/*----------------------------------------------------------------------------------------
  Allocate memory for a new Root.NodeTable array.
----------------------------------------------------------------------------------------*/
void huRootAllocNodeTables(
    huRoot Root,
    uint32 numNodeTables)
{
    uint32 freeSpace = huAllocatedRootNodeTable() - huUsedRootNodeTable();
    uint32 elementSize = sizeof(huNode);
    uint32 usedHeaderSize = (sizeof(huRoot) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(huRoot) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 spaceNeeded = utMax(numNodeTables + usedHeaderSize, freeHeaderSize);

#if defined(DD_DEBUG)
    utAssert(huRootGetNumNodeTable(Root) == 0);
#endif
    if(numNodeTables == 0) {
        return;
    }
    if(freeSpace < spaceNeeded) {
        allocMoreRootNodeTables(spaceNeeded);
    }
    huRootSetNodeTableIndex_(Root, huUsedRootNodeTable() + usedHeaderSize);
    huRootSetNumNodeTable(Root, numNodeTables);
    *(huRoot *)(void *)(huRoots.NodeTable + huUsedRootNodeTable()) = Root;
    {
        uint32 xValue;
        for(xValue = (uint32)(huRootGetNodeTableIndex_(Root)); xValue < huRootGetNodeTableIndex_(Root) + numNodeTables; xValue++) {
            huRoots.NodeTable[xValue] = huNodeNull;
        }
    }
    huSetUsedRootNodeTable(huUsedRootNodeTable() + spaceNeeded);
}

/*----------------------------------------------------------------------------------------
  Wrapper around huRootGetNodeTables for the database manager.
----------------------------------------------------------------------------------------*/
static void *getRootNodeTables(
    uint64 objectNumber,
    uint32 *numValues)
{
    huRoot Root = huIndex2Root((uint32)objectNumber);

    *numValues = huRootGetNumNodeTable(Root);
    return huRootGetNodeTables(Root);
}

/*----------------------------------------------------------------------------------------
  Wrapper around huRootAllocNodeTables for the database manager.
----------------------------------------------------------------------------------------*/
static void *allocRootNodeTables(
    uint64 objectNumber,
    uint32 numValues)
{
    huRoot Root = huIndex2Root((uint32)objectNumber);

    huRootSetNodeTableIndex_(Root, 0);
    huRootSetNumNodeTable(Root, 0);
    if(numValues == 0) {
        return NULL;
    }
    huRootAllocNodeTables(Root, numValues);
    return huRootGetNodeTables(Root);
}

/*----------------------------------------------------------------------------------------
  Free memory used by the Root.NodeTable array.
----------------------------------------------------------------------------------------*/
void huRootFreeNodeTables(
    huRoot Root)
{
    uint32 elementSize = sizeof(huNode);
    uint32 usedHeaderSize = (sizeof(huRoot) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(huRoot) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 size = utMax(huRootGetNumNodeTable(Root) + usedHeaderSize, freeHeaderSize);
    huNode *dataPtr = huRootGetNodeTables(Root) - usedHeaderSize;

    if(huRootGetNumNodeTable(Root) == 0) {
        return;
    }
    *(huRoot *)(void *)(dataPtr) = huRootNull;
    *(uint32 *)(void *)(((huRoot *)(void *)dataPtr) + 1) = size;
    huRootSetNumNodeTable(Root, 0);
    huSetFreeRootNodeTable(huFreeRootNodeTable() + size);
}

/*----------------------------------------------------------------------------------------
  Resize the Root.NodeTable array.
----------------------------------------------------------------------------------------*/
void huRootResizeNodeTables(
    huRoot Root,
    uint32 numNodeTables)
{
    uint32 freeSpace;
    uint32 elementSize = sizeof(huNode);
    uint32 usedHeaderSize = (sizeof(huRoot) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(huRoot) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 newSize = utMax(numNodeTables + usedHeaderSize, freeHeaderSize);
    uint32 oldSize = utMax(huRootGetNumNodeTable(Root) + usedHeaderSize, freeHeaderSize);
    huNode *dataPtr;

    if(numNodeTables == 0) {
        if(huRootGetNumNodeTable(Root) != 0) {
            huRootFreeNodeTables(Root);
        }
        return;
    }
    if(huRootGetNumNodeTable(Root) == 0) {
        huRootAllocNodeTables(Root, numNodeTables);
        return;
    }
    freeSpace = huAllocatedRootNodeTable() - huUsedRootNodeTable();
    if(freeSpace < newSize) {
        allocMoreRootNodeTables(newSize);
    }
    dataPtr = huRootGetNodeTables(Root) - usedHeaderSize;
    memcpy((void *)(huRoots.NodeTable + huUsedRootNodeTable()), dataPtr,
        elementSize*utMin(oldSize, newSize));
    if(newSize > oldSize) {
        {
            uint32 xValue;
            for(xValue = (uint32)(huUsedRootNodeTable() + oldSize); xValue < huUsedRootNodeTable() + oldSize + newSize - oldSize; xValue++) {
                huRoots.NodeTable[xValue] = huNodeNull;
            }
        }
    }
    *(huRoot *)(void *)dataPtr = huRootNull;
    *(uint32 *)(void *)(((huRoot *)(void *)dataPtr) + 1) = oldSize;
    huSetFreeRootNodeTable(huFreeRootNodeTable() + oldSize);
    huRootSetNodeTableIndex_(Root, huUsedRootNodeTable() + usedHeaderSize);
    huRootSetNumNodeTable(Root, numNodeTables);
    huSetUsedRootNodeTable(huUsedRootNodeTable() + newSize);
}

/*----------------------------------------------------------------------------------------
  Copy the properties of Root.
----------------------------------------------------------------------------------------*/
void huRootCopyProps(
    huRoot oldRoot,
    huRoot newRoot)
{
}

/*----------------------------------------------------------------------------------------
  Swap two elements in the heap.
----------------------------------------------------------------------------------------*/
static void rootSwapHeapNode(
    huRoot Root,
    uint32 x,
    uint32 y)
{
    huNode newY = huRootGetiHeapNode(Root, x);
    huNode newX = huRootGetiHeapNode(Root, y);

    huRootSetiHeapNode(Root, x, newX);
    huRootSetiHeapNode(Root, y, newY);
    huNodeSetRootIndex(newX, x);
    huNodeSetRootIndex(newY, y);
}

/*----------------------------------------------------------------------------------------
  Swap the element down in the heap until it's below all it's parents in cost.
----------------------------------------------------------------------------------------*/
static void rootHeapDownHeapNode(
    huRoot Root,
    uint32 startX)
{
    uint32 x = startX;
    uint32 leftIndex;
    uint32 rightIndex;
    huNode cur = huRootGetiHeapNode(Root, x);
    huNode left = huNodeNull;
    huNode right = huNodeNull;
    huNode best;
    
    utDo {
        best = cur;
        leftIndex = (x << 1) + 1;
        rightIndex = leftIndex + 1;
        if(leftIndex < huRootGetUsedHeapNode(Root)) {
            left = huRootGetiHeapNode(Root, leftIndex);
            if(huRootCompareHeapNode(best, left) > 0) {
                best = left;
            }
            if(rightIndex < huRootGetUsedHeapNode(Root)) {
                right = huRootGetiHeapNode(Root, rightIndex);
                if(huRootCompareHeapNode(best, right) > 0) {
                    best = right;
                }
            }
        }
    } utWhile(best != cur) {
        if(best == left) {
            rootSwapHeapNode(Root, x, leftIndex);
            x = leftIndex;
        } else {
            rootSwapHeapNode(Root, x, rightIndex);
            x = rightIndex;
        }
    } utRepeat;
}

/*----------------------------------------------------------------------------------------
  Swap the element up in the heap until it's greater than all it's children in cost.
----------------------------------------------------------------------------------------*/
static void rootHeapUpHeapNode(
    huRoot Root,
    uint32 startX)
{
    
    uint32 x = startX;
    huNode cur = huRootGetiHeapNode(Root, x);
    uint32 parentIndex;
    
    utDo {
        parentIndex = (x - 1) >> 1;
    } utWhile(x > 0 && huRootCompareHeapNode(huRootGetiHeapNode(Root, parentIndex), cur) > 0) {
        rootSwapHeapNode(Root, parentIndex, x);
        x = parentIndex;
    } utRepeat
}

/*----------------------------------------------------------------------------------------
  Return the top of the heap without removing it.
----------------------------------------------------------------------------------------*/
huNode huRootPeekHeapNode(
    huRoot Root)
{
    if(huRootGetUsedHeapNode(Root) == 0) {
        return huNodeNull;
    }
    return huRootGetiHeapNode(Root, 0);
}

/*----------------------------------------------------------------------------------------
  Remove the top element of the heap and return it.
----------------------------------------------------------------------------------------*/
static huNode rootPopHeapNode(
    huRoot Root,
    uint32 index)
{
    huNode cur;
    huNode retval = huRootGetiHeapNode(Root, index);
    uint32 newNum = huRootGetUsedHeapNode(Root) - 1;

    huNodeSetRoot(retval, huRootNull);
    huRootSetUsedHeapNode(Root, newNum);
    if(newNum != 0) {
        cur = huRootGetiHeapNode(Root, newNum);
        huRootSetiHeapNode(Root, index, cur);
        huNodeSetRootIndex(cur, index);
        rootHeapDownHeapNode(Root, index);
    }
    return retval;
}

/*----------------------------------------------------------------------------------------
  Remove the top element of the heap and return it.
----------------------------------------------------------------------------------------*/
huNode huRootPopHeapNode(
    huRoot Root)
{
    if(huRootGetUsedHeapNode(Root) == 0) {
        return huNodeNull;
    }
    return rootPopHeapNode(Root, 0);
}

/*----------------------------------------------------------------------------------------
  Float up the heap element.  This is for cases where an element's cost has been changed.
----------------------------------------------------------------------------------------*/
void huRootUpdateHeapNode(
    huRoot Root,
    huNode _Node)
{
    uint32 _index = huNodeGetRootIndex(_Node);

    rootHeapUpHeapNode(Root, _index);
    rootHeapDownHeapNode(Root, _index);
}

/*----------------------------------------------------------------------------------------
  Push the HeapNode onto the RootHeapNode heap.
----------------------------------------------------------------------------------------*/
void huRootPushHeapNode(
    huRoot Root,
    huNode _Node)
{
    uint32 usedHeapNode = huRootGetUsedHeapNode(Root);

#if defined(DD_DEBUG)
    if(Root == huRootNull) {
        utExit("Non existent Root");
    }
#endif
    if(usedHeapNode >= huRootGetNumHeapNode(Root)) {
        huRootResizeHeapNodes(Root, usedHeapNode + (usedHeapNode << 1) + 1);
    }
    huRootSetiHeapNode(Root, usedHeapNode, _Node);
    huRootSetUsedHeapNode(Root, usedHeapNode + 1);
    huNodeSetRootIndex(_Node, usedHeapNode);
    huNodeSetRoot(_Node, Root);
    rootHeapUpHeapNode(Root, usedHeapNode);
}

/*----------------------------------------------------------------------------------------
  Remove the HeapNode from the Root.
----------------------------------------------------------------------------------------*/
void huRootRemoveHeapNode(
    huRoot Root,
    huNode _Node)
{
#if defined(DD_DEBUG)
    if(_Node == huNodeNull) {
        utExit("Non-existent Node");
    }
    if(huNodeGetRoot(_Node) != huRootNull && huNodeGetRoot(_Node) != Root) {
        utExit("Delete Node from non-owning Root");
    }
#endif
    rootPopHeapNode(Root, huNodeGetRootIndex(_Node));
}

/*----------------------------------------------------------------------------------------
  Increase the size of the hash table.
----------------------------------------------------------------------------------------*/
static void resizeRootNodeHashTable(
    huRoot Root)
{
    huNode _Node, prevNode, nextNode;
    uint32 oldNumNodes = huRootGetNumNodeTable(Root);
    uint32 newNumNodes = oldNumNodes << 1;
    uint32 xNode, index;

    if(newNumNodes == 0) {
        newNumNodes = 2;
        huRootAllocNodeTables(Root, 2);
    } else {
        huRootResizeNodeTables(Root, newNumNodes);
    }
    for(xNode = 0; xNode < oldNumNodes; xNode++) {
        _Node = huRootGetiNodeTable(Root, xNode);
        prevNode = huNodeNull;
        while(_Node != huNodeNull) {
            nextNode = huNodeGetNextTableRootNode(_Node);
            index = (newNumNodes - 1) & utSym2Index(huNodeGetSym(_Node));
            if(index != xNode) {
                if(prevNode == huNodeNull) {
                    huRootSetiNodeTable(Root, xNode, nextNode);
                } else {
                    huNodeSetNextTableRootNode(prevNode, nextNode);
                }
                huNodeSetNextTableRootNode(_Node, huRootGetiNodeTable(Root, index));
                huRootSetiNodeTable(Root, index, _Node);
            } else {
                prevNode = _Node;
            }
            _Node = nextNode;
        }
    }
}

/*----------------------------------------------------------------------------------------
  Add the Node to the Root.  If the table is near full, build a new one twice
  as big, delete the old one, and return the new one.
----------------------------------------------------------------------------------------*/
static void addRootNodeToHashTable(
    huRoot Root,
    huNode _Node)
{
    huNode nextNode;
    uint32 index;

    if(huRootGetNumNode(Root) >> 1 >= huRootGetNumNodeTable(Root)) {
        resizeRootNodeHashTable(Root);
    }
    index = (huRootGetNumNodeTable(Root) - 1) & utSym2Index(huNodeGetSym(_Node));
    nextNode = huRootGetiNodeTable(Root, index);
    huNodeSetNextTableRootNode(_Node, nextNode);
    huRootSetiNodeTable(Root, index, _Node);
    huRootSetNumNode(Root, huRootGetNumNode(Root) + 1);
}

/*----------------------------------------------------------------------------------------
  Remove the Node from the hash table.
----------------------------------------------------------------------------------------*/
static void removeRootNodeFromHashTable(
    huRoot Root,
    huNode _Node)
{
    uint32 index = (huRootGetNumNodeTable(Root) - 1) & utSym2Index(huNodeGetSym(_Node));
    huNode prevNode, nextNode;
    
    nextNode = huRootGetiNodeTable(Root, index);
    if(nextNode == _Node) {
        huRootSetiNodeTable(Root, index, huNodeGetNextTableRootNode(nextNode));
    } else {
        do {
            prevNode = nextNode;
            nextNode = huNodeGetNextTableRootNode(nextNode);
        } while(nextNode != _Node);
        huNodeSetNextTableRootNode(prevNode, huNodeGetNextTableRootNode(_Node));
    }
    huRootSetNumNode(Root, huRootGetNumNode(Root) - 1);
    huNodeSetNextTableRootNode(_Node, huNodeNull);
}

/*----------------------------------------------------------------------------------------
  Find the Node from the Root and its hash key.
----------------------------------------------------------------------------------------*/
huNode huRootFindNode(
    huRoot Root,
    utSym Sym)
{
    uint32 mask = huRootGetNumNodeTable(Root) - 1;
    huNode _Node;

    if(mask + 1 != 0) {
        _Node = huRootGetiNodeTable(Root, utSym2Index(Sym) & mask);
        while(_Node != huNodeNull) {
            if(huNodeGetSym(_Node) == Sym) {
                return _Node;
            }
            _Node = huNodeGetNextTableRootNode(_Node);
        }
    }
    return huNodeNull;
}

/*----------------------------------------------------------------------------------------
  Find the Node from the Root and its name.
----------------------------------------------------------------------------------------*/
void huRootRenameNode(
    huRoot Root,
    huNode _Node,
    utSym sym)
{
    if(huNodeGetSym(_Node) != utSymNull) {
        removeRootNodeFromHashTable(Root, _Node);
    }
    huNodeSetSym(_Node, sym);
    if(sym != utSymNull) {
        addRootNodeToHashTable(Root, _Node);
    }
}

/*----------------------------------------------------------------------------------------
  Add the Node to the head of the list on the Root.
----------------------------------------------------------------------------------------*/
void huRootInsertNode(
    huRoot Root,
    huNode _Node)
{
#if defined(DD_DEBUG)
    if(Root == huRootNull) {
        utExit("Non-existent Root");
    }
    if(_Node == huNodeNull) {
        utExit("Non-existent Node");
    }
#endif
    huNodeSetNextRootNode(_Node, huRootGetFirstNode(Root));
    if(huRootGetFirstNode(Root) != huNodeNull) {
        huNodeSetPrevRootNode(huRootGetFirstNode(Root), _Node);
    }
    huRootSetFirstNode(Root, _Node);
    huNodeSetPrevRootNode(_Node, huNodeNull);
    if(huRootGetLastNode(Root) == huNodeNull) {
        huRootSetLastNode(Root, _Node);
    }
    if(huNodeGetSym(_Node) != utSymNull) {
        addRootNodeToHashTable(Root, _Node);
    }
}

/*----------------------------------------------------------------------------------------
  Add the Node to the end of the list on the Root.
----------------------------------------------------------------------------------------*/
void huRootAppendNode(
    huRoot Root,
    huNode _Node)
{
#if defined(DD_DEBUG)
    if(Root == huRootNull) {
        utExit("Non-existent Root");
    }
    if(_Node == huNodeNull) {
        utExit("Non-existent Node");
    }
#endif
    huNodeSetPrevRootNode(_Node, huRootGetLastNode(Root));
    if(huRootGetLastNode(Root) != huNodeNull) {
        huNodeSetNextRootNode(huRootGetLastNode(Root), _Node);
    }
    huRootSetLastNode(Root, _Node);
    huNodeSetNextRootNode(_Node, huNodeNull);
    if(huRootGetFirstNode(Root) == huNodeNull) {
        huRootSetFirstNode(Root, _Node);
    }
    if(huNodeGetSym(_Node) != utSymNull) {
        addRootNodeToHashTable(Root, _Node);
    }
}

/*----------------------------------------------------------------------------------------
  Insert the Node to the Root after the previous Node.
----------------------------------------------------------------------------------------*/
void huRootInsertAfterNode(
    huRoot Root,
    huNode prevNode,
    huNode _Node)
{
    huNode nextNode = huNodeGetNextRootNode(prevNode);

#if defined(DD_DEBUG)
    if(Root == huRootNull) {
        utExit("Non-existent Root");
    }
    if(_Node == huNodeNull) {
        utExit("Non-existent Node");
    }
#endif
    huNodeSetNextRootNode(_Node, nextNode);
    huNodeSetNextRootNode(prevNode, _Node);
    huNodeSetPrevRootNode(_Node, prevNode);
    if(nextNode != huNodeNull) {
        huNodeSetPrevRootNode(nextNode, _Node);
    }
    if(huRootGetLastNode(Root) == prevNode) {
        huRootSetLastNode(Root, _Node);
    }
    if(huNodeGetSym(_Node) != utSymNull) {
        addRootNodeToHashTable(Root, _Node);
    }
}

/*----------------------------------------------------------------------------------------
 Remove the Node from the Root.
----------------------------------------------------------------------------------------*/
void huRootRemoveNode(
    huRoot Root,
    huNode _Node)
{
    huNode pNode, nNode;

#if defined(DD_DEBUG)
    if(_Node == huNodeNull) {
        utExit("Non-existent Node");
    }
#endif
    nNode = huNodeGetNextRootNode(_Node);
    pNode = huNodeGetPrevRootNode(_Node);
    if(pNode != huNodeNull) {
        huNodeSetNextRootNode(pNode, nNode);
    } else if(huRootGetFirstNode(Root) == _Node) {
        huRootSetFirstNode(Root, nNode);
    }
    if(nNode != huNodeNull) {
        huNodeSetPrevRootNode(nNode, pNode);
    } else if(huRootGetLastNode(Root) == _Node) {
        huRootSetLastNode(Root, pNode);
    }
    huNodeSetNextRootNode(_Node, huNodeNull);
    huNodeSetPrevRootNode(_Node, huNodeNull);
    if(huNodeGetSym(_Node) != utSymNull) {
        removeRootNodeFromHashTable(Root, _Node);
    }
}

#if defined(DD_DEBUG)
/*----------------------------------------------------------------------------------------
  Write out all the fields of an object.
----------------------------------------------------------------------------------------*/
void huShowRoot(
    huRoot Root)
{
    utDatabaseShowObject("hu", "Root", huRoot2Index(Root));
}
#endif

/*----------------------------------------------------------------------------------------
  Destroy Node including everything in it. Remove from parents.
----------------------------------------------------------------------------------------*/
void huNodeDestroy(
    huNode Node)
{
    huNode LeftNode_;
    huNode RightNode_;
    huRoot owningRoot = huNodeGetRoot(Node);
    huNode owningNode = huNodeGetNode(Node);

    if(huNodeDestructorCallback != NULL) {
        huNodeDestructorCallback(Node);
    }
    LeftNode_ = huNodeGetLeftNode(Node);
    if(LeftNode_ != huNodeNull) {
        huNodeSetNode(LeftNode_, huNodeNull);
    }
    RightNode_ = huNodeGetRightNode(Node);
    if(RightNode_ != huNodeNull) {
        huNodeSetNode(RightNode_, huNodeNull);
    }
    if(owningRoot != huRootNull) {
        huRootRemoveHeapNode(owningRoot, Node);
    }
    if(owningNode != huNodeNull) {
        huNodeSetLeftNode(owningNode, huNodeNull);
    }
    huNodeFree(Node);
}

/*----------------------------------------------------------------------------------------
  Default constructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static uint64 allocNode(void)
{
    huNode Node = huNodeAlloc();

    return huNode2Index(Node);
}

/*----------------------------------------------------------------------------------------
  Destructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static void destroyNode(
    uint64 objectIndex)
{
    huNodeDestroy(huIndex2Node((uint32)objectIndex));
}

/*----------------------------------------------------------------------------------------
  Allocate the field arrays of Node.
----------------------------------------------------------------------------------------*/
static void allocNodes(void)
{
    huSetAllocatedNode(2);
    huSetUsedNode(1);
    huSetFirstFreeNode(huNodeNull);
    huNodes.Sym = utNewAInitFirst(utSym, (huAllocatedNode()));
    huNodes.Count = utNewAInitFirst(uint64, (huAllocatedNode()));
    huNodes.NegCount = utNewAInitFirst(int64, (huAllocatedNode()));
    huNodes.Path = utNewAInitFirst(uint32, (huAllocatedNode()));
    huNodes.PathLength = utNewAInitFirst(uint32, (huAllocatedNode()));
    huNodes.Root = utNewAInitFirst(huRoot, (huAllocatedNode()));
    huNodes.RootIndex = utNewAInitFirst(uint32, (huAllocatedNode()));
    huNodes.NextRootNode = utNewAInitFirst(huNode, (huAllocatedNode()));
    huNodes.PrevRootNode = utNewAInitFirst(huNode, (huAllocatedNode()));
    huNodes.NextTableRootNode = utNewAInitFirst(huNode, (huAllocatedNode()));
    huNodes.Node = utNewAInitFirst(huNode, (huAllocatedNode()));
    huNodes.LeftNode = utNewAInitFirst(huNode, (huAllocatedNode()));
    huNodes.RightNode = utNewAInitFirst(huNode, (huAllocatedNode()));
}

/*----------------------------------------------------------------------------------------
  Realloc the arrays of properties for class Node.
----------------------------------------------------------------------------------------*/
static void reallocNodes(
    uint32 newSize)
{
    utResizeArray(huNodes.Sym, (newSize));
    utResizeArray(huNodes.Count, (newSize));
    utResizeArray(huNodes.NegCount, (newSize));
    utResizeArray(huNodes.Path, (newSize));
    utResizeArray(huNodes.PathLength, (newSize));
    utResizeArray(huNodes.Root, (newSize));
    utResizeArray(huNodes.RootIndex, (newSize));
    utResizeArray(huNodes.NextRootNode, (newSize));
    utResizeArray(huNodes.PrevRootNode, (newSize));
    utResizeArray(huNodes.NextTableRootNode, (newSize));
    utResizeArray(huNodes.Node, (newSize));
    utResizeArray(huNodes.LeftNode, (newSize));
    utResizeArray(huNodes.RightNode, (newSize));
    huSetAllocatedNode(newSize);
}

/*----------------------------------------------------------------------------------------
  Allocate more Nodes.
----------------------------------------------------------------------------------------*/
void huNodeAllocMore(void)
{
    reallocNodes((uint32)(huAllocatedNode() + (huAllocatedNode() >> 1)));
}

/*----------------------------------------------------------------------------------------
  Copy the properties of Node.
----------------------------------------------------------------------------------------*/
void huNodeCopyProps(
    huNode oldNode,
    huNode newNode)
{
    huNodeSetCount(newNode, huNodeGetCount(oldNode));
    huNodeSetNegCount(newNode, huNodeGetNegCount(oldNode));
    huNodeSetPath(newNode, huNodeGetPath(oldNode));
    huNodeSetPathLength(newNode, huNodeGetPathLength(oldNode));
}

#if defined(DD_DEBUG)
/*----------------------------------------------------------------------------------------
  Write out all the fields of an object.
----------------------------------------------------------------------------------------*/
void huShowNode(
    huNode Node)
{
    utDatabaseShowObject("hu", "Node", huNode2Index(Node));
}
#endif

/*----------------------------------------------------------------------------------------
  Free memory used by the hu database.
----------------------------------------------------------------------------------------*/
void huDatabaseStop(void)
{
    utFree(huRoots.HeapNodeIndex_);
    utFree(huRoots.NumHeapNode);
    utFree(huRoots.HeapNode);
    utFree(huRoots.UsedHeapNode);
    utFree(huRoots.FirstNode);
    utFree(huRoots.LastNode);
    utFree(huRoots.NodeTableIndex_);
    utFree(huRoots.NumNodeTable);
    utFree(huRoots.NodeTable);
    utFree(huRoots.NumNode);
    utFree(huRoots.TopNode);
    utFree(huNodes.Sym);
    utFree(huNodes.Count);
    utFree(huNodes.NegCount);
    utFree(huNodes.Path);
    utFree(huNodes.PathLength);
    utFree(huNodes.Root);
    utFree(huNodes.RootIndex);
    utFree(huNodes.NextRootNode);
    utFree(huNodes.PrevRootNode);
    utFree(huNodes.NextTableRootNode);
    utFree(huNodes.Node);
    utFree(huNodes.LeftNode);
    utFree(huNodes.RightNode);
    utUnregisterModule(huModuleID);
}

/*----------------------------------------------------------------------------------------
  Allocate memory used by the hu database.
----------------------------------------------------------------------------------------*/
void huDatabaseStart(void)
{
    if(!utInitialized()) {
        utStart();
    }
    huRootData.hash = 0xc7c83515;
    huModuleID = utRegisterModule("hu", false, huHash(), 2, 24, 0, sizeof(struct huRootType_),
        &huRootData, huDatabaseStart, huDatabaseStop);
    utRegisterClass("Root", 11, &huRootData.usedRoot, &huRootData.allocatedRoot,
        &huRootData.firstFreeRoot, 4, 4, allocRoot, destroyRoot);
    utRegisterField("HeapNodeIndex_", &huRoots.HeapNodeIndex_, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("NumHeapNode", &huRoots.NumHeapNode, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("HeapNode", &huRoots.HeapNode, sizeof(huNode), UT_POINTER, "Node");
    utRegisterArray(&huRootData.usedRootHeapNode, &huRootData.allocatedRootHeapNode,
        getRootHeapNodes, allocRootHeapNodes, huCompactRootHeapNodes);
    utRegisterField("UsedHeapNode", &huRoots.UsedHeapNode, sizeof(uint32), UT_UINT, NULL);
    utRegisterField("FirstNode", &huRoots.FirstNode, sizeof(huNode), UT_POINTER, "Node");
    utRegisterField("LastNode", &huRoots.LastNode, sizeof(huNode), UT_POINTER, "Node");
    utRegisterField("NodeTableIndex_", &huRoots.NodeTableIndex_, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("NumNodeTable", &huRoots.NumNodeTable, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("NodeTable", &huRoots.NodeTable, sizeof(huNode), UT_POINTER, "Node");
    utRegisterArray(&huRootData.usedRootNodeTable, &huRootData.allocatedRootNodeTable,
        getRootNodeTables, allocRootNodeTables, huCompactRootNodeTables);
    utRegisterField("NumNode", &huRoots.NumNode, sizeof(uint32), UT_UINT, NULL);
    utRegisterField("TopNode", &huRoots.TopNode, sizeof(huNode), UT_POINTER, "Node");
    utRegisterClass("Node", 13, &huRootData.usedNode, &huRootData.allocatedNode,
        &huRootData.firstFreeNode, 11, 4, allocNode, destroyNode);
    utRegisterField("Sym", &huNodes.Sym, sizeof(utSym), UT_SYM, NULL);
    utRegisterField("Count", &huNodes.Count, sizeof(uint64), UT_UINT, NULL);
    utRegisterField("NegCount", &huNodes.NegCount, sizeof(int64), UT_INT, NULL);
    utRegisterField("Path", &huNodes.Path, sizeof(uint32), UT_UINT, NULL);
    utRegisterField("PathLength", &huNodes.PathLength, sizeof(uint32), UT_UINT, NULL);
    utRegisterField("Root", &huNodes.Root, sizeof(huRoot), UT_POINTER, "Root");
    utRegisterField("RootIndex", &huNodes.RootIndex, sizeof(uint32), UT_UINT, NULL);
    utRegisterField("NextRootNode", &huNodes.NextRootNode, sizeof(huNode), UT_POINTER, "Node");
    utRegisterField("PrevRootNode", &huNodes.PrevRootNode, sizeof(huNode), UT_POINTER, "Node");
    utRegisterField("NextTableRootNode", &huNodes.NextTableRootNode, sizeof(huNode), UT_POINTER, "Node");
    utRegisterField("Node", &huNodes.Node, sizeof(huNode), UT_POINTER, "Node");
    utRegisterField("LeftNode", &huNodes.LeftNode, sizeof(huNode), UT_POINTER, "Node");
    utRegisterField("RightNode", &huNodes.RightNode, sizeof(huNode), UT_POINTER, "Node");
    allocRoots();
    allocNodes();
}

