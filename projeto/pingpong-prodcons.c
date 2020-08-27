#include "pingpong.h"

typedef struct buffer_item
{
    struct buffer_item *prev, *next;
    int valor;
} buffer_item;

buffer_item *buffer;
semaphore_t s_buffer, s_item, s_vaga;
task_t prod1, prod2, prod3;
task_t cons1, cons2;

void produtor(void * arg)
{
    while(1)
    {
        task_sleep(1);

        //item = random(0..99)
        buffer_item *item;
        item = malloc(sizeof(buffer_item));
        item->valor = rand()%100;

        sem_down(&s_vaga);
        sem_down(&s_buffer);

        //insere item no buffer
        queue_append((queue_t **)&buffer, (queue_t *)item);
        printf("%s produziu %d\n", (char *) arg, item->valor);

        sem_up(&s_buffer);
        sem_up(&s_item);
    }
}

void consumidor(void * arg)
{
    while(1)
    {
        sem_down(&s_item);
        sem_down(&s_buffer);

        //retira item do buffer
        int valor_item = buffer->valor;
        queue_remove((queue_t **) &buffer, (queue_t *)buffer);

        sem_up(&s_buffer);
        sem_up(&s_vaga);

        //print item
        printf("%s consumiu %d\n", (char *)arg, valor_item);
        task_sleep(1);
    }
}

int main()
{
    pingpong_init();

    sem_create(&s_buffer, 1);
    sem_create(&s_item, 0);
    sem_create(&s_vaga, 5);

    task_create(&prod1, produtor, "prod1");
    task_create(&prod2, produtor, "prod2");
    task_create(&prod3, produtor, "prod3");
    task_create(&cons1, consumidor, "                 cons1");
    task_create(&cons2, consumidor, "                 cons2");

    task_join(&cons1);

    sem_destroy(&s_buffer);
    sem_destroy(&s_item);
    sem_destroy(&s_vaga);

    task_exit(0);

    return 0;
}