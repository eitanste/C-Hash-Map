#include "vector.h"
#include "hashmap.h"

#define INITIAL_SIZE 0
#define HASH_MAP_REDUCTION_FACTOR 0.5
#define MIN_CAPACITY    2

#define INSERT_ELEMENT     1
#define ERASE_ELEMENT   2

#define MAX(a, b) (((a) > (b)) ? (a) : (b))


static int init_hashmap(hashmap *hashmap, hash_func func, size_t capacity) {
    hashmap->size = 0;
    hashmap->capacity = capacity;
    hashmap->hash_func = func;
    
    hashmap->buckets = (vector**) calloc(sizeof(void *), hashmap->capacity);
    if (hashmap->buckets == NULL) {
        free(hashmap);
        exit(EXIT_FAILURE);
    }
    for (size_t i = 0; i < hashmap->capacity; ++i) {
        hashmap->buckets[i] = vector_alloc(pair_copy, pair_cmp, pair_free);
        if (hashmap->buckets[i] == NULL){
            return 0;
        }
    }
    for (size_t i = 0; i < hashmap->capacity; ++i) {
        if (hashmap->buckets[i] == NULL) {
            return 0;
        }
    }
    return 1;
}

static size_t hashmap_hash_func(const hashmap *hashmap, const_keyT key)
{
    size_t val = hashmap->hash_func(key);
    // capacity must be a degree of 2!
    return val & (hashmap->capacity - 1);
}

hashmap *hashmap_alloc(hash_func func) {
    hashmap *hashmap = NULL;

    if (func == NULL) {
        return NULL;
    }

    hashmap = malloc(sizeof(struct hashmap));
    if (hashmap == NULL)
        exit(EXIT_FAILURE);

    if (!init_hashmap(hashmap, func, HASH_MAP_INITIAL_CAP)){
        hashmap_clear(hashmap);
        free(hashmap);
        return NULL;
    }
    return hashmap;
}

static void hashmap_free_buckets(hashmap *hash_map) {
    for (size_t i = 0; i < hash_map->capacity; ++i) {
        vector_free(&(hash_map->buckets[i]));
    }
    free(hash_map->buckets);
    hash_map->buckets = NULL;
    hash_map->size = 0;
}

void hashmap_free(hashmap **p_hash_map) {
    if (p_hash_map == NULL || *p_hash_map == NULL) {
        return;
    }
    hashmap_free_buckets(*p_hash_map);
    free(*p_hash_map);
    *p_hash_map = NULL;
}


vector *hashmap_backup_elements(hashmap *hash_map){
    // create a new vector with all the stored pairs
    vector *bucket = NULL;
    vector *temp_vec = vector_alloc(pair_copy,pair_cmp,pair_free);

    for (size_t i = 0; i < hash_map->capacity; ++i) {
        bucket = hash_map->buckets[i];
        if (bucket != NULL && bucket->size > 0) {
            for (size_t j = 0; j < bucket->size; ++j) {
                if (!vector_push_back(temp_vec, bucket->data[j])){
                    vector_free(&temp_vec);
                    return NULL;
                }
            }
        }
    }
    return temp_vec;
}

static int hashmap_insert_pair(hashmap *hash_map, const pair *in_pair) {
    // init vars
    size_t hash_value;
    vector *bucket = NULL;

    // checks the args
    if (hash_map == NULL || in_pair == NULL)
        return 0;

    if (hashmap_at(hash_map, in_pair->key) != NULL) {
        // pair with this key already exists
        return 0;
    }

    hash_value = hashmap_hash_func(hash_map, in_pair->key);
    bucket = hash_map->buckets[hash_value];
    vector_push_back(bucket, in_pair);
    hash_map->size++;

    return 1;
}

static int hashmap_restore_elements(hashmap *hash_map,vector* temp_vec ) {
    for (size_t i = 0; i < temp_vec->size; ++i) {
        if (!hashmap_insert_pair(hash_map, vector_at(temp_vec, i)))
            return 0;
    }
    return 1;
}

