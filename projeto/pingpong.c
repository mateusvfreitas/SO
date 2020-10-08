#include "pingpong.h"
#define PRIO 0
#define QUANTUM 20

// globais
int taskId;
task_t taskMain;
task_t *taskCurrent;
task_t dispatcher;
task_t *tasksReady;
task_t *tasksSuspended;
unsigned int clock;

// estrutura que define um tratador de sinal
struct sigaction action ;
// estrutura de inicialização to timer
struct itimerval timer;

// envelhecimento de tarefas
task_t *aging(task_t *tasksReady)
{
    int lowest;
    int alfa = -1;

    task_t *first = tasksReady;
    task_t *aux = tasksReady->next;
    task_t *eldest = tasksReady;
    lowest = eldest->dynamicPrio;

    while(aux != first)
    {
        if((aux->dynamicPrio < lowest))
        {
            lowest = aux->dynamicPrio;
            eldest->dynamicPrio += alfa;
            eldest = aux;
        }

        else
        {
            aux->dynamicPrio--;
        }
        aux = aux->next;
    }

    eldest->dynamicPrio = eldest->staticPrio;
    return eldest;
}

task_t *scheduler()
{
    task_t *schedule = aging(tasksReady);

//    em caso de política FCFS, usar:
//    task_t *schedule = tasksReady;
    return schedule;
}

// controle geral de tarefas
void dispatcher_body(void *arg)
{
    while (queue_size((queue_t *)tasksReady) > 0)
    {
        task_t *next = scheduler();
        if (next)
        {
            // Trocas de contexto
            task_switch(next);
            if (next->status == Ready)
            {
                queue_remove((queue_t **)&tasksReady, (queue_t *)next);
                queue_append((queue_t **)&tasksReady, (queue_t *)next);
            }

            else if (next->status == Ended)
            {
                queue_remove((queue_t **)&tasksReady, (queue_t *)next);
            }

            else
            {
                return;
            }
        }
    }
    task_exit(0);
}

// Similar à timer.c ===========================================================
void tratador()
{
    clock += 1;
    // Ao ser acionada, a rotina de tratamento de ticks de relógio deve decrementar o contador de quantum da tarefa
    // corrente, se for uma tarefa de usuário.
    if(taskCurrent->sysTask == 0)
    {
        taskCurrent->ticks--;
        
        // Se o contador de quantum chegar a zero, a tarefa em execução deve voltar à fila de prontas e o controle do
        // processador deve ser devolvido ao dispatcher.
        if(taskCurrent->ticks == 0)
        {
            task_yield();
        }
    }

    else
    {
        return;
    }
}

void timerFunction()
{
    // registra a ação para o sinal de timer SIGALRM
    action.sa_handler = tratador ;
    sigemptyset (&action.sa_mask) ;
    action.sa_flags = 0 ;
    if (sigaction (SIGALRM, &action, 0) < 0)
    {
        perror ("Erro em sigaction: ") ;
        exit (1) ;
    }

    // ajusta valores do temporizador
    timer.it_value.tv_usec = 1000 ;      // primeiro disparo, em micro-segundos
    timer.it_value.tv_sec  = 0 ;      // primeiro disparo, em segundos
    timer.it_interval.tv_usec = 1000 ;   // disparos subsequentes, em micro-segundos
    timer.it_interval.tv_sec  = 0 ;   // disparos subsequentes, em segundos

    // arma o temporizador ITIMER_REAL (vide man setitimer)
    if (setitimer (ITIMER_REAL, &timer, 0) < 0)
    {
        perror ("Erro em setitimer: ") ;
        exit (1) ;
    }
}
// =============================================================================

// funções gerais ==============================================================

// Inicializa o sistema operacional; deve ser chamada no inicio do main()
void pingpong_init()
{
    taskId = 0;
    taskMain.tid = 0;
    taskId += 1;
    taskCurrent = &taskMain;

    task_create(&dispatcher, dispatcher_body, NULL);
    dispatcher.controle = &taskMain;
    dispatcher.sysTask = 1; // Dispatcher é uma tarefa crítica -> tarefa de sistema
    queue_remove((queue_t **)&tasksReady, (queue_t *)&dispatcher);
    timerFunction();

    /* desativa o buffer da saida padrao (stdout), usado pela função printf */
    setvbuf(stdout, 0, _IONBF, 0);
}

// gerência de tarefas =========================================================

