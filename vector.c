#include "vector.h"

#define INITIAL_SIZE 0
#define DECREASE_FACTOR 0.5
#define MIN_VEC_CAPACITY 2

/**
 * initializes the vector with default values
 * @param vector
 * @param elem_copy_func
 * @param elem_cmp_func
 * @param elem_free_func
 */
static void init_vec(vector *vector, vector_elem_cpy elem_copy_func,
                     vector_elem_cmp elem_cmp_func,
                     vector_elem_free elem_free_func) {
    vector->elem_free_func = elem_free_func;
    vector->elem_cmp_func = elem_cmp_func;
    vector->elem_copy_func = elem_copy_func;
    vector->capacity = VECTOR_INITIAL_CAP;
    vector->size = INITIAL_SIZE;
    vector->data = NULL;
}


vector *vector_alloc(vector_elem_cpy elem_copy_func,
                     vector_elem_cmp elem_cmp_func,
                     vector_elem_free elem_free_func) {
    vector *vec = NULL;

    // args check
    if (elem_copy_func == NULL
    || elem_cmp_func == NULL
    || elem_free_func == NULL) {
        return NULL;
    }

    // allocating memory for the vector instance
    vec = malloc(sizeof(vector));
    if (vec == NULL)
        exit(EXIT_FAILURE);

    // initialize vec's data
    init_vec(vec, elem_copy_func, elem_cmp_func, elem_free_func);
    vec->data = malloc(sizeof(void *) * VECTOR_INITIAL_CAP);
    if (vec->data == NULL) {
        free(vec);
        exit(EXIT_FAILURE);
    }
    return vec;
}

void vector_clear(vector *vector) {
    if (vector == NULL)
        return;

   for (int i = ((int) vector->size) - 1; i >= 0; i--) {
        vector_erase(vector, i);
    }
}

void vector_free(vector **p_vector) {
    if (p_vector != NULL && *p_vector != NULL) {
        vector_clear(*p_vector);
        free((*p_vector)->data);
        free(*p_vector);
        *p_vector = NULL;
    }
}

void *vector_at(const vector *vector, size_t ind) {
    if (vector == NULL || vector->size <= ind) {
        return NULL;
    }
    return vector->data[ind];
}

int vector_find(const vector *vector, const void *value) {
    if (vector != NULL && value != NULL) {
        for (size_t i = 0; i < vector->size; ++i) {
            if (vector->elem_cmp_func(vector->data[i], value))
                return (int) i;
        }
    }
    return -1;
}

double vector_get_load_factor(const vector *vector) {
    if (vector != NULL) {
        return ((double) vector->size / (double) vector->capacity);
    }
    return -1.0;
}

int vector_push_back(vector *vector, const void *value) {
    double load_factor;
    void **temp = NULL;
    size_t end_idx;

    if (vector == NULL || value == NULL)
        return 0;

    end_idx = vector->size;
    vector->data[end_idx] = vector->elem_copy_func(value);
    vector->size++;


    load_factor = vector_get_load_factor(vector);

    // resize the data if needed
    if (load_factor > VECTOR_MAX_LOAD_FACTOR) {
        temp = vector->data;
        temp = realloc(temp, sizeof(void *) *
        (vector->capacity * VECTOR_GROWTH_FACTOR));
        if (temp == NULL) {
            exit(EXIT_FAILURE);
        }
        vector->data = temp;
        vector->capacity = vector->capacity * VECTOR_GROWTH_FACTOR;
    }

    return 1;
}

int vector_erase(vector *vector, size_t ind) {
    double load_factor;
    void *temp;

    if (vector != NULL && vector->size > 0 && vector_at(vector, ind) != NULL) {
        size_t end_idx = vector->size - 1;

        // free the elem, and replace the last one in its place
        vector->elem_free_func(&(vector->data[ind]));
        vector->size--;
        vector->data[ind] = vector->data[end_idx];
        vector->data[end_idx] = NULL;

        load_factor = vector_get_load_factor(vector);

        // resize the data if needed
        if (load_factor < VECTOR_MIN_LOAD_FACTOR
        && vector->capacity >= MIN_VEC_CAPACITY) {
            size_t new_capacity =
                    (size_t)  ((double) vector->capacity * DECREASE_FACTOR);

            temp = vector->data;
            temp = realloc(vector->data, (sizeof(void *) * new_capacity));
            if (temp == NULL) {
                exit(EXIT_FAILURE);
            }
            vector->data = temp;
            vector->capacity = new_capacity;
        }
        return 1;


    }
    return 0;
}



