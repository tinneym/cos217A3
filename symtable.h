/* symtablelist.h
Author: Tinney Mak */

#include <stddef.h>
#ifndef SYMTABLE_INCLUDED
#define SYMTABLE_INCLUDED

/* A SymTable_T object is a collection of bindings which consist of a key and a value */
typedef struct SymTable* SymTable_T;

/* Returns new SymTable object that contains no bindings or NULL if insufficient
memory is available */
SymTable_T SymTable_new(void);

/* Frees all memory occupied by oSymTable */
void SymTable_free(SymTable_T oSymTable);

/* Returns the number of bindings in oSymTable */
size_t SymTable_getLength(SymTable_T oSymTable);

/* If oSymTable doesn't contain a binding with key pcKey, add a new binding to
oSymTable where the key is pcKey and the value is pvValue and return 1; otherwise,
if the binding already exists or there is insufficient memory, leave oSymTable unchanged 
and return 0 */
int SymTable_put(SymTable_T oSymTable, 
    const char *pcKey, const void *pvValue);

/* If oSymTable contains a binding with pcKey, replace the binding's value with pvValue
and return the old value; otherwise, leave oSymTable unchanged and return NULL */
void *SymTable_replace(SymTable_T oSymTable,
    const char *pcKey, const void *pvValue);

/* Return 1 if oSymTable contains a binding whose key is pcKey and 0 otherwise */
int SymTable_contains(SymTable_T oSymTable, const char *pcKey);

/* Return the value of the binding within oSymTable whose key is pcKey or NULL
if no such binding exists */
void *SymTable_get(SymTable_T oSymTable, const char *pcKey);

/* If oSymTable contains pcKey, remove that binding from the table and return the 
binding's value; otherwise, make no changes and return NULL */
void *SymTable_remove(SymTable_T oSymTable, const char *pcKey);

/* Apply function *pfApply to each binding in oSymTable, passing pvExtra as an extra 
parameter */
void SymTable_map(SymTable_T oSymTable,
    void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra),
    const void *pvExtra);

#endif