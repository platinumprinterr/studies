#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "rand_malloc.h"

#define INIT_CAPACITY 16
#define BINARY_SUM_EXTRA_PADDING 2
#define MEMORY_ERROR(context) fprintf(stderr, "Memory allocation error during %s\n", context)

void cleanup_resources(char **binaryNumbers, int binaryCount, char *binarySum) {
    if (binaryNumbers) {
        for (int i = 0; i < binaryCount; i++)
            free(binaryNumbers[i]);
        free(binaryNumbers);
    }
    if (binarySum) free(binarySum);
}

char *getLine(int *allocationErrorFlag) {
    int capacity = INIT_CAPACITY;
    char *lineBuffer = (char *)rand_malloc(capacity);
    if (!lineBuffer) {
        MEMORY_ERROR("input line allocation");
        *allocationErrorFlag = 1;
        return NULL;
    }

    int length = 0, ch;
    while ((ch = getchar()) != EOF) {
        if (ch == '\n') break;

        if (length >= capacity - 1) {
            capacity *= 2;
            char *tempBuffer = (char *)rand_realloc(lineBuffer, capacity);
            if (!tempBuffer) {
                free(lineBuffer);
                MEMORY_ERROR("line buffer expansion");
                *allocationErrorFlag = 1;
                return NULL;
            }
            lineBuffer = tempBuffer;
        }

        lineBuffer[length++] = (char)ch;
    }

    if (ch == EOF && length == 0) {
        free(lineBuffer);
        return NULL;
    }

    lineBuffer[length] = '\0';
    return lineBuffer;
}

char *trimWhitespace(char *str) {
    while (isspace((unsigned char)*str)) str++;
    if (*str == '\0') return str;

    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';

    return str;
}

int isValidBinary(const char *str) {
    while (*str) {
        if (*str != '0' && *str != '1') return 0;
        str++;
    }
    return 1;
}

char *stripLeadingZeros(char *str) {
    while (*str == '0') str++;
    return (*str) ? str : "0";
}

void reverse(char *str) {
    int len = strlen(str);
    for (int i = 0; i < len / 2; i++) {
        char tmp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = tmp;
    }
}

char *addBinaryStrings(const char *a, const char *b) {
    int lenA = strlen(a);
    int lenB = strlen(b);
    int maxLen = (lenA > lenB ? lenA : lenB) + BINARY_SUM_EXTRA_PADDING;
    char *result = (char *)rand_malloc(maxLen);
    if (!result) {
        MEMORY_ERROR("binary addition result allocation");
        return NULL;
    }

    int i = lenA - 1, j = lenB - 1, k = 0, carry = 0;
    while (i >= 0 || j >= 0 || carry) {
        int bitA = (i >= 0) ? a[i--] - '0' : 0;
        int bitB = (j >= 0) ? b[j--] - '0' : 0;
        int sum = bitA + bitB + carry;
        result[k++] = (sum % 2) + '0';
        carry = sum / 2;
    }

    result[k] = '\0';
    reverse(result);
    return result;
}

int main() {
    char *inputLine;
    char **binaryNumbers = NULL;
    int binaryCount = 0;
    char *binarySum = NULL;
    int allocationErrorFlag = 0;

    while ((inputLine = getLine(&allocationErrorFlag)) != NULL) {
        if (allocationErrorFlag) {
            cleanup_resources(binaryNumbers, binaryCount, binarySum);
            return 1;
        }

        char *trimmed = trimWhitespace(inputLine);
        if (*trimmed == '\0') {
            free(inputLine);
            continue;
        }

        if (!isValidBinary(trimmed)) {
            fprintf(stderr, "Error: Invalid input\n");
            free(inputLine);
            cleanup_resources(binaryNumbers, binaryCount, binarySum);
            return 1;
        }

        char *cleanedBinary = stripLeadingZeros(trimmed);
        char *copiedBinary = strdup(cleanedBinary);
        if (!copiedBinary) {
            MEMORY_ERROR("copying cleaned binary string");
            free(inputLine);
            cleanup_resources(binaryNumbers, binaryCount, binarySum);
            return 1;
        }

        char **tempArray = (char **)rand_realloc(binaryNumbers, (binaryCount + 1) * sizeof(char *));
        if (!tempArray) {
            MEMORY_ERROR("expanding binary numbers array");
            free(copiedBinary);
            free(inputLine);
            cleanup_resources(binaryNumbers, binaryCount, binarySum);
            return 1;
        }

        binaryNumbers = tempArray;
        binaryNumbers[binaryCount++] = copiedBinary;

        if (!binarySum) {
            binarySum = strdup(copiedBinary);
            if (!binarySum) {
                MEMORY_ERROR("initializing sum");
                free(inputLine);
                cleanup_resources(binaryNumbers, binaryCount, binarySum);
                return 1;
            }
        } else {
            char *newSum = addBinaryStrings(binarySum, copiedBinary);
            if (!newSum) {
                free(inputLine);
                cleanup_resources(binaryNumbers, binaryCount, binarySum);
                return 1;
            }
            free(binarySum);
            binarySum = newSum;
        }

        free(inputLine);
    }

    if (allocationErrorFlag) {
        cleanup_resources(binaryNumbers, binaryCount, binarySum);
        return 1;
    }

    if (binaryCount == 0) {
        fprintf(stderr, "Error: No valid input\n");
        return 1;
    }

    printf("Sum:\n%s\n", binarySum);
    printf("Input numbers:\n");
    for (int i = 0; i < binaryCount; i++) {
        printf("%s\n", binaryNumbers[i]);
    }

    cleanup_resources(binaryNumbers, binaryCount, binarySum);
    return 0;
}
