#include "string.h"

void *memset(void *dest, int c, size_t n) {
    unsigned char *d = dest;

    while (n--) {
        *d++ = (unsigned char)c;
    }

    return dest;
}

void *memcpy(void *dest, const void *src, size_t n) {
    unsigned char *d = dest;
    const unsigned char *s = src;

    while (n--) {
        *d++ = *s++;
    }

    return dest;
}

void *memmove(void *dest, const void *src, size_t n) {
    unsigned char *d = dest;
    const unsigned char *s = src;

    if (d < s) {
        while (n--) {
            *d++ = *s++;
        }
    } else {
        d += n;
        s += n;

        while (n--) {
            *--d = *--s;
        }
    }

    return dest;
}

int memcmp(const void *a, const void *b, size_t n) {
    const unsigned char *p1 = a;
    const unsigned char *p2 = b;

    while (n--) {
        if (*p1 != *p2) {
            return *p1 - *p2;
        }

        p1++;
        p2++;
    }

    return 0;
}

size_t strlen(const char *str) {
    size_t len = 0;

    while (str[len]) {
        len++;
    }

    return len;
}

char *strcpy(char *dest, const char *src) {
    char *ret = dest;

    while ((*dest++ = *src++))
        ;

    return ret;
}

char *strncpy(char *dest, const char *src, size_t n) {
    size_t i;

    for (i = 0; i < n && src[i]; i++) {
        dest[i] = src[i];
    }

    for (; i < n; i++) {
        dest[i] = '\0';
    }

    return dest;
}

int strcmp(const char *a, const char *b) {
    while (*a && (*a == *b)) {
        a++;
        b++;
    }

    return (unsigned char)*a - (unsigned char)*b;
}

int strncmp(const char *a, const char *b, size_t n) {
    while (n && *a && (*a == *b)) {
        a++;
        b++;
        n--;
    }

    if (n == 0) {
        return 0;
    }

    return (unsigned char)*a - (unsigned char)*b;
}

char *strcat(char *dest, const char *src) {
    char *ret = dest;

    while (*dest) {
        dest++;
    }

    while ((*dest++ = *src++))
        ;

    return ret;
}

char *strncat(char *dest, const char *src, size_t n) {
    char *ret = dest;

    while (*dest) {
        dest++;
    }

    while (n-- && *src) {
        *dest++ = *src++;
    }

    *dest = '\0';

    return ret;
}

char *strchr(const char *str, int c) {
    while (*str) {
        if (*str == (char)c) {
            return (char *)str;
        }

        str++;
    }

    if (c == '\0') {
        return (char *)str;
    }

    return 0;
}
