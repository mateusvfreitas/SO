// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DAINF UTFPR
// Versão 1.0 -- Março de 2015
//
// Estruturas de dados internas do sistema operacional

#ifndef __DATATYPES__
#define __DATATYPES__

#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>
#include "queue.h"

#define STACKSIZE 32768		/* tamanho de pilha das threads */
#define _XOPEN_SOURCE 600	/* para compilar no MacOS */

enum status {READY, SUSPENDED, ENDED, SLEEPING};


// Estrutura que define uma tarefa
typedef struct task_t
{
    struct task_t *prev, *next ;
    int tid;
    ucontext_t context; // userlevel context
    enum status status;
    struct task_t *controle;
    int staticPrio;
    int dynamicPrio;
    int sysTask; // diferencia tarefas de sistema de tarefas de usuário
    int ticks;
    unsigned int activations;
    unsigned int processorTime;
    unsigned int execStart;
    unsigned int procStart;
    unsigned int timeToWakeUp; // task_sleep
    int exitCode;
    int waitingTId; // referencia quem colocou para dormir
    struct task_t **queueType; // referencia a fila em que a tarefa se encontra
} task_t ;

// estrutura que define um semáforo
typedef struct
{
    int destroyed;
    int semCount;
    task_t *semQueue;
} semaphore_t ;

// estrutura que define um mutex
typedef struct
{
  // preencher quando necessário
} mutex_t ;

// estrutura que define uma barreira
typedef struct
{
    int numThreads;
    int destroyed;
    task_t *barrierTaskQueue; // tarefas que chegam na barreira
    semaphore_t *semaphore; // condições de disputa
} barrier_t ;

typedef struct
{
    struct message *prev, *next;
    int size;
    void *conteudo;
} message;

// estrutura que define uma fila de mensagens
typedef struct
{
    semaphore_t *s_buffer, *s_mensagem, *s_vaga;
    int sizeMensagem;
    int destroyed;
    message *messageQueue;
} mqueue_t ;



#endif
