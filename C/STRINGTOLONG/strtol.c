#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int trimSpaces(const char *input) {
    int position = 0;
    while (isspace((unsigned char)input[position])) 
        position++;
    return position;
}

bool parseSignChar(const char c, int *position) {
    if (c != '-' && c != '+') 
        return false;
    (*position)++;
    return (c == '-');
}

void processHexPrefix(const char *input, int *position) {
    const char *hexPtr = input + *position;
    if (!(hexPtr[0] == '0' && (hexPtr[1] == 'x' || hexPtr[1] == 'X'))) 
        return;
    *position += 2;
    if (!isxdigit((unsigned char)input[*position])) 
        *position -= 2;
}

int detectBasePrefix(const char *input, int position) {
    if (input[position] == '0') {
        if (input[position + 1] == 'x' || input[position + 1] == 'X') 
            return 16;
        return 8;
    }
    return isdigit((unsigned char)input[position]) ? 10 : -1;
}

void determineRadix(const char *input, int *base, int *position) {
    if (*base == 0) 
        *base = detectBasePrefix(input, *position);
    if (*base == 1 || *base < 0 || *base > 36) {
        errno = EINVAL;
        return;
    }
    if (*base == 16) 
        processHexPrefix(input, position);
}

int getCharValue(int base, char c) {
    int digitValue = (int)INFINITY;
    if (isdigit((unsigned char)c)) 
        digitValue = c - '0';
    else if (isalpha((unsigned char)c)) 
        digitValue = tolower(c) - 'a' + 10;
    return (digitValue < base) ? digitValue : -1;
}

long int strtol(const char *str, char **endptr, int base) {
    errno = 0;
    int position = trimSpaces(str);
    bool isNegative = parseSignChar(str[position], &position);
    determineRadix(str, &base, &position);
    if (errno == EINVAL) {
        if (endptr) 
            *endptr = (char *)str;
        return 0;
    }
    long int result = 0;
    int startPos = position;
    for (int digit = 0; str[position] != '\0'; position++) {
        digit = getCharValue(base, str[position]);
        if (digit == -1) 
            break;
        if ((result > (LONG_MAX - digit) / base) || (result < (LONG_MIN + digit) / base)) 
            errno = ERANGE;
        result *= base;
        result += isNegative ? -digit : digit;
        if (endptr) 
            *endptr = (char *)(str + position + 1);
    }
    if (position == startPos && endptr) 
        *endptr = (char *)str;
    if (errno == ERANGE) 
        return isNegative ? LONG_MIN : LONG_MAX;
    return result;
}
