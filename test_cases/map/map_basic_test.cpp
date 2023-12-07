#include "map_test_data.h"

static int map_basic_test(void *arg)
{
    (void) arg;

    int retv = -ENOMEM;

    test_data_t *test_data = generate_test_data(TEST_DATA_SIZE);
    CHECK_PTR_GOTO(test_data, data_err);

    map_t *map = map_create_in_pool(31, bkdr_hash, CONFIG_TEST_CASE_MEMPOOLS);
    CHECK_PTR_GOTO(map, map_err);

    retv = 0;
    srand(rand());

    FOR_I (TEST_DATA_SIZE) {
        test_data_t *iter = &test_data[i];
        retv = map_insert(map, iter->key, iter->val);
        GOTO_IF_NZERO(retv, clean_exit);
    }

    FOR_I (TEST_DATA_SIZE) {
        test_data_t *iter = &test_data[i];
        map_value_t search_res = 0;
        retv = map_search(map, iter->key, &search_res);
        GOTO_IF_NZERO(retv, clean_exit);
        GOTO_IF(search_res != iter->val, clean_exit);
    }

clean_exit:
    map_delete(map);
map_err:
    release_test_data(test_data);
data_err:
    return retv;
}

EXPORT_TEST_CASE(map_basic_test);
