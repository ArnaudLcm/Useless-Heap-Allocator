
#include <unistd.h>

#include "alloc.h"
#include "single_threaded.h"
#include "test.h"
int main(int argc, char *argv[]) {
    new_test_batch("test_single_threaded");
    test_single_threaded_batch();
    end_test_batch();
    return 0;
}