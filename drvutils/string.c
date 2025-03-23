#include "string.h"

// String comparing
int32_t strcmp(const int8_t * s1, const int8_t * s2)
{
    while (*s1 && (*s1 == *s2))
    {
        s1++;
        s2++;
    }

    return *((const uint8_t *)s1 - *(const uint8_t *)s2);
}

// String copying
int8_t * strcpy(int8_t * dest, const int8_t * src)
{
    int8_t * d = dest;

    while ((*d++ = *src++));

    return dest;
}

// String length
size_t strlen(const int8_t * str)
{
    const int8_t * s;

    for (s = str; *s; ++s);

    return (size_t)(s - str);
}

// Memory copying
void * memcpy(void * dest, const void * src, size_t n)
{
    uint8_t * d = (uint8_t *)dest;
    const uint8_t * s = (const uint8_t *)src;

    for (size_t i = 0; i < n; i++)
    {
        d[i] = s[i];
    }

    return dest;
}

// Memory filling
void * memset(void * ptr, uint8_t value, size_t n)
{
    uint8_t * p = (uint8_t *)ptr;
    for (size_t i = 0; i < n; i++)
    {
        p[i] = (uint8_t)value;
    }

    return ptr;
}

// Memory comparing
int32_t memcmp(const void * s1, const void * s2, size_t n)
{
    const uint8_t * p1 = (const uint8_t *)s1;
    const uint8_t * p2 = (const uint8_t *)s2;

    for (size_t i = 0; i < n; i++)
    {
        if (p1[i] != p2[i])
        {
            return p1[i] - p2[i];
        }
    }

    return 0;
}

// Uppercase the character
int8_t toupper(int8_t c)
{
    if (c >= 'a' && c <= 'z')
    {
        return c - 'a' + 'A';
    }

    return c;
}

// Lowercase the character
int8_t tolower(int8_t c)
{
    if (c >= 'A' && c <= 'Z')
    {
        return c - 'A' + 'a';
    }
    
    return c;
}

// Check if the character is alphabetic
int8_t isalpha(int8_t c)
{
    return ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'));
}

// Check if the character is a digit
int8_t isdigit(int8_t c)
{
    return (c >= '0' && c <= '9');
}

// Check if the character is alphanumeric
int8_t isalnum(int8_t c)
{
    return (isalpha(c) || isdigit(c));
} 