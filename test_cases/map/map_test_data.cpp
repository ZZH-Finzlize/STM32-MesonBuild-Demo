#include <cstdlib>
#include "map_test_data.h"

test_data_t *generate_test_data(size_t size)
{
    test_data_t *test_data = new (mem_pool_t::testcase_pool) test_data_t[size];

    CHECK_PTR(test_data, NULL);

    FOR_I (size) {
        test_data_t *iter = &test_data[i];
        ITER_ARRAY (key_iter, iter->key) {
            *key_iter = std::rand() % 95 + 32;
        }
        iter->key[sizeof(iter->key) - 1] = '\0';

        iter->val = static_cast<map_t::value_t>(std::rand());
    }

    return test_data;
}
