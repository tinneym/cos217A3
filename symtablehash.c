/* symtablehash.c
Author: Tinney Mak */

#include <assert.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "symtable.h"

/* Array of bucket counts for hash table */
static const size_t auBucketCounts[] = {
    509, 1021, 2039, 4093, 8191, 16381, 32749, 65521
};

/* Variable for keeping track of bucket count array index*/
static size_t counter = 0;

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

/* Helper function that returns a hash code for pcKey that is between 0 and uBucketCount-1, inclusive.*/
static size_t SymTable_hash(const char *pcKey, size_t uBucketCount);

/* Helper function that inserts node psToInsert into the array that ppsSymNode points to where the index of the bucket 
equals hashCode. Returns 1 if successful and 0 if not.*/
static int SymTable_insert(struct Node** ppsSymNode, struct Node* psToInsert, size_t hashCode);

/* Helper function that, for oSymTable, goes into the array of buckets where the index equals hashCode and removes the 
node corresponding to pcKey. Returns 1 if successful and 0 if not.*/
static int SymTable_delete(SymTable_T oSymTable, const char *pcKey, size_t hashCode);

/* A SymTable is a structure that points to the first Node of the array of head nodes. */
struct SymTable {
    /* The address of the node pointing to the array of buckets */
    struct Node** ppsSymNode;

    /* The number of the bindings in the symbol table */
    size_t length;
};

SymTable_T SymTable_new(void) {
    SymTable_T oSymTable;

    oSymTable = (SymTable_T)calloc(1, sizeof(struct SymTable));
    if (oSymTable == NULL) {
        return NULL;
    }

    /* Allocates memory for array of buckets and initializes them to NULL */
    oSymTable->ppsSymNode = (struct Node**)calloc(auBucketCounts[0], sizeof(struct Node*));
    if (oSymTable->ppsSymNode == NULL) {
        free(oSymTable);
        return NULL;
    }

    oSymTable->length = 0;
    return oSymTable;
}

void SymTable_free(SymTable_T oSymTable) {
    struct Node* psCurrentNode;
    struct Node* psNextNode;
    size_t i = 0;  /* loop counter */

    assert(oSymTable != NULL);

    for (i = 0; i < auBucketCounts[counter]; i++) {
        for (psCurrentNode = oSymTable->ppsSymNode[i]; 
                psCurrentNode != NULL; 
                psCurrentNode = psNextNode) {
                psNextNode = psCurrentNode->psNextNode;
                free((char*)(psCurrentNode->pcKey));
                free(psCurrentNode);
        }
    }

    free(oSymTable->ppsSymNode);
    free(oSymTable);
}

size_t SymTable_getLength(SymTable_T oSymTable) {
    assert(oSymTable != NULL);
    return oSymTable->length;
}

int SymTable_put(SymTable_T oSymTable, 
    const char *pcKey, const void *pvValue) {
        struct Node* psNewNode;
        size_t hashCode; /* hash code for the bucket the binding will be put in */
        int check = 1; /* check variable for insufficient memory case */

        assert(oSymTable != NULL);
        assert(pcKey != NULL);

        if (SymTable_contains(oSymTable, pcKey) == 1) {
            return 0;
        }

        /* Expand if number of bindings is greater than number of buckets */
        if (SymTable_getLength(oSymTable) > auBucketCounts[counter]) {
            struct Node** psNewMem;
            struct Node* psCurrentNode;
            size_t newHashCode;
            size_t i = 0;  /* loop counter */

            counter++;

            /* Allocate memory for new bucket array */
            psNewMem = (struct Node**)calloc(auBucketCounts[counter], sizeof(struct Node*));
            if (psNewMem == NULL) {
                counter--;
                check = 0;
            }
            
            /* If there is sufficient memory, continue into loop */
            if (check == 1) {
                /* Loop to rehash keys and "move" them into the new bucket array */
                for (i = 0; i < auBucketCounts[counter]; i++) {
                    for (psCurrentNode = oSymTable->ppsSymNode[i];
                            psCurrentNode != NULL; 
                            psCurrentNode = psCurrentNode->psNextNode) {
                        
                    /* Calculate past and new hash code */
                    hashCode = SymTable_hash(psCurrentNode->pcKey, auBucketCounts[counter-1]);
                    newHashCode = SymTable_hash(psCurrentNode->pcKey, auBucketCounts[counter]);
                    
                    /* Insert node in new bucket array and delete from old bucket array */
                    SymTable_insert(psNewMem, psCurrentNode, newHashCode);
                    SymTable_delete(oSymTable, pcKey, hashCode);
                    }
                }

                free(oSymTable->ppsSymNode);
                oSymTable->ppsSymNode = psNewMem;
            }
        } 
        
        /* Allocate space for new node to be inserted */
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

        /* Hash key and put in corresponding bucket */
        hashCode = SymTable_hash(pcKey, auBucketCounts[counter]);

        if (oSymTable->ppsSymNode == NULL) {
            return 0;
        }

        /* If bucket is empty */
        if (oSymTable->ppsSymNode[hashCode] == NULL) {
            oSymTable->ppsSymNode[hashCode] = psNewNode;
            psNewNode->psNextNode = NULL;
        } else {
            psNewNode->psNextNode = oSymTable->ppsSymNode[hashCode];
            oSymTable->ppsSymNode[hashCode] = psNewNode;
        }

        (oSymTable->length)++;
        return 1;
}

