#include "header.h"

void pop_back(List * list) // удаляем последний элемент и переназначаем ссылки
{
    if (list == NULL) {
        return;
    }

    if ((list)->first == NULL) {
        return;
    }
    if ((list)->first == (list)->last) {
        free((list)->first);
        (list)->first = (list)->last = NULL;
    } else
    {
        Node * prev = (list)->last->prev;
        free((list)->last);
        list->last = prev;
        prev->next = NULL;
    }
}

void show() // показывает состояние блоков памяти
{
    for (int i = 0; i <= N; ++i)
    {
        printf("block %d\n", i);
        Node * node = freeBlocks[i].first;
        while(node != NULL)
        {
            printf("    free block from %d to %d\n", node->mem - memory, node->mem - memory + (1<<i));
            node = node->next;
        }
    }
    printf("\n");
}

void write_additional_info(char * start, int size, char is_free) // запись дополнительной информации о блоке
{
    start -= sizeof(Info); // сдвиг указателя влево на количество байт, в которые запишется информация

    int * curBuf = (int*)(start); // записывается сначала количество байт, затем свободен блок или нет
    *curBuf = size;

    start += sizeof(int);
    *start = is_free;
}

void push_back(char * buf, int block) // создаём элемент и перекидываем ссылку на него
{
    Node * node = malloc(sizeof(Node));
    memset(node, 0, sizeof(Node));
    node->mem = buf;
    if (freeBlocks[block].first == NULL) {
        freeBlocks[block].first = freeBlocks[block].last = node;
    }
    else {
        freeBlocks[block].last->next = node;
        node->prev = freeBlocks[block].last;
        freeBlocks[block].last = node;
    }
}

void push_back_vector(char * buf, int block, int is_free) // закидывает элемент в конец и записывает информацию о блоке, который добавляется
{
    push_back(buf, block);
    write_additional_info(buf + sizeof(Info), pw(block), is_free);
}

Info get_additional_info(char * start) // узнать информацию о блоке (размер и свободен он или нет)
{
    Info res;
    start -= sizeof(Info);

    int * curBuf = (int*)(start);
    res.size = *curBuf;

    res.is_free = *(start + intSize);
    return res;
}

int minDegree(int size) //минимальная степень двойки
{
    char isNeed = 0; // есть ли единичный бит в середине   000010110110 -> 100000000 -> 8
    int counter = 0; // считает количество байт
    while (size > 1)
    {
        if (size & 1)
            isNeed = 1;
        counter++;
        size >>= 1;
    }
    return counter + isNeed; // возращает не 2^N, а N
}

char * getTwin(char * brother, int block) // ищем близнеца
{
    int ind = brother - memory;
    ind ^= pw(block); // побитовое сложение
    return memory + ind;
}

void divideLastFreeBlock(int block) // делим блок на два равных
{
    char * buf = freeBlocks[block].last->mem;

    pop_back(&freeBlocks[block]);
    push_back_vector(getTwin(buf, block - 1), block - 1, 1);
    push_back_vector(buf, block - 1, 1);
}

void initMemory() // инициализация всего
{
    intSize = sizeof(int); // размер инта
    bitsInByte = 8; // количество бит в байте
    N = 8; // количество выделяемой памяти (2^N) (используется 8 для удобства работы, но можно поставить и большее число, исходя из размера freeBlocks)
    BLOCK_SIZE = (1 << N); // размер блока памяти, вычисляется с помощью побитового сдвига N влево

    memory = malloc(BLOCK_SIZE); // выделяем память, с которой будем произволить различные операции
    memset(memory, 0, sizeof(memory)); // стандартная функция, которая заполняет memory нулями, столько, сколько выделено (size)

    for (int i = 0; i <= N; ++i)
        freeBlocks[i].first = freeBlocks[i].last = NULL; // пустой массив

    push_back_vector(memory, N, 1);
}

char * alloc_by_degree(int size, int degree) // выделение памяти под блок размера degree
{
    if (degree > N) // не можем выделить память больше той, которая используется
        return NULL;

    if (freeBlocks[degree].last == 0) // Проверим, есть ли свободный блок. Нет - пытаемся использовать блок побольше. Да - смотрим, надо ли его делить
        return alloc_by_degree(size, degree + 1);

    if (pw(degree) >= size * 2)
    {
        divideLastFreeBlock(degree);
        return alloc_by_degree(size, degree - 1);
    }

    char * res = freeBlocks[degree].last->mem; // передаем функции pop_back указатель на этот элемент

    pop_back(&freeBlocks[degree]);
    return res;
}

