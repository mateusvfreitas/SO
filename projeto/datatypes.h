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
#include "queue.h"
#include <signal.h>
#include <sys/time.h>

#define STACKSIZE 32768		/* tamanho de pilha das threads */
#define _XOPEN_SOURCE 600	/* para compilar no MacOS */

enum status {Ready, Suspended, Ended};

// Estrutura que define uma tarefa
typedef struct task_t
{
    struct task_t *prev, *next ;
    int tid;
    ucontext_t context; // Userlevel context
    enum status status;
    struct task_t *controle;
    int staticPrio;
    int dynamicPrio;
    int sysTask; // Flag que diferencia tarefas de sistema de taredas de usuário
    int ticks;

} task_t ;

// estrutura que define um semáforo
typedef struct
{
  // preencher quando necessário
} semaphore_t ;

// estrutura que define um mutex
typedef struct
{
  // preencher quando necessário
} mutex_t ;

// estrutura que define uma barreira
typedef struct
{
  // preencher quando necessário
} barrier_t ;

// estrutura que define uma fila de mensagens
typedef struct
{
  // preencher quando necessário
} mqueue_t ;

#endif
