
#include <stdlib.h>
#include <unistd.h>

#include "log.h"
#include "multi_threaded.h"
#include "single_threaded.h"
#include "test.h"
int main(int argc, char *argv[]) {
    void *current_break = (void *)sbrk(0);  // Cast the return value to void*
    if (current_break == (void *)-1) {
        return 1;
    }
    log_debug("Current end of data section (program break): %p\n", current_break);

    void *memory = malloc(1024);  // Allocate 1 KB

    void *new_break = (void *)sbrk(0);
    log_debug("Program break after malloc: %p\n", new_break);

    new_test_batch("test_single_threaded");
    test_single_threaded_batch();
    end_test_batch();

    new_test_batch("test_multi_threaded");
    test_multi_threaded_batch();
    end_test_batch();
    return 0;
}