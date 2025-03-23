#ifndef STRING_H
#define STRING_H

#include "stdint.h"
#include "stddef.h"

// String comparing
int32_t strcmp(const int8_t * s1, const int8_t * s2);

// String copying
int8_t * strcpy(int8_t * dest, const int8_t * src);

// String length
size_t strlen(const int8_t * str);

// Memory operations
void * memcpy(void * dest, const void * src, size_t n);
void * memset(void * ptr, uint8_t value, size_t n);
int32_t memcmp(const void * s1, const void * s2, size_t n);

// Character operations
int8_t toupper(int8_t c);
int8_t tolower(int8_t c);
int8_t isalpha(int8_t c);
int8_t isdigit(int8_t c);
int8_t isalnum(int8_t c);

#endif // STRING_H 