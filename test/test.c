#include "test.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *test_batch_name = NULL;
unsigned int tests_count = 0;
unsigned int tests_failed_count = 0;
char **failed_tests_outputs = NULL;

/**
 * @brief Create a new testing batch
 *
 * @param name : Testing batch name
 */
void new_test_batch(char *name) {
    tests_count = 0;
    tests_failed_count = 0;
    test_batch_name = name;
    failed_tests_outputs = NULL;
}

/**
 * @brief End the current testing batch and send results
 */
void end_test_batch() {
    if (tests_failed_count > 0) {
        printf("\033[0;31m[%s] %02d/%02d tests succeeded. %d failed :\n", test_batch_name,
               tests_count - tests_failed_count, tests_count, tests_failed_count);
        if (failed_tests_outputs) {
            // Print and free each output line
            for (int i = 0; i < tests_failed_count; i++) {
                printf("%s\n", failed_tests_outputs[i]);
                free(failed_tests_outputs[i]);
            }
            // Free the global array
            free(failed_tests_outputs);
        }
        // Set the output color back to default
        printf("\033[0;0m");
    } else {
        printf("\033[0;32m[%s] %02d/%02d tests succeeded.\033[0;0m\n", test_batch_name,
               tests_count - tests_failed_count, tests_count);
    }
}

/**
 * @brief Test if the given boolean is true in order to count how many tests succeeded
 * If b is false, an output is generated using the given parameters
 * You should use this function by calling the ASSERT_TRUE macro that is simplier to use
 *
 * @param b : boolean to test (expect to be true)
 * @param file_name : name of the file that contains the assert_true call
 * @param line_numer : number of line that that contains the assert_true call
 * @param function_name : name of the function that contains the assert_true call
 */
unsigned int assert_true(unsigned int b, char *file_name, int line_number, const char *function_name) {
    tests_count++;
    if (!b) {
        tests_failed_count++;

        // Expand the dynamic array of output lines
        failed_tests_outputs = realloc(failed_tests_outputs, tests_failed_count * sizeof(char *));

        // Create a temporary string to store the new output line
        char *s;
        asprintf(&s, "\t-> %s:%d in '%s'", file_name, line_number, function_name);

        // Allocate space to copy the new line
        failed_tests_outputs[tests_failed_count - 1] = malloc((strlen(s) + 1) * sizeof(char));
        strcpy(failed_tests_outputs[tests_failed_count - 1], s);

        // Free the temporary variable
        free(s);
    }
    return b;
}