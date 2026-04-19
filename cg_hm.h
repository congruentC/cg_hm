// cg_hm.h

#ifndef cghm_H
#define cghm_H
#define CGSV_IMPLEMENTATION
#include <stddef.h>


#define CAPACITY 1024
#define FNV_OFFSET_BASIS 0xcbf29ce484222325
#define FNV_PRIME 0x00000100000001b3
typedef struct{
    void *key;
    void *value;
    // 0 - empty, never used, stop probing,
    // 1 - occupied, has key/value,
    // 2 - tombstone, was deleted, keep probing past this
    int occupied;
}Entry;

typedef struct{
    Entry *arr;
    int capacity;
    int filled;

    size_t (*hash_fn)(void *key);

}cghm;

// ------------ Hash Functions ----------------

// Helper function for the fnv and int hash functions
size_t cghm_hash_bytes(void *data, size_t len);

size_t cghm_hash_fnv(void *key);

size_t cghm_hash_int(void *key);



// ------------- Hash Operations -------------

// Initializes an empty hash map with initial
// capacity of CAPACITY.
cghm cghm_init();

// Inserts the key-value pair into the hash map.
// Returns 1 for inserted, 0 for updated value
// if key already exists, and -1 for error.
int cghm_insert(cghm *hash_map, void *key, void *value);

// Deletes the entry for the specified key from the map.
// Returns 1 on success, 0 otherwise.
int cghm_delete(cghm *hash_map, void *key);

// Returns the index of either the found key or
// the empty slot it lands on. Can use occupied field
// to check if key was found or empty slot.
int cghm_find(cghm *hash_map, void *key);

// Returns the total number of key-value pairs
// currently in the map.
int cghm_size(cghm *hash_map);

// Retrieve the value associated with a specific key.
// Returns NULL if the key is not found
void *cghm_get(cghm *hash_map, void *key);

// Deallocates the heap allocated entry array in the hash map.
void cghm_free(cghm *hash_map);

#ifdef CGHM_IMPLEMENTATION


// -------------------------------------------

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

cghm cghm_init(){
    cghm hash_map;
    hash_map.capacity = CAPACITY;
    hash_map.arr = calloc(hash_map.capacity, sizeof(Entry));
    hash_map.filled = 0;
    return hash_map;
}

size_t cghm_hash_bytes(void *data, size_t len){
    uint8_t *byte_key = (uint8_t *)data;
    uint64_t hash = FNV_OFFSET_BASIS;

    for (size_t i = 0; i < len; i++){
       hash = hash ^ byte_key[i];
       hash = hash * FNV_PRIME;
   }
   return hash;
}

size_t cghm_hash_fnv(void *key){
    return cghm_hash_bytes(key, strlen(key));
}

size_t cghm_hash_int(void *key){
   return cghm_hash_bytes(key, sizeof(int));
}


static void cghm_resize(cghm *hash_map){
    Entry *new_arr = calloc(hash_map->capacity * 2, sizeof(Entry));
    // insert old key values into new arr
    Entry *old_arr = hash_map->arr;
    int old_cap = hash_map->capacity;
    hash_map->arr = new_arr;
    hash_map->capacity *= 2;
    for (size_t i = 0; i < old_cap; i++){
        if (old_arr[i].occupied == 1){
            cghm_insert(hash_map, old_arr[i].key, old_arr[i].value);
        }
    }
    free(old_arr);
}

// ------------- Hash Operations -------------

int cghm_find(cghm *hash_map, void *key){
    int index = -1;

    size_t hash = hash_map->hash_fn(key);
    hash = hash % hash_map->capacity;
    // probing - linear
    int first = 0;
    size_t first_tombstone = 0;
    while ((hash_map->arr[hash].occupied == 1 && hash_map->arr[hash].key != key) || hash_map->arr[hash].occupied == 2){
        if (hash_map->arr[hash].occupied == 2 && first != 1){
            first_tombstone = hash;
            first = 1;
        }
        hash = (hash + 1) % hash_map->capacity;
    }

    if (hash_map->arr[hash].key != key && first == 1){
        return first_tombstone;
    }
    return hash;
}

int cghm_insert(cghm *hash_map, void *key, void *value){
    int result = -1;

    int hash = cghm_find(hash_map, key);

    if (hash_map->arr[hash].occupied == 1){ // update
        hash_map->arr[hash].value = value;
        result = 0;

    }
    else if (hash_map->arr[hash].occupied == 0 || hash_map->arr[hash].occupied == 2){ // fresh
        hash_map->arr[hash].occupied = 1;
        hash_map->arr[hash].key = key;
        hash_map->arr[hash].value = value;
        hash_map->filled++;
        result = 1;
    }

    if ((float)hash_map->filled / hash_map->capacity > 0.75f){
        cghm_resize(hash_map);
    }

    return result;
}

int cghm_size(cghm *hash_map){
    return (int)hash_map->filled;
}

void *cghm_get(cghm *hash_map, void *key){
    void *value = NULL;

    int index = cghm_find(hash_map, key);
    if (hash_map->arr[index].occupied == 1){
        value = hash_map->arr[index].value;
    }

    return value;
}

int cghm_delete(cghm *hash_map, void *key){
    int result = 0;

    int index = cghm_find(hash_map, key);
    if (hash_map->arr[index].occupied != 0){
        hash_map->arr[index].occupied = 2;
        hash_map->arr[index].value = NULL;
        hash_map->arr[index].key = NULL;
        hash_map->filled--;
        result = 1;
    }

    return result;
}

void cghm_free(cghm *hash_map){
    free(hash_map->arr);
}

#endif // CGHM_IMPLEMENTATION
#endif // CGHM_H
