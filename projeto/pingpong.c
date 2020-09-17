#include "pingpong.h"
#define PRIO 0
#define QUANTUM 20
#define WAKE_UP_CHECK 73 // Checa fila de adormecidas a cada 73 milisegundos

// globais
int taskId;
task_t taskMain;
task_t dispatcher;
task_t *taskCurrent;
task_t *tasksReady;
task_t *tasksSuspended;
task_t *tasksSleeping;
unsigned int clock;
int preemptavel = 1; // var lock -> garante as mudanças de fila do semáforo

// estrutura que define um tratador de sinal
struct sigaction action;
// estrutura de inicialização to timer
struct itimerval timer;

// envelhecimento de tarefas
task_t *aging(task_t *tasksToAge)
{
    if(tasksToAge == NULL)
    {
        return NULL;
    }
    int lowest;
    int alfa = -1;

    task_t *first = tasksToAge;
    task_t *aux = tasksToAge;
    task_t *eldest = tasksToAge;
    lowest = eldest->dynamicPrio;

    do
    {
        if (aux->dynamicPrio < lowest)
        {
            lowest = aux->dynamicPrio;
            eldest = aux;
            eldest->dynamicPrio += alfa;

        }

        aux = aux->next;
    } while (aux != first);

    eldest->dynamicPrio = eldest->staticPrio;
    return eldest;
}

// determina a próxima tarefa a executar
task_t *scheduler()
{
    task_t *schedule = aging(tasksReady);

    // em caso de política FCFS, usar:
    // task_t *schedule = tasksReady;
    return schedule;
}

