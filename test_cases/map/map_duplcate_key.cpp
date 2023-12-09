#include "map_test_data.h"

static int map_duplcate_key_test(void* arg)
{
    (void) arg;
    const char* test_key = "test_key";
    map_t::value_t res_value = 0;

    int retv = -ENOMEM;
    map_t map(31, bkdr_hash, CONFIG_TEST_CASE_MEMPOOL);
    GOTO_IF(0 == map.length(), clean_exit);

    retv = map.insert(test_key, 141516);
    GOTO_IF_NZERO(retv, clean_exit);

    retv = map.search(test_key, &res_value);
    GOTO_IF_NZERO(retv, clean_exit);
    RETURN_IF(res_value != 141516, -EINVAL);

    retv = map.insert(test_key, 141518);
    GOTO_IF_NZERO(retv, clean_exit);

    retv = map.search(test_key, &res_value);
    GOTO_IF_NZERO(retv, clean_exit);
    GOTO_IF(res_value != 141518, clean_exit);

clean_exit:
    return retv;
}

EXPORT_TEST_CASE(map_duplcate_key_test);
