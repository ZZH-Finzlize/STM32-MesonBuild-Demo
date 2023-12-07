#include "test_frame.h"
#include "test_cases.h"
#include "bitmap_test_conf.h"
#include "util/bitmap/bitmap.h"

static int bitmap_basic_test(void *arg)
{
    (void) arg;
    int retv = -ENOMEM;

    uint8_t test_data[BITMAP_TEST_DATA_SIZE];

    srand(rand());

    FOR_ARRAY_I (test_data) {
        test_data[i] = rand() % 128;
    }

    bitmap_t *bitmap = bitmap_create(128);
    CHECK_PTR_GOTO(bitmap, bitmap_err);

    // testcase starts here
    retv = 0;

    FOR_ARRAY_I (test_data) {
        bitmap_save(bitmap, test_data[i]);
    }

    FOR_ARRAY_I (test_data) {
        GOTO_IF_ZERO(bitmap_check(bitmap, test_data[i]), error_exit);
    }

    FOR_ARRAY_I (test_data) {
        bitmap_drop(bitmap, test_data[i]);
    }

    FOR_ARRAY_I (test_data) {
        GOTO_IF_NZERO(bitmap_check(bitmap, test_data[i]), error_exit);
    }

clean_exit:
    bitmap_delete(bitmap);
bitmap_err:
    return retv;

error_exit:
    retv = -EINVAL;
    goto clean_exit;
}

EXPORT_TEST_CASE(bitmap_basic_test);
