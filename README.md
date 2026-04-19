# cg_hm
A generic hash map for C (cghm). Keys and values are `void *` — the map owns neither. You supply a hash function at init time; two are provided out of the box for strings and integers. Collisions are resolved with open addressing and linear probing. Tombstone deletion keeps lookups correct after removal. The map resizes automatically at 75% load. *Might add more hash functions and refactor open addressing to quadratic or something else later.

Part of [CongruentC](https://github.com/CongruentC).

---

## Usage
Drop `cg_hm.h` into your project. In exactly one translation unit, define the implementation macro before including:

```c
#define CGHM_IMPLEMENTATION
#include "cg_hm.h"
```

Every other file includes it bare:

```c
#include "cg_hm.h"
```

No build system changes are necessary, nor is linking against a separate library.

---

## Example

```c
#define CGHM_IMPLEMENTATION
#include "cg_hm.h"
#include <stdio.h>

int main(void) {
    cghm map = cghm_init();
    map.hash_fn = cghm_hash_fnv;

    int age = 23;
    cghm_insert(&map, "alice", &age);

    int *result = cghm_get(&map, "alice");
    printf("alice: %d\n", *result);

    cghm_delete(&map, "alice");
    cghm_free(&map);
    return 0;
}
```

Output:
```
alice: 23
```

---

## API

| Function | Description |
|---|---|
| `cghm_init()` | Initialize an empty map |
| `cghm_insert(map, key, value)` | Insert or update a key-value pair |
| `cghm_get(map, key)` | Retrieve value by key, or NULL |
| `cghm_delete(map, key)` | Delete entry by key |
| `cghm_find(map, key)` | Return index of key or empty slot |
| `cghm_size(map)` | Return number of entries |
| `cghm_free(map)` | Free allocated memory |
| `cghm_hash_fnv(key)` | FNV-1a hash for string keys |
| `cghm_hash_int(key)` | FNV-1a hash for int keys |

---

## License
MIT
