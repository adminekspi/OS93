#include "ports.h"

unsigned char inb(unsigned short port)
{
    unsigned char result;
    __asm__ volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

unsigned short inw(unsigned short port)
{
    unsigned short result;
    __asm__ volatile ("inw %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

unsigned int ind(unsigned short port)
{
    unsigned int result;
    __asm__ volatile ("inl %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

void outb(unsigned short port, unsigned char value)
{
    __asm__ volatile ("outb %0, %1" : : "a" (value), "Nd" (port));
}

void outw(unsigned short port, unsigned short value)
{
    __asm__ volatile ("outw %0, %1" : : "a" (value), "Nd" (port));
}

void outd(unsigned short port, unsigned int value)
{
    __asm__ volatile ("outl %0, %1" : : "a" (value), "Nd" (port));
}
