#include "map_test_data.h"

static int map_basic_test(void *arg)
{
    (void) arg;

    int retv = -ENOMEM;

    test_data_t *test_data = generate_test_data(TEST_DATA_SIZE);
    CHECK_PTR(test_data, retv);

    map_t map(31, bkdr_hash, CONFIG_TEST_CASE_MEMPOOL);
    GOTO_IF(0 == map.length(), map_err);

    retv = 0;
    srand(rand());

    FOR_I (TEST_DATA_SIZE) {
        test_data_t *iter = &test_data[i];
        retv = map.insert(iter->key, iter->val);
        GOTO_IF_NZERO(retv, clean_exit);
    }

    FOR_I (TEST_DATA_SIZE) {
        test_data_t *iter = &test_data[i];
        map_t::value_t search_res = 0;
        retv = map.search(iter->key, &search_res);
        GOTO_IF_NZERO(retv, clean_exit);
        GOTO_IF(search_res != iter->val, clean_exit);
    }

clean_exit:
map_err:
    release_test_data(test_data);
    return retv;
}

EXPORT_TEST_CASE(map_basic_test);
