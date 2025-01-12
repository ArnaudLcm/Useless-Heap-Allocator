
#include <unistd.h>

#include "alloc.h"
#include "single_threaded.h"
#include "test.h"
#include "list_tests.h"

int main(int argc, char *argv[]) {
    new_test_batch("test_lists");
    test_operations_on_list_batch();
    end_test_batch();

    new_test_batch("test_single_threaded");
    test_single_threaded_batch();
    end_test_batch();
    return 0;
}