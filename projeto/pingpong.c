#include "pingpong.h"

int taskId;
task_t taskMain;
task_t *taskCurrent;
task_t dispatcher;
task_t *tasksReady;
task_t *tasksSuspended;

task_t* scheduler()
{
    return tasksReady;
}

void dispatcher_body(void *arg)
{
    while(queue_size((queue_t*)tasksReady) > 0)
    {
        task_t* next = scheduler();
        if(next)
        {
            // Trocas de contexto
            task_switch(next);
            if(next->status == Ready)
            {
                queue_remove((queue_t **) &tasksReady, (queue_t *) next);
                queue_append((queue_t **) &tasksReady, (queue_t *) next);
            }

            else if(next->status == Ended)
            {
                queue_remove((queue_t **) &tasksReady, (queue_t *) next);
            }

            else
            {
                return;
            }
            
        }
    }
    task_exit(0);

    // while (userTasks > 0)
    // {
    //     next = scheduler(); // scheduler é uma função
    //     if (next)
    //     {
    //         ...                    // ações antes de lançar a tarefa "next", se houverem
    //             task_switch(next); // transfere controle para a tarefa "next"
    //         ...                    // ações após retornar da tarefa "next", se houverem
    //     }
    // }
    // task_exit(0); // encerra a tarefa dispatcher
}

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
    queue_remove((queue_t **) &tasksReady, (queue_t *) &dispatcher);

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

    taskCurrent->status = Ended;

    task_switch(taskCurrent->controle);
}

// alterna a execução para a tarefa indicada
int task_switch(task_t *task)
{
#ifdef DEBUG
    printf("task_switch: trocando contexto %d -> %d\n", taskCurrent->tid, task->tid);
#endif
    // ucontext_t *contextCurrent = &taskCurrent->context;
    
    task_t *taskPrevious = taskCurrent;
    taskCurrent = task;

    swapcontext(&taskPrevious->context, &task->context);

    return 0;
}

// retorna o identificador da tarefa corrente (main eh 0)
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
    if(task == NULL)
    {
        task = taskCurrent;
    }

    if(queue == NULL)
    {
        return;
    }

    queue_remove((queue_t **) &tasksReady, (queue_t *) task);
    queue_append((queue_t **) &tasksSuspended, (queue_t *) task);

    task->status = Suspended;
}

// acorda uma tarefa, retirando-a de sua fila atual, adicionando-a à fila de
// tarefas prontas ("ready queue") e mudando seu estado para "pronta"
void task_resume(task_t *task)
{
    queue_remove((queue_t **) &tasksSuspended, (queue_t *) task);
    queue_append((queue_t **) &tasksReady, (queue_t *) task);

    task->status = Ready;
}

void task_yield()
{
    task_switch(&dispatcher);
}