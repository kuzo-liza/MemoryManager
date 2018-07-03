#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int bitsInByte; // количество битов в байте
int intSize; // размер инта
int N; // количество выделяемой памяти (2^N)
int BLOCK_SIZE; // размер блока памяти, вычисляется с помощью побитового сдвига N влево

#define pw(n) (1<<n) // макрос побитового сдвига

char * memory = NULL;

typedef struct Node_t
{
    struct Node_t * prev; // указатель на предыдущий элемент
    struct Node_t * next; // указатель на следующий элемент
    char * mem; // указатель на начало блока памяти
} Node;

typedef struct Info_t
{
    int size; // размер блока памяти
    char is_free; // свободен ли блок,
} Info;

typedef struct List_t
{
    Node * first; // указатель на первый элемент
    Node * last; // указатель на последний элемент
} List;

List freeBlocks[22]; // массив блоков, должен быть < N

#endif // HEADER_H
