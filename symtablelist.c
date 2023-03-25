/* symtablelist.c
Author: Tinney Mak */

#include <assert.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "symtable.h"

/* Each Node contains a binding, consisting of a key and value. Nodes are linked
to form a list. */
struct Node {
    /* The key */
    const char* pcKey;

    /* The value */
    void* pvValue;

    /* The address of the next Node */
    struct Node* psNextNode;
};

/* A SymTable is a structure that points to the first Node. */
struct SymTable {
    /* The address of the first Node */
    struct Node* psFirstNode;
    /* The number of the bindings in the symbol table */
    size_t length;
};

SymTable_T SymTable_new(void) {
    SymTable_T oSymTable;

    oSymTable = (SymTable_T)calloc(1, sizeof(struct SymTable));
    if (oSymTable == NULL) {
        return NULL;
    }

    oSymTable->psFirstNode = NULL;
    oSymTable->length = 0;
    return oSymTable;
}

void SymTable_free(SymTable_T oSymTable) {
    struct Node* psCurrentNode;
    struct Node* psNextNode;

    assert(oSymTable != NULL);

    for (psCurrentNode = oSymTable->psFirstNode;
            psCurrentNode != NULL;
            psCurrentNode = psNextNode) 
        {
            psNextNode = psCurrentNode->psNextNode;
            free((char*)(psCurrentNode->pcKey));
            free(psCurrentNode);
        }

    free(oSymTable);
}

size_t SymTable_getLength(SymTable_T oSymTable) {
    assert(oSymTable != NULL);
    return oSymTable->length;
}

int SymTable_put(SymTable_T oSymTable, 
    const char *pcKey, const void *pvValue) {
        struct Node* psNewNode;

        assert(oSymTable != NULL);
        assert(pcKey != NULL);

        if (SymTable_contains(oSymTable, pcKey) == 1) {
            return 0;
        }

        psNewNode = (struct Node*)calloc(1, sizeof(struct Node));
        if (psNewNode == NULL) {
            return 0;
        }

        /* makes defensive copy of key */
        psNewNode->pcKey = (char*)malloc(strlen(pcKey) + 1);
        if (psNewNode->pcKey == NULL) {
            free(psNewNode);
            return 0;
        }

        strcpy((void*)(psNewNode->pcKey), pcKey);

        psNewNode->pvValue = (void*)pvValue;
        psNewNode->psNextNode = oSymTable->psFirstNode;
        oSymTable->psFirstNode = psNewNode;

        (oSymTable->length)++;
        return 1;
}

void *SymTable_replace(SymTable_T oSymTable, 
    const char *pcKey, const void *pvValue) {
        void* pvOldValue;
        struct Node* psCurrentNode;

        assert(oSymTable != NULL);
        assert(pcKey != NULL);

        if (SymTable_contains(oSymTable, pcKey) == 0) {
            return NULL;
        }

        for (psCurrentNode = oSymTable->psFirstNode;
                psCurrentNode != NULL;
                psCurrentNode = psCurrentNode->psNextNode) 
        {
            if (strcmp(psCurrentNode->pcKey, pcKey) == 0) {
                pvOldValue = psCurrentNode->pvValue;
                psCurrentNode->pvValue = (void*)pvValue;
                return (void*)pvOldValue;
            }
        }
        return NULL;
}

int SymTable_contains(SymTable_T oSymTable, const char *pcKey) {
    struct Node* psCurrentNode;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    for (psCurrentNode = oSymTable->psFirstNode;
            psCurrentNode != NULL;
            psCurrentNode = psCurrentNode->psNextNode) 
        {
            if (strcmp(psCurrentNode->pcKey, pcKey) == 0) {
                return 1;
            }
        }
    return 0;

}

void *SymTable_get(SymTable_T oSymTable, const char *pcKey) {
    struct Node* psCurrentNode;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    if (SymTable_contains(oSymTable, pcKey) == 0) {
            return NULL;
        }
    
    for (psCurrentNode = oSymTable->psFirstNode;
            psCurrentNode != NULL;
            psCurrentNode = psCurrentNode->psNextNode) 
        {
            if (strcmp(psCurrentNode->pcKey, pcKey) == 0) {
                return (void*)psCurrentNode->pvValue;
            }
        }
    return NULL;
}

void *SymTable_remove(SymTable_T oSymTable, const char *pcKey) {
    void* pvOldValue;
    struct Node* psCurrentNode;
    struct Node* psNextNode;
    struct Node* psPreviousNode;
    
    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    if (SymTable_contains(oSymTable, pcKey) == 0) {
            return NULL;
        }

    psPreviousNode = oSymTable->psFirstNode;
    
    for (psCurrentNode = oSymTable->psFirstNode;
            psCurrentNode != NULL;
            psCurrentNode = psNextNode) 
        {
            if (strcmp(psCurrentNode->pcKey, pcKey) == 0) {
                pvOldValue = psCurrentNode->pvValue;
                psNextNode = psCurrentNode->psNextNode;
                /* If key to be freed is the first one */
                if (psCurrentNode->pcKey == oSymTable->psFirstNode->pcKey) {
                    oSymTable->psFirstNode = psNextNode;
                    free((char*)(psCurrentNode->pcKey));
                    free(psCurrentNode);
                    (oSymTable->length)--;
                    return (void*)pvOldValue;
                } else {
                    psPreviousNode->psNextNode = psNextNode;
                    free((char*)(psCurrentNode->pcKey));
                    free(psCurrentNode);
                    (oSymTable->length)--;
                    return (void*)pvOldValue;
                }
            }
            psNextNode = psCurrentNode->psNextNode;
            psPreviousNode = psCurrentNode;
        }
        return NULL;
}

void SymTable_map(SymTable_T oSymTable,
    void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra),
    const void *pvExtra) {
        struct Node* psCurrentNode;

        assert(oSymTable != NULL);
        assert(pfApply != NULL);

        for (psCurrentNode = oSymTable->psFirstNode;
                psCurrentNode != NULL;
                psCurrentNode = psCurrentNode->psNextNode) {
                    (*pfApply)((void*)psCurrentNode->pcKey, (void*)psCurrentNode->pvValue, (void*)pvExtra);
                }
    }
  