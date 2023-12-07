#include "map_test_data.h"

static int map_duplcate_key_test(void* arg)
{
    (void) arg;
    const char* test_key = "test_key";

    int retv = 0;
    map_t* map = map_create_in_pool(31, bkdr_hash, CONFIG_TEST_CASE_MEMPOOLS);
    CHECK_PTR(map, -ENOMEM);

    retv = map_insert(map, test_key, 141516);
    GOTO_IF_NZERO(retv, clean_exit);

    map_value_t res_value = 0;

    retv = map_search(map, test_key, &res_value);
    GOTO_IF_NZERO(retv, clean_exit);
    RETURN_IF(res_value != 141516, -EINVAL);

    retv = map_insert(map, test_key, 141518);
    GOTO_IF_NZERO(retv, clean_exit);

    retv = map_search(map, test_key, &res_value);
    GOTO_IF_NZERO(retv, clean_exit);
    GOTO_IF(res_value != 141518, clean_exit);

clean_exit:
    map_delete(map);
    return retv;
}

EXPORT_TEST_CASE(map_duplcate_key_test);
