#include "queue.h"
#include <stdio.h>

//------------------------------------------------------------------------------
// Insere um elemento no final da fila.
// Condicoes a verificar, gerando msgs de erro:
// - a fila deve existir
// - o elemento deve existir
// - o elemento nao deve estar em outra fila
void queue_append(queue_t **queue, queue_t *elem)
{
    // a fila deve existir
    if (queue != NULL)
    {
        // o elemento deve existir
        if (elem != NULL)
        {
            // o elemento nao deve estar em outra fila
            if (elem->next == NULL && elem->prev == NULL)
            {
                queue_t *first = *queue;

                if (first == NULL)
                {
                    *queue = elem;
                    elem->prev = elem;
                    elem->next = elem;
                }

                else
                {
                    queue_t *last = first->prev;
                    first->prev = elem;
                    last->next = elem;
                    elem->prev = last;
                    elem->next = first;
                }
            }

            else
            {
                printf("O elemento esta em outra fila\n");
            }
        }

        else
        {
            printf("O elemento nao existe\n");
        }
    }

    else
    {
        printf("A fila nao existe\n");
    }
}

//------------------------------------------------------------------------------
// Remove o elemento indicado da fila, sem o destruir.
// Condicoes a verificar, gerando msgs de erro:
// - a fila deve existir
// - a fila nao deve estar vazia
// - o elemento deve existir
// - o elemento deve pertencer a fila indicada
// Retorno: apontador para o elemento removido, ou NULL se erro
queue_t *queue_remove(queue_t **queue, queue_t *elem)
{
    // - a fila deve existir
    if (queue != NULL)
    {
        queue_t *current = *queue;
        queue_t *first = *queue;
        int removed = 0;

        // - a fila nao deve estar vazia
        if (current != NULL)
        {
            // - o elemento deve existir
            if (elem != NULL)
            {
                while(!removed)
                {
                    //o elemento deve pertencer a fila indicada
                    if (current == elem)
                    {
                        current->prev->next = current->next;
                        current->next->prev = current->prev;

                        //caso remova o primeiro elemento, deve-se mudar o ponteiro da queue
                        if (current == first)
                        {
                            *queue = current->next;

                            //se o primeiro for o único elemento
                            if (current->next == first)
                            {    
                                *queue = NULL;
                            }
                        }

                        current->next = NULL;
                        current->prev = NULL;

                        removed = 1;
                    }

                    else if (current != elem && current->next != first)
                    {
                        current = current->next;
                    }

                    else
                    {
                        printf("O elemento nao pertence a fila indicada\n");
                        removed = 1;

                        return NULL;
                    }
                }
            }

            else
            {
                printf("O elemento nao existe\n");
                return NULL;
            }
        }

        else
        {
            printf("A fila esta vazia\n");
            return NULL;
        }

        return elem;
    }

    else
    {
        printf("A fila nao existe\n");
        return NULL;
    }
}

//------------------------------------------------------------------------------
// Conta o numero de elementos na fila
// Retorno: numero de elementos na fila
int queue_size(queue_t *queue)
{
    queue_t *current = queue;
    queue_t *first;
    int size = 0;

    // fila vazia
    if (current == NULL)
    {
        return size;
    }

    // Conta o numero de elementos da fila
    else
    {
        first = current;
        size = 1;

        while (first->prev != current)
        {
            size++;
            current = current->next;
        }

        return size;
    }
}

//------------------------------------------------------------------------------
// Percorre a fila e imprime na tela seu conteúdo. A impressão de cada
// elemento é feita por uma função externa, definida pelo programa que
// usa a biblioteca. Essa função deve ter o seguinte protótipo:
//
// void print_elem (void *ptr) ; // ptr aponta para o elemento a imprimir
void queue_print(char *name, queue_t *queue, void print_elem(void *))
{
    queue_t *current = queue;
    int finished = 0;

    printf("%s: [", name);

    if (current != NULL)
    {
        // Percorre a fila e imprime na tela seu conteúdo
        while (!finished)
        {
            print_elem(current);
            current = current->next;

            if (current == queue)
            {
                finished = 1;
            }

            else
            {
                printf(" ");
            }
        }
    }
    printf("]\n");
}