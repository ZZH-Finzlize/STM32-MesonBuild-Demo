#include "test_frame.h"
#include "test_cases.h"
#include "bitmap_test_conf.h"
#include "util/bitmap/bitmap.h"

static int bitmap_basic_test(void *arg)
{
    (void) arg;
    int retv = -ENOMEM;

    uint8_t test_data[BITMAP_TEST_DATA_SIZE];

    std::srand(std::rand());

    FOR_ARRAY_I (test_data) {
        test_data[i] = std::rand() % 128;
    }

    bitmap_t bitmap(128, CONFIG_TEST_CASE_MEMPOOL);
    GOTO_IF(0 == bitmap.length(), bitmap_err);

    // testcase starts here
    retv = 0;

    FOR_ARRAY_I (test_data) {
        bitmap.save(test_data[i]);
    }

    FOR_ARRAY_I (test_data) {
        GOTO_IF_ZERO(bitmap.check(test_data[i]), error_exit);
    }

    FOR_ARRAY_I (test_data) {
        bitmap.drop(test_data[i]);
    }

    FOR_ARRAY_I (test_data) {
        GOTO_IF_NZERO(bitmap.check(test_data[i]), error_exit);
    }

bitmap_err:
    return retv;

error_exit:
    retv = -EINVAL;
    goto bitmap_err;
}

EXPORT_TEST_CASE(bitmap_basic_test);
