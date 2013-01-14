/*----------------------------------------------------------------------------------------
  Module header file for: hu module
----------------------------------------------------------------------------------------*/
#ifndef HUDATABASE_H

#define HUDATABASE_H

#if defined __cplusplus
extern "C" {
#endif

#ifndef DD_UTIL_H
#include "ddutil.h"
#endif

extern uint8 huModuleID;
/* Class reference definitions */
#if (defined(DD_DEBUG) && !defined(DD_NOSTRICT)) || defined(DD_STRICT)
typedef struct _struct_huRoot{char val;} *huRoot;
#define huRootNull ((huRoot)0)
typedef struct _struct_huNode{char val;} *huNode;
#define huNodeNull ((huNode)0)
#else
typedef uint32 huRoot;
#define huRootNull 0
typedef uint32 huNode;
#define huNodeNull 0
#endif

/* Constructor/Destructor hooks. */
typedef void (*huRootCallbackType)(huRoot);
extern huRootCallbackType huRootConstructorCallback;
extern huRootCallbackType huRootDestructorCallback;
typedef void (*huNodeCallbackType)(huNode);
extern huNodeCallbackType huNodeConstructorCallback;
extern huNodeCallbackType huNodeDestructorCallback;

/*----------------------------------------------------------------------------------------
  Root structure
----------------------------------------------------------------------------------------*/
struct huRootType_ {
    uint32 hash; /* This depends only on the structure of the database */
    huRoot firstFreeRoot;
    uint32 usedRoot, allocatedRoot;
    uint32 usedRootHeapNode, allocatedRootHeapNode, freeRootHeapNode;
    uint32 usedRootNodeTable, allocatedRootNodeTable, freeRootNodeTable;
    huNode firstFreeNode;
    uint32 usedNode, allocatedNode;
};
extern struct huRootType_ huRootData;

utInlineC uint32 huHash(void) {return huRootData.hash;}
utInlineC huRoot huFirstFreeRoot(void) {return huRootData.firstFreeRoot;}
utInlineC void huSetFirstFreeRoot(huRoot value) {huRootData.firstFreeRoot = (value);}
utInlineC uint32 huUsedRoot(void) {return huRootData.usedRoot;}
utInlineC uint32 huAllocatedRoot(void) {return huRootData.allocatedRoot;}
utInlineC void huSetUsedRoot(uint32 value) {huRootData.usedRoot = value;}
utInlineC void huSetAllocatedRoot(uint32 value) {huRootData.allocatedRoot = value;}
utInlineC uint32 huUsedRootHeapNode(void) {return huRootData.usedRootHeapNode;}
utInlineC uint32 huAllocatedRootHeapNode(void) {return huRootData.allocatedRootHeapNode;}
utInlineC uint32 huFreeRootHeapNode(void) {return huRootData.freeRootHeapNode;}
utInlineC void huSetUsedRootHeapNode(uint32 value) {huRootData.usedRootHeapNode = value;}
utInlineC void huSetAllocatedRootHeapNode(uint32 value) {huRootData.allocatedRootHeapNode = value;}
utInlineC void huSetFreeRootHeapNode(int32 value) {huRootData.freeRootHeapNode = value;}
utInlineC uint32 huUsedRootNodeTable(void) {return huRootData.usedRootNodeTable;}
utInlineC uint32 huAllocatedRootNodeTable(void) {return huRootData.allocatedRootNodeTable;}
utInlineC uint32 huFreeRootNodeTable(void) {return huRootData.freeRootNodeTable;}
utInlineC void huSetUsedRootNodeTable(uint32 value) {huRootData.usedRootNodeTable = value;}
utInlineC void huSetAllocatedRootNodeTable(uint32 value) {huRootData.allocatedRootNodeTable = value;}
utInlineC void huSetFreeRootNodeTable(int32 value) {huRootData.freeRootNodeTable = value;}
utInlineC huNode huFirstFreeNode(void) {return huRootData.firstFreeNode;}
utInlineC void huSetFirstFreeNode(huNode value) {huRootData.firstFreeNode = (value);}
utInlineC uint32 huUsedNode(void) {return huRootData.usedNode;}
utInlineC uint32 huAllocatedNode(void) {return huRootData.allocatedNode;}
utInlineC void huSetUsedNode(uint32 value) {huRootData.usedNode = value;}
utInlineC void huSetAllocatedNode(uint32 value) {huRootData.allocatedNode = value;}

/* Validate macros */
#if defined(DD_DEBUG)
utInlineC huRoot huValidRoot(huRoot Root) {
    utAssert(utLikely(Root != huRootNull && (uint32)(Root - (huRoot)0) < huRootData.usedRoot));
    return Root;}
utInlineC huNode huValidNode(huNode Node) {
    utAssert(utLikely(Node != huNodeNull && (uint32)(Node - (huNode)0) < huRootData.usedNode));
    return Node;}
#else
utInlineC huRoot huValidRoot(huRoot Root) {return Root;}
utInlineC huNode huValidNode(huNode Node) {return Node;}
#endif

/* Object ref to integer conversions */
#if (defined(DD_DEBUG) && !defined(DD_NOSTRICT)) || defined(DD_STRICT)
utInlineC uint32 huRoot2Index(huRoot Root) {return Root - (huRoot)0;}
utInlineC uint32 huRoot2ValidIndex(huRoot Root) {return huValidRoot(Root) - (huRoot)0;}
utInlineC huRoot huIndex2Root(uint32 xRoot) {return (huRoot)(xRoot + (huRoot)(0));}
utInlineC uint32 huNode2Index(huNode Node) {return Node - (huNode)0;}
utInlineC uint32 huNode2ValidIndex(huNode Node) {return huValidNode(Node) - (huNode)0;}
utInlineC huNode huIndex2Node(uint32 xNode) {return (huNode)(xNode + (huNode)(0));}
#else
utInlineC uint32 huRoot2Index(huRoot Root) {return Root;}
utInlineC uint32 huRoot2ValidIndex(huRoot Root) {return huValidRoot(Root);}
utInlineC huRoot huIndex2Root(uint32 xRoot) {return xRoot;}
utInlineC uint32 huNode2Index(huNode Node) {return Node;}
utInlineC uint32 huNode2ValidIndex(huNode Node) {return huValidNode(Node);}
utInlineC huNode huIndex2Node(uint32 xNode) {return xNode;}
#endif

/*----------------------------------------------------------------------------------------
  Fields for class Root.
----------------------------------------------------------------------------------------*/
struct huRootFields {
    uint32 *HeapNodeIndex_;
    uint32 *NumHeapNode;
    huNode *HeapNode;
    uint32 *UsedHeapNode;
    huNode *FirstNode;
    huNode *LastNode;
    uint32 *NodeTableIndex_;
    uint32 *NumNodeTable;
    huNode *NodeTable;
    uint32 *NumNode;
    huNode *TopNode;
};
extern struct huRootFields huRoots;

void huRootAllocMore(void);
void huRootCopyProps(huRoot huOldRoot, huRoot huNewRoot);
void huRootAllocHeapNodes(huRoot Root, uint32 numHeapNodes);
void huRootResizeHeapNodes(huRoot Root, uint32 numHeapNodes);
void huRootFreeHeapNodes(huRoot Root);
void huCompactRootHeapNodes(void);
void huRootAllocNodeTables(huRoot Root, uint32 numNodeTables);
void huRootResizeNodeTables(huRoot Root, uint32 numNodeTables);
void huRootFreeNodeTables(huRoot Root);
void huCompactRootNodeTables(void);
utInlineC uint32 huRootGetHeapNodeIndex_(huRoot Root) {return huRoots.HeapNodeIndex_[huRoot2ValidIndex(Root)];}
utInlineC void huRootSetHeapNodeIndex_(huRoot Root, uint32 value) {huRoots.HeapNodeIndex_[huRoot2ValidIndex(Root)] = value;}
utInlineC uint32 huRootGetNumHeapNode(huRoot Root) {return huRoots.NumHeapNode[huRoot2ValidIndex(Root)];}
utInlineC void huRootSetNumHeapNode(huRoot Root, uint32 value) {huRoots.NumHeapNode[huRoot2ValidIndex(Root)] = value;}
#if defined(DD_DEBUG)
utInlineC uint32 huRootCheckHeapNodeIndex(huRoot Root, uint32 x) {utAssert(x < huRootGetNumHeapNode(Root)); return x;}
#else
utInlineC uint32 huRootCheckHeapNodeIndex(huRoot Root, uint32 x) {return x;}
#endif
utInlineC huNode huRootGetiHeapNode(huRoot Root, uint32 x) {return huRoots.HeapNode[
    huRootGetHeapNodeIndex_(Root) + huRootCheckHeapNodeIndex(Root, x)];}
utInlineC huNode *huRootGetHeapNode(huRoot Root) {return huRoots.HeapNode + huRootGetHeapNodeIndex_(Root);}
#define huRootGetHeapNodes huRootGetHeapNode
utInlineC void huRootSetHeapNode(huRoot Root, huNode *valuePtr, uint32 numHeapNode) {
    huRootResizeHeapNodes(Root, numHeapNode);
    memcpy(huRootGetHeapNodes(Root), valuePtr, numHeapNode*sizeof(huNode));}
utInlineC void huRootSetiHeapNode(huRoot Root, uint32 x, huNode value) {
    huRoots.HeapNode[huRootGetHeapNodeIndex_(Root) + huRootCheckHeapNodeIndex(Root, (x))] = value;}
utInlineC uint32 huRootGetUsedHeapNode(huRoot Root) {return huRoots.UsedHeapNode[huRoot2ValidIndex(Root)];}
utInlineC void huRootSetUsedHeapNode(huRoot Root, uint32 value) {huRoots.UsedHeapNode[huRoot2ValidIndex(Root)] = value;}
utInlineC huNode huRootGetFirstNode(huRoot Root) {return huRoots.FirstNode[huRoot2ValidIndex(Root)];}
utInlineC void huRootSetFirstNode(huRoot Root, huNode value) {huRoots.FirstNode[huRoot2ValidIndex(Root)] = value;}
utInlineC huNode huRootGetLastNode(huRoot Root) {return huRoots.LastNode[huRoot2ValidIndex(Root)];}
utInlineC void huRootSetLastNode(huRoot Root, huNode value) {huRoots.LastNode[huRoot2ValidIndex(Root)] = value;}
utInlineC uint32 huRootGetNodeTableIndex_(huRoot Root) {return huRoots.NodeTableIndex_[huRoot2ValidIndex(Root)];}
utInlineC void huRootSetNodeTableIndex_(huRoot Root, uint32 value) {huRoots.NodeTableIndex_[huRoot2ValidIndex(Root)] = value;}
utInlineC uint32 huRootGetNumNodeTable(huRoot Root) {return huRoots.NumNodeTable[huRoot2ValidIndex(Root)];}
utInlineC void huRootSetNumNodeTable(huRoot Root, uint32 value) {huRoots.NumNodeTable[huRoot2ValidIndex(Root)] = value;}
#if defined(DD_DEBUG)
utInlineC uint32 huRootCheckNodeTableIndex(huRoot Root, uint32 x) {utAssert(x < huRootGetNumNodeTable(Root)); return x;}
#else
utInlineC uint32 huRootCheckNodeTableIndex(huRoot Root, uint32 x) {return x;}
#endif
utInlineC huNode huRootGetiNodeTable(huRoot Root, uint32 x) {return huRoots.NodeTable[
    huRootGetNodeTableIndex_(Root) + huRootCheckNodeTableIndex(Root, x)];}
utInlineC huNode *huRootGetNodeTable(huRoot Root) {return huRoots.NodeTable + huRootGetNodeTableIndex_(Root);}
#define huRootGetNodeTables huRootGetNodeTable
utInlineC void huRootSetNodeTable(huRoot Root, huNode *valuePtr, uint32 numNodeTable) {
    huRootResizeNodeTables(Root, numNodeTable);
    memcpy(huRootGetNodeTables(Root), valuePtr, numNodeTable*sizeof(huNode));}
utInlineC void huRootSetiNodeTable(huRoot Root, uint32 x, huNode value) {
    huRoots.NodeTable[huRootGetNodeTableIndex_(Root) + huRootCheckNodeTableIndex(Root, (x))] = value;}
utInlineC uint32 huRootGetNumNode(huRoot Root) {return huRoots.NumNode[huRoot2ValidIndex(Root)];}
utInlineC void huRootSetNumNode(huRoot Root, uint32 value) {huRoots.NumNode[huRoot2ValidIndex(Root)] = value;}
utInlineC huNode huRootGetTopNode(huRoot Root) {return huRoots.TopNode[huRoot2ValidIndex(Root)];}
utInlineC void huRootSetTopNode(huRoot Root, huNode value) {huRoots.TopNode[huRoot2ValidIndex(Root)] = value;}
utInlineC void huRootSetConstructorCallback(void(*func)(huRoot)) {huRootConstructorCallback = func;}
utInlineC huRootCallbackType huRootGetConstructorCallback(void) {return huRootConstructorCallback;}
utInlineC void huRootSetDestructorCallback(void(*func)(huRoot)) {huRootDestructorCallback = func;}
utInlineC huRootCallbackType huRootGetDestructorCallback(void) {return huRootDestructorCallback;}
utInlineC huRoot huRootNextFree(huRoot Root) {return ((huRoot *)(void *)(huRoots.FirstNode))[huRoot2ValidIndex(Root)];}
utInlineC void huRootSetNextFree(huRoot Root, huRoot value) {
    ((huRoot *)(void *)(huRoots.FirstNode))[huRoot2ValidIndex(Root)] = value;}
utInlineC void huRootFree(huRoot Root) {
    huRootFreeHeapNodes(Root);
    huRootFreeNodeTables(Root);
    huRootSetNextFree(Root, huRootData.firstFreeRoot);
    huSetFirstFreeRoot(Root);}
void huRootDestroy(huRoot Root);
utInlineC huRoot huRootAllocRaw(void) {
    huRoot Root;
    if(huRootData.firstFreeRoot != huRootNull) {
        Root = huRootData.firstFreeRoot;
        huSetFirstFreeRoot(huRootNextFree(Root));
    } else {
        if(huRootData.usedRoot == huRootData.allocatedRoot) {
            huRootAllocMore();
        }
        Root = huIndex2Root(huRootData.usedRoot);
        huSetUsedRoot(huUsedRoot() + 1);
    }
    return Root;}
utInlineC huRoot huRootAlloc(void) {
    huRoot Root = huRootAllocRaw();
    huRootSetHeapNodeIndex_(Root, 0);
    huRootSetNumHeapNode(Root, 0);
    huRootSetNumHeapNode(Root, 0);
    huRootSetUsedHeapNode(Root, 0);
    huRootSetFirstNode(Root, huNodeNull);
    huRootSetLastNode(Root, huNodeNull);
    huRootSetNodeTableIndex_(Root, 0);
    huRootSetNumNodeTable(Root, 0);
    huRootSetNumNodeTable(Root, 0);
    huRootSetNumNode(Root, 0);
    huRootSetTopNode(Root, huNodeNull);
    if(huRootConstructorCallback != NULL) {
        huRootConstructorCallback(Root);
    }
    return Root;}

/*----------------------------------------------------------------------------------------
  Fields for class Node.
----------------------------------------------------------------------------------------*/
struct huNodeFields {
    utSym *Sym;
    uint64 *Count;
    int64 *NegCount;
    uint32 *Path;
    uint32 *PathLength;
    huRoot *Root;
    uint32 *RootIndex;
    huNode *NextRootNode;
    huNode *PrevRootNode;
    huNode *NextTableRootNode;
    huNode *Node;
    huNode *LeftNode;
    huNode *RightNode;
};
extern struct huNodeFields huNodes;

void huNodeAllocMore(void);
void huNodeCopyProps(huNode huOldNode, huNode huNewNode);
utInlineC utSym huNodeGetSym(huNode Node) {return huNodes.Sym[huNode2ValidIndex(Node)];}
utInlineC void huNodeSetSym(huNode Node, utSym value) {huNodes.Sym[huNode2ValidIndex(Node)] = value;}
utInlineC uint64 huNodeGetCount(huNode Node) {return huNodes.Count[huNode2ValidIndex(Node)];}
utInlineC void huNodeSetCount(huNode Node, uint64 value) {huNodes.Count[huNode2ValidIndex(Node)] = value;}
utInlineC int64 huNodeGetNegCount(huNode Node) {return huNodes.NegCount[huNode2ValidIndex(Node)];}
utInlineC void huNodeSetNegCount(huNode Node, int64 value) {huNodes.NegCount[huNode2ValidIndex(Node)] = value;}
utInlineC uint32 huNodeGetPath(huNode Node) {return huNodes.Path[huNode2ValidIndex(Node)];}
utInlineC void huNodeSetPath(huNode Node, uint32 value) {huNodes.Path[huNode2ValidIndex(Node)] = value;}
utInlineC uint32 huNodeGetPathLength(huNode Node) {return huNodes.PathLength[huNode2ValidIndex(Node)];}
utInlineC void huNodeSetPathLength(huNode Node, uint32 value) {huNodes.PathLength[huNode2ValidIndex(Node)] = value;}
utInlineC huRoot huNodeGetRoot(huNode Node) {return huNodes.Root[huNode2ValidIndex(Node)];}
utInlineC void huNodeSetRoot(huNode Node, huRoot value) {huNodes.Root[huNode2ValidIndex(Node)] = value;}
utInlineC uint32 huNodeGetRootIndex(huNode Node) {return huNodes.RootIndex[huNode2ValidIndex(Node)];}
utInlineC void huNodeSetRootIndex(huNode Node, uint32 value) {huNodes.RootIndex[huNode2ValidIndex(Node)] = value;}
utInlineC huNode huNodeGetNextRootNode(huNode Node) {return huNodes.NextRootNode[huNode2ValidIndex(Node)];}
utInlineC void huNodeSetNextRootNode(huNode Node, huNode value) {huNodes.NextRootNode[huNode2ValidIndex(Node)] = value;}
utInlineC huNode huNodeGetPrevRootNode(huNode Node) {return huNodes.PrevRootNode[huNode2ValidIndex(Node)];}
utInlineC void huNodeSetPrevRootNode(huNode Node, huNode value) {huNodes.PrevRootNode[huNode2ValidIndex(Node)] = value;}
utInlineC huNode huNodeGetNextTableRootNode(huNode Node) {return huNodes.NextTableRootNode[huNode2ValidIndex(Node)];}
utInlineC void huNodeSetNextTableRootNode(huNode Node, huNode value) {huNodes.NextTableRootNode[huNode2ValidIndex(Node)] = value;}
utInlineC huNode huNodeGetNode(huNode Node) {return huNodes.Node[huNode2ValidIndex(Node)];}
utInlineC void huNodeSetNode(huNode Node, huNode value) {huNodes.Node[huNode2ValidIndex(Node)] = value;}
utInlineC huNode huNodeGetLeftNode(huNode Node) {return huNodes.LeftNode[huNode2ValidIndex(Node)];}
utInlineC void huNodeSetLeftNode(huNode Node, huNode value) {huNodes.LeftNode[huNode2ValidIndex(Node)] = value;}
utInlineC huNode huNodeGetRightNode(huNode Node) {return huNodes.RightNode[huNode2ValidIndex(Node)];}
utInlineC void huNodeSetRightNode(huNode Node, huNode value) {huNodes.RightNode[huNode2ValidIndex(Node)] = value;}
utInlineC void huNodeSetConstructorCallback(void(*func)(huNode)) {huNodeConstructorCallback = func;}
utInlineC huNodeCallbackType huNodeGetConstructorCallback(void) {return huNodeConstructorCallback;}
utInlineC void huNodeSetDestructorCallback(void(*func)(huNode)) {huNodeDestructorCallback = func;}
utInlineC huNodeCallbackType huNodeGetDestructorCallback(void) {return huNodeDestructorCallback;}
utInlineC huNode huNodeNextFree(huNode Node) {return ((huNode *)(void *)(huNodes.Sym))[huNode2ValidIndex(Node)];}
utInlineC void huNodeSetNextFree(huNode Node, huNode value) {
    ((huNode *)(void *)(huNodes.Sym))[huNode2ValidIndex(Node)] = value;}
utInlineC void huNodeFree(huNode Node) {
    huNodeSetNextFree(Node, huRootData.firstFreeNode);
    huSetFirstFreeNode(Node);}
void huNodeDestroy(huNode Node);
utInlineC huNode huNodeAllocRaw(void) {
    huNode Node;
    if(huRootData.firstFreeNode != huNodeNull) {
        Node = huRootData.firstFreeNode;
        huSetFirstFreeNode(huNodeNextFree(Node));
    } else {
        if(huRootData.usedNode == huRootData.allocatedNode) {
            huNodeAllocMore();
        }
        Node = huIndex2Node(huRootData.usedNode);
        huSetUsedNode(huUsedNode() + 1);
    }
    return Node;}
utInlineC huNode huNodeAlloc(void) {
    huNode Node = huNodeAllocRaw();
    huNodeSetSym(Node, utSymNull);
    huNodeSetCount(Node, 0);
    huNodeSetNegCount(Node, 0);
    huNodeSetPath(Node, 0);
    huNodeSetPathLength(Node, 0);
    huNodeSetRoot(Node, huRootNull);
    huNodeSetRootIndex(Node, 0);
    huNodeSetNextRootNode(Node, huNodeNull);
    huNodeSetPrevRootNode(Node, huNodeNull);
    huNodeSetNextTableRootNode(Node, huNodeNull);
    huNodeSetNode(Node, huNodeNull);
    huNodeSetLeftNode(Node, huNodeNull);
    huNodeSetRightNode(Node, huNodeNull);
    if(huNodeConstructorCallback != NULL) {
        huNodeConstructorCallback(Node);
    }
    return Node;}

/*----------------------------------------------------------------------------------------
  Relationship macros between classes.
----------------------------------------------------------------------------------------*/
#define huForeachRootHeapNode(pVar, cVar) { \
    uint32 _xNode; \
    for(_xNode = 0; _xNode < huRootGetUsedHeapNode(pVar); _xNode++) { \
        cVar = huRootGetiHeapNode(pVar, _xNode); \
        if(cVar != huNodeNull) {
#define huEndRootHeapNode }}}
huNode huRootFindNode(huRoot Root, utSym Sym);
void huRootRenameNode(huRoot Root, huNode _Node, utSym sym);
utInlineC char *huNodeGetName(huNode Node) {return utSymGetName(huNodeGetSym(Node));}
#define huForeachRootNode(pVar, cVar) \
    for(cVar = huRootGetFirstNode(pVar); cVar != huNodeNull; \
        cVar = huNodeGetNextRootNode(cVar))
#define huEndRootNode
#define huSafeForeachRootNode(pVar, cVar) { \
    huNode _nextNode; \
    for(cVar = huRootGetFirstNode(pVar); cVar != huNodeNull; cVar = _nextNode) { \
        _nextNode = huNodeGetNextRootNode(cVar);
#define huEndSafeRootNode }}
void huRootRemoveHeapNode(huRoot Root, huNode _Node);
void huRootUpdateHeapNode(huRoot Root, huNode _Node);
huNode huRootPeekHeapNode(huRoot Root);
void huRootPushHeapNode(huRoot Root, huNode _Node);
huNode huRootPopHeapNode(huRoot Root);
utInlineC int huRootCompareHeapNode(huNode left, huNode right) {return huNodeGetNegCount(right) > huNodeGetNegCount(left)? 1 : (huNodeGetNegCount(right) < huNodeGetNegCount(left)? -1 : 0);}
void huRootInsertNode(huRoot Root, huNode _Node);
void huRootRemoveNode(huRoot Root, huNode _Node);
void huRootInsertAfterNode(huRoot Root, huNode prevNode, huNode _Node);
void huRootAppendNode(huRoot Root, huNode _Node);
utInlineC void huNodeInsertLeftNode(huNode Node, huNode _Node) {huNodeSetLeftNode(Node, _Node); huNodeSetNode(_Node, Node);}
utInlineC void huNodeRemoveLeftNode(huNode Node, huNode _Node) {huNodeSetLeftNode(Node, huNodeNull); huNodeSetNode(_Node, huNodeNull);}
utInlineC void huNodeInsertRightNode(huNode Node, huNode _Node) {huNodeSetRightNode(Node, _Node); huNodeSetNode(_Node, Node);}
utInlineC void huNodeRemoveRightNode(huNode Node, huNode _Node) {huNodeSetRightNode(Node, huNodeNull); huNodeSetNode(_Node, huNodeNull);}
void huDatabaseStart(void);
void huDatabaseStop(void);
#if defined __cplusplus
}
#endif

#endif