char * alloc(int size) // функция выделения памяти
{
    int needed_size = size + sizeof(Info);
    int degree = minDegree(needed_size); // 5 -> 3

    char * res = alloc_by_degree(pw(degree), degree);
    if (res == NULL)
        return NULL;
    res += sizeof(Info);

    write_additional_info(res, pw(degree), 0);
    return res;
}

int get_degree(int n) // возвращает степень введенного числа
{
    int c = 0;
    while (n > 1)
    {
        c++;
        n >>= 1;
    }
    return c;
}

void erase(List * list, Node * node) // находим элемент и переназначаем ссылки соседних элементов, очищаем память
{
    if (list == NULL)
        return;
    if (list->first == NULL)
        return;
    if (list->last == NULL)
        return;

    if (list->first == list->last && list->first != node)
        return;

    if (list->first == list->last && list->first == node)
    {
        list->first = list->last = NULL;
        free(node);
    } else if (list->first == node)
    {
        list->first = list->first->next;
        list->first->prev = NULL;
        free(node);
    } else if (list->last == node)
    {
        list->last->prev->next = NULL;
        list->last = node->prev;
        free(node);
    } else
    {
        node->prev->next = node->next;
        node->next->prev = node->prev;
        free(node);
    }
}

char * min(char * buf, char * twin) // возращает минимальный из адресов для близнецов
{
    if (buf - memory > twin - memory)
        return twin;
    return buf;
}

void tryGlueWithTwin(char * buf, int degree) // пытаемся объединить блоки
{
    if (degree < N)
    {
        char * twin = getTwin(buf, degree);
        Info twin_info = get_additional_info(twin + sizeof(Info));
        if (twin_info.size == pw(degree) && twin_info.is_free > 0)
        {
            pop_back(&freeBlocks[degree]);
            char ok = 0;
            Node * cur = freeBlocks[degree].first;
            while(cur != NULL)
            {
                if (cur->mem == twin)
                {
                    erase(&freeBlocks[degree], cur);
                    ok = 1;
                    break;
                }
                cur = cur->next;
            }
            if (!ok)
            {
                printf("Something wrong : cant find twin in free blocks\n");
                fprintf(stderr, "Something wrong : cant find twin in free blocks\n");
                exit(0);
            }
            char * mn = min(buf, twin);
            push_back_vector(mn, degree + 1, 1);
            tryGlueWithTwin(mn, degree + 1);
        }
    }
}

void myfree(char * buf) // очищаем память
{
    if (buf - memory < sizeof(Info))
    {
        printf("try to dealloc not allocated memory by ind < 0: %d\n", buf - memory);
        fprintf(stderr,"try to dealloc not allocated memory by ind < 0: %d\n", buf - memory);
        return;
    }
    Info cur = get_additional_info(buf);
    if (cur.size <= 0)
    {
        printf("try to dealloc not allocated memory by ind size <= 0: %d\n", buf - memory);
        fprintf(stderr,"try to dealloc not allocated memory by ind size <= 0: %d\n", buf - memory);
        return;
    }

    if (cur.is_free > 0)
    {
        printf("try to dealloc not allocated memory by ind is_free>0: %d\n", buf - memory);
        fprintf(stderr, "try to dealloc not allocated memory by ind is_free>0: %d\n", buf - memory);
        return;
    }
    buf -= sizeof(Info);
    int size = cur.size;
    int degree = get_degree(size);

    push_back_vector(buf, degree, 1);

    tryGlueWithTwin(buf, degree);
}

