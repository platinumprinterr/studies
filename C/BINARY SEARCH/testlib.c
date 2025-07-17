#include <stdio.h>
#include <stdint.h>
#include <string.h>

typedef struct {
    int test_key;
    char name[20];
} test_t;

int compareStruct(const void *a, const void *b) {
    const test_t *left = (const test_t *)a;
    const test_t *right = (const test_t *)b;
    return (left->test_key > right->test_key) - (left->test_key < right->test_key);
}

int compareDouble(const void *a, const void *b) {
    return (*(double *)a > *(double *)b) - (*(double *)a < *(double *)b);
}

int compareInt(const void *a, const void *b) {
    return (*(int *)a > *(int *)b) - (*(int *)a < *(int *)b);
}

int compareChar(const void *a, const void *b) {
    return (*(char *)a > *(char *)b) - (*(char *)a < *(char *)b);
}

int compareStr(const void *a, const void *b) {
    return strcmp((const char *)a, (const char *)b);
}

void *bsearch(const void *key, const void *base, size_t num, size_t size,
              int (*compar)(const void *, const void *)) {
    while (num > 0) {
        size_t mid_index = num / 2;
        uint8_t *middle = (uint8_t *)base + (mid_index * size);
        int cmp_result = compar(key, middle);

        if (cmp_result == 0) {
            return (void *)middle;
        } else if (cmp_result > 0) {
            base = middle + size;
            num -= mid_index + 1;
        } else {
            num = mid_index;
        }
    }
    return NULL;
}

// Test function
int run_test(void *arr, int n, size_t size, void *key, void *expected,
             int (*compare)(const void *, const void *)) {
    void *item = bsearch(key, arr, n, size, compare);
    if (item != NULL && expected != NULL && compare(item, expected) == 0) {
        return 1;
    } else if (item == NULL && expected == NULL) {
        return 1;
    } else {
        return 0;
    }
}

int main() {
    int successful_tests = 0;

    // Integer tests
    successful_tests = 0;
    int arr_int[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    for (int i = 1; i <= 10; i++) {
        successful_tests += run_test(arr_int, 10, sizeof(int), &i, &arr_int[i - 1], compareInt);
    }
    printf("Successful tests for integers: %d\n", successful_tests);

    // Double tests
    successful_tests = 0;
    double arr_double[] = {1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 11.0};
    for (int i = 0; i < 10; i++) {
        double key_double = arr_double[i];
        successful_tests += run_test(arr_double, 10, sizeof(double), &key_double, &arr_double[i], compareDouble);
    }
    printf("Successful tests for doubles: %d\n", successful_tests);

    // Character tests
    successful_tests = 0;
    char arr_char[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j'};
    for (int i = 0; i < 10; i++) {
        char key_char = arr_char[i];
        successful_tests += run_test(arr_char, 10, sizeof(char), &key_char, &arr_char[i], compareChar);
    }
    printf("Successful tests for characters: %d\n", successful_tests);

    // Struct tests
    successful_tests = 0;
    test_t arr_struct[] = {{1, "Julian"}, {2, "John"}, {3, "Doe"}, {4, "Jane"}, {5, "Doe"},
                           {6, "John"}, {7, "Julian"}, {8, "Jane"}, {9, "Doe"}, {10, "John"}};
    for (int i = 0; i < 10; i++) {
        test_t key_struct = arr_struct[i];
        successful_tests += run_test(arr_struct, 10, sizeof(test_t), &key_struct, &arr_struct[i], compareStruct);
    }
    printf("Successful tests for structs: %d\n", successful_tests);

    return 0;
}