void *SymTable_replace(SymTable_T oSymTable, 
    const char *pcKey, const void *pvValue) {
        void* pvOldValue;
        struct Node* psCurrentNode;
        size_t hashCode; /* hash code for the bucket the binding will be put in */

        assert(oSymTable != NULL);
        assert(pcKey != NULL);

        if (SymTable_contains(oSymTable, pcKey) == 0) {
            return NULL;
        }

        hashCode = SymTable_hash(pcKey, auBucketCounts[counter]);

        for (psCurrentNode = oSymTable->ppsSymNode[hashCode]; 
                psCurrentNode != NULL; 
                psCurrentNode = psCurrentNode->psNextNode) {
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
    size_t hashCode; /* hash code for the bucket the binding will be put in */

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    hashCode = SymTable_hash(pcKey, auBucketCounts[counter]);

    if (oSymTable->ppsSymNode[hashCode] == NULL) {
        return 0;
    }

    for (psCurrentNode = oSymTable->ppsSymNode[hashCode]; 
            psCurrentNode != NULL; 
            psCurrentNode = psCurrentNode->psNextNode) {
        if (strcmp(psCurrentNode->pcKey, pcKey) == 0) {
                return 1;
            }
        }

    return 0;
}

void *SymTable_get(SymTable_T oSymTable, const char *pcKey) {
    struct Node* psCurrentNode;
    size_t hashCode; /* hash code for the bucket the binding will be put in */

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    if (SymTable_contains(oSymTable, pcKey) == 0) {
            return NULL;
        }
    
    hashCode = SymTable_hash(pcKey, auBucketCounts[counter]);

    for (psCurrentNode = oSymTable->ppsSymNode[hashCode]; 
            psCurrentNode != NULL; 
            psCurrentNode = psCurrentNode->psNextNode) {
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
    size_t hashCode; /* hash code for the bucket the binding will be put in */

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    if (SymTable_contains(oSymTable, pcKey) == 0) {
            return NULL;
        }

    hashCode = SymTable_hash(pcKey, auBucketCounts[counter]);

    psPreviousNode = oSymTable->ppsSymNode[hashCode];

    for (psCurrentNode = oSymTable->ppsSymNode[hashCode]; 
            psCurrentNode != NULL; 
            psCurrentNode = psCurrentNode->psNextNode) {
        /* If key matches */
        if (strcmp(psCurrentNode->pcKey, pcKey) == 0) {
                pvOldValue = psCurrentNode->pvValue;
                psNextNode = psCurrentNode->psNextNode;
                /* If key to be freed is the first one */
                if (psCurrentNode->pcKey == (oSymTable->ppsSymNode[hashCode])->pcKey) {
                    oSymTable->ppsSymNode[hashCode] = psNextNode;
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
        size_t i = 0;  /* loop counter */

        assert(oSymTable != NULL);
        assert(pfApply != NULL);

        for (i = 0; i < auBucketCounts[counter]; i++) {
            for (psCurrentNode = oSymTable->ppsSymNode[i]; 
                    psCurrentNode != NULL; 
                    psCurrentNode = psCurrentNode->psNextNode) {
                (*pfApply)((void*)psCurrentNode->pcKey, (void*)psCurrentNode->pvValue, (void*)pvExtra);
            }
        }
    }

/* Helper insert function */
int SymTable_insert(struct Node** ppsSymNode, struct Node* psToInsert, size_t hashCode) {
    assert(ppsSymNode != NULL);
    assert(psToInsert != NULL);

    if (ppsSymNode[hashCode] == NULL) {
        ppsSymNode[hashCode] = psToInsert;
        psToInsert->psNextNode = NULL;
        return 1;
    } else {
        psToInsert->psNextNode = ppsSymNode[hashCode];
        ppsSymNode[hashCode] = psToInsert;
        return 1;
    }
}

/* Helper delete function without freeing Node */
int SymTable_delete(SymTable_T oSymTable, const char *pcKey, size_t hashCode) {
    struct Node* psCurrentNode;
    struct Node* psNextNode;
    struct Node* psPreviousNode;

    assert(pcKey != NULL);

    for (psCurrentNode = oSymTable->ppsSymNode[hashCode]; 
            psCurrentNode != NULL; 
            psCurrentNode = psCurrentNode->psNextNode) {
        /* If key matches */
        if (strcmp(psCurrentNode->pcKey, pcKey) == 0) {
                psNextNode = psCurrentNode->psNextNode;
                /* If key to be deleted is the first one */
                if (psCurrentNode->pcKey == (oSymTable->ppsSymNode[hashCode])->pcKey) {
                    oSymTable->ppsSymNode[hashCode] = psNextNode;
                    (oSymTable->length)--;
                    return 1;
                } else {
                    psPreviousNode = psCurrentNode;
                    psPreviousNode->psNextNode = psNextNode;
                    (oSymTable->length)--;
                    return 1;
                }
            }
            psNextNode = psCurrentNode->psNextNode;
            psPreviousNode = psCurrentNode;
        }
    return 0;
}

/* Helper hash function */
size_t SymTable_hash(const char *pcKey, size_t uBucketCount)
{
   const size_t HASH_MULTIPLIER = 65599;
   size_t u;
   size_t uHash = 0;

   assert(pcKey != NULL);

   for (u = 0; pcKey[u] != '\0'; u++)
      uHash = uHash * HASH_MULTIPLIER + (size_t)pcKey[u];

   return uHash % uBucketCount;
}
  