// controle geral de tarefas
void dispatcher_body(void *arg)
{
    while (queue_size((queue_t *)tasksReady) > 0 || queue_size((queue_t *)tasksSleeping) > 0)
    {
        //periodicamente o dispatcher percorre a fila de tarefas adormecidas
        if (systime() % WAKE_UP_CHECK == 0 && tasksSleeping != NULL)
        {
            task_t *sleepArray[666];
            int i = 0;
            task_t *first = tasksSleeping;
            task_t *aux = tasksSleeping;
            do
            {
                if (systime() >= aux->timeToWakeUp)
                {
                    sleepArray[i] = aux;
                    i++;
                }
                aux = aux->next;
            } while (aux != first);

            //remove do array e coloca na fila
            for (int j = 0; j < i; j++)
            {
                task_resume(sleepArray[j]);
            }
        }

        //tarefa mais velha da fila ready
        task_t *next = scheduler();
        if (next)
        {
            // Trocas de contexto
            if (next->status == ENDED)
            {
                queue_remove((queue_t **)&tasksReady, (queue_t *)next);
            }
            else if (next->status == SUSPENDED)
            {
                task_suspend(next, &tasksSuspended);
            }
            else if (next->status == READY)
            {
                queue_remove((queue_t **)&tasksReady, (queue_t *)next);
                queue_append((queue_t **)&tasksReady, (queue_t *)next);
                task_switch(next);
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
    if (taskCurrent->sysTask == 0)
    {
        taskCurrent->ticks--;

        // Se o contador de quantum chegar a zero, a tarefa em execução deve voltar à fila de prontas e o controle do
        // processador deve ser devolvido ao dispatcher.
        if (taskCurrent->ticks == 0)
        {
            if (preemptavel)
            {
                task_yield();
            }
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
    action.sa_handler = tratador;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if (sigaction(SIGALRM, &action, 0) < 0)
    {
        perror("Erro em sigaction: ");
        exit(1);
    }

    // ajusta valores do temporizador
    timer.it_value.tv_usec = 1000;    // primeiro disparo, em micro-segundos
    timer.it_value.tv_sec = 0;        // primeiro disparo, em segundos
    timer.it_interval.tv_usec = 1000; // disparos subsequentes, em micro-segundos
    timer.it_interval.tv_sec = 0;     // disparos subsequentes, em segundos

    // arma o temporizador ITIMER_REAL (vide man setitimer)
    if (setitimer(ITIMER_REAL, &timer, 0) < 0)
    {
        perror("Erro em setitimer: ");
        exit(1);
    }

    clock = 0;
}
// =============================================================================

// funções gerais ==============================================================

// Inicializa o sistema operacional; deve ser chamada no inicio do main()
void pingpong_init()
{
    //criando taskMain
    taskId = 0;
    taskMain.tid = 0;
    taskId += 1;
    taskMain.controle = &dispatcher;
    taskMain.staticPrio = PRIO;
    taskMain.dynamicPrio = PRIO;
    taskMain.status = READY;
    taskMain.queueType = &tasksReady;
    taskMain.sysTask = 0;
    taskMain.activations = 0;
    taskMain.processorTime = 0;
    taskMain.procStart = systime();
    taskMain.execStart = systime();

    queue_append((queue_t **)&tasksReady, (queue_t *)&taskMain);

    taskCurrent = &taskMain;

    task_create(&dispatcher, dispatcher_body, NULL);
    dispatcher.controle = NULL;
    dispatcher.sysTask = 1; // Dispatcher é uma tarefa crítica -> tarefa de sistema
    queue_remove((queue_t **)&tasksReady, (queue_t *)&dispatcher);
    timerFunction();

    task_yield();

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

    task->queueType = &tasksReady;
    queue_append((queue_t **)&tasksReady, (queue_t *)task);

    task->context = newContext;
    task->tid = taskId;
    taskId++;
    task->status = READY;
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

    taskCurrent->status = ENDED;

    // acorda tarefas suspensas pela tarefa encerrada
    task_t *suspArray[666];
    int i = 0;
    if (tasksSuspended)
    {
        task_t *first = tasksSuspended;
        task_t *aux = tasksSuspended;
        do
        {
            if (aux->waitingTId == taskCurrent->tid)
            {
                // coloca no array de tarefas a acordar
                suspArray[i] = aux;
                i++;
                aux->waitingTId = -1;
            }
            aux = aux->next;
        } while (aux != first);

        // percorre array e acorda as tarefas
        for (int j = 0; j < i; j++)
        {
            task_resume(suspArray[j]);
        }
    }
    taskCurrent->exitCode = exitCode;
    if (taskCurrent->controle != NULL)
    {
        task_switch(taskCurrent->controle);
    }
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

// suspende uma tarefa, retirando-a de sua fila atual, adicionando-a à fila queue e mudando seu estado para "suspensa"; usa a tarefa atual se task==NULL
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

    task->queueType = queue;
    queue_remove((queue_t **)&tasksReady, (queue_t *)task);
    queue_append((queue_t **)queue, (queue_t *)task);

    task->status = SUSPENDED;
}

// acorda uma tarefa, retirando-a de sua fila atual, adicionando-a à fila de tarefas prontas ("ready queue") e mudando seu estado para "pronta"
void task_resume(task_t *task)
{
    queue_remove((queue_t **)task->queueType,(queue_t *)task);
    queue_append((queue_t **)&tasksReady, (queue_t *)task);
    task->status = READY;
}

// operações de escalonamento ==================================================

// libera o processador para a próxima tarefa, retornando à fila de tarefas prontas ("ready queue")
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

// taskCurrent é suspensa até a conclusão de *task
int task_join(task_t *task)
{
    //se a tarefa nao existir ou ja foi encerrada
    if ((task == NULL) || (task->status == ENDED))
    {
        return (-1);
    }
    taskCurrent->waitingTId = task->tid;
    task_suspend(taskCurrent, &tasksSuspended);
    task_yield();
    return task->exitCode;
}

// suspende a tarefa corrente por t segundos
void task_sleep(int t)
{
    // Retira a tarefa solicitante da fila de prontas e a coloca na de adormecidas
    queue_remove((queue_t **)&tasksReady, (queue_t *)taskCurrent);
    queue_append((queue_t **)&tasksSleeping, (queue_t *)taskCurrent);
    taskCurrent->queueType = &tasksSleeping;
    taskCurrent->status = SLEEPING;

    // Calcula o instante em que a tarefa deve ser acordada
    taskCurrent->timeToWakeUp = systime() + 1000 * t;

    // Devolve o controle ao dispatcher
    task_yield();
}

// operações de IPC ============================================================

// semáforos

// cria um semáforo com valor inicial "value"
int sem_create (semaphore_t *s, int value)
{
    if (s)
    {
        s->semCount = value;
        s->semQueue = NULL;
        s->destroyed = 0;
        return 0;
    }
    return (-1);
}

// requisita o semáforo
int sem_down (semaphore_t *s)
{
    if (s == NULL || s->destroyed)
    {
        return (-1);
    }
    preemptavel = 0;
    s->semCount -= 1;
    //caso o contador seja negativo, a tarefa corrente é suspensa, inserida no final da fila do semaforo e a execução volta ao dispatcher.
    if(s->semCount < 0)
    {
        task_suspend(NULL, &s->semQueue);
        preemptavel = 1;
        task_yield();
    }
    preemptavel = 1;
    return 0;
}

// libera o semáforo
int sem_up (semaphore_t *s)
{
    if (s == NULL || s->destroyed)
    {
        return (-1);
    }
    preemptavel = 0;

    s->semCount += 1;    
    // se tiver tarefa suspensa, acorda a primeira da fila do semáforo
    if (s->semCount <= 0)
    {
        task_resume(s->semQueue);
    }
    preemptavel = 1;
    return 0;
}

// destroi o semáforo, liberando as tarefas bloqueadas
int sem_destroy (semaphore_t *s)
{
    if (s == NULL)
    {
        return (-1);
    }

    s->destroyed = 1;

    if(s->semQueue == NULL)
    {
        return 0;
    }

    int i = 0;
    task_t *resumeArray[666];
    task_t *aux = s->semQueue;
    task_t *first = s->semQueue;
    
    do
    {
        resumeArray[i] = aux;
        i++;
        aux = aux->next;
    } while (aux != first);
    
    for (int j = 0; j < i; j++)
    {
        task_resume(resumeArray[j]);
    }

    return 0;
}

// barreiras

// Inicializa uma barreira
int barrier_create (barrier_t *b, int N)
{
    if(b == NULL)
    {
        return -1;
    }
    b->destroyed = 0;
    semaphore_t barrierSem;
    sem_create(&barrierSem, 1);
    b->semaphore = &barrierSem;
    b->numThreads = N;
    return 0;
}

// Chega a uma barreira
int barrier_join (barrier_t *b)
{
    sem_down(b->semaphore); //para controle de condições de disputa

    if(b == NULL || b->destroyed == 1)
    {
        return -1;
    }

    b->numThreads -= 1;
    
    if(b->numThreads > 0)
    {
        task_suspend(NULL, &b->barrierTaskQueue);
        task_yield();
    }
    else
    {
        barrier_destroy(b);
    }
    
    sem_up(b->semaphore);

    return 0;
}

// Destrói uma barreira
int barrier_destroy (barrier_t *b)
{
    if(b == NULL)
    {
        return -1;
    }

    int i = 0;
    task_t *resumeArray[666];
    task_t *aux = b->barrierTaskQueue;
    task_t *first = b->barrierTaskQueue;

    do
    {
        resumeArray[i] = aux;
        i++;
        aux = aux->next;
    } while (aux != first);

    for (int j = 0; j < i; j++)
    {
        task_resume(resumeArray[j]);
    }

    b->destroyed = 1;

    return 0;
}

// filas de mensagens

// cria uma fila para até max mensagens de size bytes cada
int mqueue_create (mqueue_t *queue, int max, int size)
{
    if(queue == NULL)
    {
        return -1;
    }
    
    semaphore_t *s_buffer, *s_mensagem, *s_vaga;
    s_buffer = malloc(sizeof(semaphore_t));
    s_mensagem = malloc(sizeof(semaphore_t));
    s_vaga = malloc(sizeof(semaphore_t));
    
    sem_create(s_buffer, 1);
    sem_create(s_mensagem, 0);
    sem_create(s_vaga, max);

    queue->s_buffer = s_buffer;
    queue->s_mensagem = s_mensagem;
    queue->s_vaga = s_vaga;
    queue->sizeMensagem = size;

    return 0;
}

// envia uma mensagem para a fila
int mqueue_send (mqueue_t *queue, void *msg)
{
    sem_down(queue->s_vaga);
    sem_down(queue->s_buffer);

    if(queue == NULL || queue->destroyed == 1|| msg == NULL)
    {
        return -1;
    }
    

    message *mensagem = malloc(sizeof(message));
    mensagem->conteudo = malloc(queue->sizeMensagem);
    mensagem->size = queue->sizeMensagem;

    memcpy(mensagem->conteudo, msg, mensagem->size);
    queue_append((queue_t **) &queue->messageQueue, (queue_t *) mensagem);

    sem_up(queue->s_buffer);
    sem_up(queue->s_mensagem);

    return 0;
}

// recebe uma mensagem da fila
int mqueue_recv (mqueue_t *queue, void *msg)
{
    sem_down(queue->s_mensagem);
    sem_down(queue->s_buffer);

    if(queue == NULL || queue->destroyed || msg == NULL)
    {
        return -1;
    }
    

    message *mensagem = queue->messageQueue;

    queue_remove((queue_t **) &queue->messageQueue, (queue_t *) mensagem);
    memcpy(msg, mensagem->conteudo, mensagem->size);

    sem_up(queue->s_buffer);
    sem_up(queue->s_vaga);

    return 0;
}

// destroi a fila, liberando as tarefas bloqueadas
int mqueue_destroy (mqueue_t *queue)
{
    if(queue == NULL)
    {
        return -1;
    }
    
    sem_destroy(queue->s_buffer);
    sem_destroy(queue->s_mensagem);
    sem_destroy(queue->s_vaga);

    queue->destroyed = 1;

    return 0;
}

// informa o número de mensagens atualmente na fila
int mqueue_msgs (mqueue_t *queue)
{
    if(queue == NULL || queue->s_mensagem == NULL || queue->destroyed == 1)
    {
        return -1;
    }

    return queue->s_mensagem->semCount;
}