void write_to_file(FILE * fptr) // на каждой строке записываем 6 значений: начало блока памяти, размер блока, свободен ли он, начало блока близнеца,
{                               // размер блока близнеца, свободен ли блок близнец
    int cnt = 0;
    for (int i = 0; i <= N;++i)
    {
        Node *node = freeBlocks[i].first;
        while (node != NULL)
        {
            ++cnt;
            node = node->next;
        }
    }

    fprintf(fptr, "%d\n", cnt);
    for (int i = 0; i <= N;++i)
    {
        Node * node = freeBlocks[i].first;
        while(node != NULL)
        {
            Info info = get_additional_info(node->mem + sizeof(Info));
            fprintf(fptr, "%d %d %d ", node->mem - memory, info.size, (int)info.is_free);
            char * twin = getTwin(node->mem, i);
            Info twin_info = get_additional_info(twin + sizeof(Info));
            if (i != N)
                fprintf(fptr, "%d %d %d\n", twin - memory, twin_info.size, (int)twin_info.is_free);
            else
                fprintf(fptr, "%d %d %d\n", 0, 0, 0);
                node = node->next;
        }
    }
}

void read_from_file(FILE * fptr) {
    for (int i = 0; i <= N; ++i) // удаляем элементы, пока последний элемент не равен нулю
    {
        while(freeBlocks[i].last != NULL)
            erase(&freeBlocks[i], freeBlocks[i].last);
    }

    memset(memory, 0, sizeof(memory)); // заполняем всю используюемую память нулями
    int start, size; // параметры блока памяти, начало блока, его размер
    char is_free; // свободен ли блок

    int cnt = 0; // количество строк
    fscanf(fptr, "%d", &cnt);

    while(cnt--)
    {
        // считывание блока и запись его в конец
        fscanf(fptr, "%d %d %c", &start, &size, &is_free);
        push_back_vector(memory + start, minDegree(size), 1);

        // считывание блока близнеца и запись информации о блоке
        fscanf(fptr, "%d %d %c", &start, &size, &is_free);
        write_additional_info(memory + start + sizeof(Info), size, 0);
    }
}

int done() // проверяем утечки памяти
{
    int cnt = 0; // количество утечек памяти
    for (int i = 0; i < N; ++i)
    {
        Node * cur = freeBlocks[i].first;
        while(cur != NULL)
        {
            ++cnt;
            cur = cur->next;
        }
    }
    if (cnt > 0)
        return cnt;

    if (freeBlocks[N].first != freeBlocks[N].last)
        return 1;

    if (freeBlocks[N].first == NULL)
        return 1;

    Info info = get_additional_info(freeBlocks[N].first->mem + sizeof(Info));
    if (info.is_free == 0)
        return 1;

    return 0;
}

int main()
{
    initMemory(); // выделяем объем памяти, с которым будем работать
    while(1)
    {
        int t, s;
        scanf("%d", &t);

        if (t == 1) // выделяем s байт
        {
            scanf("%d", &s);
            char * buf = alloc(s);
            if (buf == NULL)
                printf("There is no free memory\n");
            else
                printf("malloc %d bytes by index %d\n", s, buf - memory);
        }
        else if (t== 2) // очищаем память с заданного адреса
        {
            scanf("%d", &s);
            myfree(memory + s);
        }
        else if (t == 3) // показать информацию о блоках
        {
            show();
        }
        else if (t == 4) // записываем в файл, указываем путь до файла, если файла не было, он создается, если был, то он перезаписывается
        {
            FILE * fptr;
            const char * filename = malloc(100); // = "D:/Users/user/Documents/MemoryManager/input.txt"

            scanf("%s", filename);
            fptr = fopen(filename, "w");
            if (fptr == NULL)
            {
                printf("cant create file");
                fprintf(stderr, "cant create file");
                return 0;
            }
            write_to_file(fptr);
            fclose(fptr);
        }
        else if (t == 5) // считываем из файла, указываем путь до файла
        {
            FILE * fptr;
            const char * filename = malloc(100); // = "D:/Users/user/Documents/MemoryManager/input.txt"

            scanf("%s", filename);
            fptr = fopen(filename, "r");
            if (fptr == NULL)
            {
                printf("cant read file");
                fprintf(stderr, "cant read file");
                return 0;
            }
            read_from_file(fptr);
            fclose(fptr);
        }
        else // любое другое число
            break;
    }

    int mem_leaks = done(); // проверяем, есть ли утечки памяти
    if (mem_leaks > 0)
        printf("There are %d mem_leaks\n", mem_leaks);
    else
        printf("Everything is ok!\n");

    return 0;
}