// Cria uma nova tarefa. Retorna um ID> 0 ou erro.
// 1º parâmetro: descritor da nova tarefa
// 2º parâmetro: funcao corpo da tarefa
// 3º parâmetro: argumentos para a tarefa
int task_create(task_t *task, void (*start_func)(void *), void *arg)
{
    ucontext_t newContext;
    char *stack = malloc(STACKSIZE);

    // Salva o contexto atual na variável newContext
    getcontext(&newContext);

    if (stack)
    {
        newContext.uc_stack.ss_sp = stack;
        newContext.uc_stack.ss_size = STACKSIZE;
        newContext.uc_stack.ss_flags = 0;
        newContext.uc_link = 0;
    }
    else
    {
        perror("Erro na criacao da pilha: ");
        exit(-1);
    }

    // Ajusta alguns valores internos do contexto salvo em newContext
    makecontext(&newContext, (void *)(*start_func), 1, arg);

    queue_append((queue_t **)&tasksReady, (queue_t *)task);

    task->context = newContext;
    task->tid = taskId;
    taskId++;
    task->status = Ready;
    task->controle = &dispatcher;
    task->staticPrio = PRIO;
    task->dynamicPrio = PRIO;
    task->sysTask = 0;
    task->activations = 0;
    task->processorTime = 0;
    task->execStart = systime();
    task->procStart = systime();

#ifdef DEBUG
    printf("task_create: criou tarefa %d\n", task->tid);
#endif

    return task->tid;
}

// Termina a tarefa corrente, indicando um valor de status encerramento
void task_exit(int exitCode)
{
#ifdef DEBUG
    printf("task_exit: tarefa %d sendo encerrada\n", taskCurrent->tid);
#endif
    printf("Task %d: execution time %u ms, processor time %u ms, %u activations\n",
            taskCurrent->tid, systime() - taskCurrent->execStart, taskCurrent->processorTime, taskCurrent->activations);

    taskCurrent->status = Ended;
    task_switch(taskCurrent->controle);
}

// alterna a execução para a tarefa indicada
int task_switch(task_t *task)
{
#ifdef DEBUG
   printf("task_switch: trocando contexto %d -> %d\n", taskCurrent->tid, task->tid);
#endif

    task_t *taskPrevious = taskCurrent;
    taskCurrent = task;

    taskPrevious->processorTime += systime() - taskPrevious->procStart;
    taskCurrent->procStart = systime();
    taskCurrent->activations += 1;

    taskCurrent->ticks = QUANTUM;

    swapcontext(&taskPrevious->context, &task->context);

    return 0;
}

// retorna o identificador da tarefa corrente (main é 0)
int task_id()
{
#ifdef DEBUG
    printf("task_id: tarefa tem id = %d\n", taskCurrent->tid);
#endif
    return (taskCurrent->tid);
}

// suspende uma tarefa, retirando-a de sua fila atual, adicionando-a à fila
// queue e mudando seu estado para "suspensa"; usa a tarefa atual se task==NULL
void task_suspend(task_t *task, task_t **queue)
{
    if (task == NULL)
    {
        task = taskCurrent;
    }

    if (queue == NULL)
    {
        return;
    }

    queue_remove((queue_t **)&tasksReady, (queue_t *)task);
    queue_append((queue_t **)&tasksSuspended, (queue_t *)task);

    task->status = Suspended;
}

// acorda uma tarefa, retirando-a de sua fila atual, adicionando-a à fila de
// tarefas prontas ("ready queue") e mudando seu estado para "pronta"
void task_resume(task_t *task)
{
    queue_remove((queue_t **)&tasksSuspended, (queue_t *)task);
    queue_append((queue_t **)&tasksReady, (queue_t *)task);

    task->status = Ready;
}

// operações de escalonamento ==================================================

// libera o processador para a próxima tarefa, retornando à fila de tarefas
// prontas ("ready queue")
void task_yield()
{
    task_switch(&dispatcher);
}

// define a prioridade estática de uma tarefa (ou a tarefa atual)
void task_setprio(task_t *task, int prio)
{
    if (task == NULL)
    {
        task = taskCurrent;
        if (prio <= 20 && prio >= -20)
        {
            task->staticPrio = prio;
            task->dynamicPrio = prio;
        }
        else
        {
            // prioridade invalida;
        }
    }
    else
    {
        if (prio <= 20 && prio >= -20)
        {
            task->staticPrio = prio;
            task->dynamicPrio = prio;
        }
        else
        {
            // prioridade invalida;
        }
    }
}

// retorna a prioridade estática de uma tarefa (ou da tarefa atual)
int task_getprio(task_t *task)
{
    if (task == NULL)
    {
        return taskCurrent->staticPrio;
    }
    else
    {
        return task->staticPrio;
    }
}

unsigned int systime()
{
    return clock;
}