static int hashmap_resize(hashmap *hash_map, int mode) {
    double load_factor = hashmap_get_load_factor(hash_map);
    double resize_factor = 0;
    double new_capacity;
    vector* temp_vec = NULL;

    if (load_factor == -1)
        return 0;

    // resize the hash map
    if (mode == INSERT_ELEMENT && load_factor > HASH_MAP_MAX_LOAD_FACTOR)
        resize_factor = HASH_MAP_GROWTH_FACTOR;
    else if (mode == ERASE_ELEMENT && load_factor < HASH_MAP_MIN_LOAD_FACTOR)
        resize_factor = HASH_MAP_REDUCTION_FACTOR;
    else
        return 1;

    if (resize_factor) {
        // save all pairs in temporary vector
        temp_vec =  hashmap_backup_elements(hash_map);
        if (temp_vec == NULL) {
            vector_free(&temp_vec);
            return 0;
        }

        hashmap_free_buckets(hash_map);

        new_capacity = MAX(resize_factor * (double) hash_map->capacity,
                           MIN_CAPACITY);

        init_hashmap(hash_map,hash_map->hash_func, (size_t) new_capacity);

        // add all pairs
        hashmap_restore_elements(hash_map,temp_vec);
        vector_free(&temp_vec);
    }

    return  1;
}


int hashmap_insert(hashmap *hash_map, const pair *in_pair) {
    if (!hashmap_insert_pair(hash_map, in_pair) ||
    !hashmap_resize(hash_map, INSERT_ELEMENT))
        return 0;

    return 1;
}

static pair *hashmap_pair_at(const hashmap *hash_map, const_keyT key,
                             size_t *hash, size_t *idx) {
    size_t hash_value;
    vector *bucket = NULL;

    if (hash_map == NULL || key == NULL)
        return NULL;

    hash_value = hashmap_hash_func(hash_map, key);

    bucket = hash_map->buckets[hash_value];
    if (bucket->size == 0)
        return NULL;

    for (size_t i = 0; i < bucket->size; ++i) {
        pair *p = (pair *) vector_at(bucket, i);

        if (p->key_cmp(p->key, key)) {
            *hash = hash_value;
            *idx = i;
            return p;
        }
    }
    return NULL;
}

valueT hashmap_at(const hashmap *hash_map, const_keyT key) {
    size_t hash;
    size_t idx;
    pair *p = hashmap_pair_at(hash_map, key, &hash, &idx);

    if (p == NULL)
        return NULL;

    return p->value;
}

int hashmap_contains_key(hashmap *hash_map, const_keyT key) {
    return hashmap_at(hash_map, key) != NULL;
}

int hashmap_contains_value(hashmap *hash_map, const_valueT value) {
    vector * bucket = NULL;

    if (hash_map ==NULL)
        return 0;

    for (size_t j = 0; j < hash_map->capacity; ++j) {
        bucket = hash_map->buckets[j];
        for (size_t i = 0; i < bucket->size; ++i) {
            pair *p = (pair *) vector_at(bucket, i);
            if (p->value_cmp(p->value, value)) {
                return 1;
            }
        }
    }
    return  0;
}

static int hashmap_erase_pair(hashmap *hash_map, const_keyT key) {
    pair* p = NULL;
    size_t hash;
    size_t idx;
    vector *bucket = NULL;

    p = hashmap_pair_at(hash_map, key, &hash, &idx);
    if (p == NULL)
        return 0;

    bucket = hash_map->buckets[hash];

    if (!vector_erase(bucket, idx))
        return 0;

    hash_map->size--;
    return 1;
}

int hashmap_erase(hashmap *hash_map, const_keyT key) {
    if (!hashmap_erase_pair(hash_map, key) ||
    !hashmap_resize(hash_map, ERASE_ELEMENT))
        return 0;

    return 1;
}

double hashmap_get_load_factor(const hashmap *hash_map) {
    if (hash_map != NULL && hash_map->capacity != 0) {
        return ((double) hash_map->size / (double) hash_map->capacity);
    }
    return -1.0;
}

void hashmap_clear(hashmap *hash_map) {
    vector *bucket = NULL;
    size_t del_count = 0;
    size_t i;
    int j;

    if (hash_map == NULL)
        return;

    for (i = 0; i < hash_map->capacity; ++i) {
        bucket = hash_map->buckets[i];
        for (j = (int) bucket->size - 1; j >= 0; --j) {
            hash_map->size -= bucket->size;
            del_count += bucket->size;
            vector_clear(bucket);
        }
    }

    for (i = 0; i < del_count; i++)
        hashmap_resize(hash_map, ERASE_ELEMENT);
}