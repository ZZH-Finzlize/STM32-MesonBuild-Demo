#include "map_test_data.h"

test_data_t *generate_test_data(uint32_t size)
{
    test_data_t *test_data = (test_data_t *) memAlloc(
        sizeof(test_data_t) * size, CONFIG_TEST_CASE_MEMPOOLS);

    CHECK_PTR(test_data, NULL);

    FOR_I (size) {
        test_data_t *iter = &test_data[i];
        ITER_ARRAY (key_iter, iter->key) {
            *key_iter = rand() % 95 + 32;
        }
        iter->key[sizeof(iter->key) - 1] = '\0';

        iter->val = (map_value_t) rand();
    }

    return test_data;
}
