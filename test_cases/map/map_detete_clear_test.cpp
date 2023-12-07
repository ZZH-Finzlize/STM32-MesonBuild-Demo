#include "map_test_data.h"

static int map_delete_test(void *arg)
{
    (void) arg;

    int retv = -ENOMEM;

    test_data_t *test_data = generate_test_data(TEST_DATA_SIZE);
    CHECK_PTR_GOTO(test_data, data_err);

    map_t *map = map_create_in_pool(31, bkdr_hash, CONFIG_TEST_CASE_MEMPOOLS);
    CHECK_PTR_GOTO(map, map_err);

    retv = 0;

    FOR_I (TEST_DATA_SIZE) {
        test_data_t *iter = &test_data[i];
        retv = map_insert(map, iter->key, iter->val);
        GOTO_IF_NZERO(retv, clean_exit);
    }

    FOR_I (TEST_DATA_SIZE) {
        test_data_t *iter = &test_data[i];
        retv = map_remove(map, iter->key);
        GOTO_IF_NZERO(retv, clean_exit);
    }

clean_exit:
    map_delete(map);
map_err:
    release_test_data(test_data);
data_err:

    return retv;
}

EXPORT_TEST_CASE(map_delete_test);
