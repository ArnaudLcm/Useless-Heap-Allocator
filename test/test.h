#ifndef __TESTS_H__
#define __TESTS_H__

// THIS VAR IS REDEFINED IN EACH TEST FILE IF A SPECIAL CLEANING FUNCTION IS NEEDED
#define TEST_CLEAN
#undef TEST_CLEAN

#define __ASSERT(b) assert_true(b, __FILE__, __LINE__, __func__)

#define ASSERT_TRUE(b) __ASSERT(b);
#define ASSERT_TRUE_FATAL(b) if(!__ASSERT(b)) { TEST_CLEAN; return; }

void new_test_batch(char* name);

unsigned int assert_true(unsigned int b, char* filename, int line_number, const char* function_name);

void end_test_batch();

#endif // __TESTS_H__
