#include "pingpong.h"


int taskId;
task_t taskMain;
task_t* taskCurrent; 


// funções gerais ==============================================================

// Inicializa o sistema operacional; deve ser chamada no inicio do main()
void pingpong_init ()
{
    taskId = 0; 
    taskMain.tid = 0;
    taskId += 1;

    taskCurrent = &taskMain;

    /* desativa o buffer da saida padrao (stdout), usado pela função printf */
    setvbuf (stdout, 0, _IONBF, 0) ;
}

// gerência de tarefas =========================================================

// Cria uma nova tarefa. Retorna um ID> 0 ou erro.
// 1º parâmetro: descritor da nova tarefa
// 2º parâmetro: funcao corpo da tarefa
// 3º parâmetro: argumentos para a tarefa
int task_create (task_t *task, void (*start_func)(void *), void *arg)
{
    ucontext_t newContext;
    char *stack = malloc (STACKSIZE) ;

    // Salva o contexto atual na variável newContext
    getcontext (&newContext);

    if (stack)
    {
        newContext.uc_stack.ss_sp = stack ;
        newContext.uc_stack.ss_size = STACKSIZE;
        newContext.uc_stack.ss_flags = 0;
        newContext.uc_link = 0;
    }
    else
    {
        perror ("Erro na criacao da pilha: ");
        exit (-1);
    }

    // Ajusta alguns valores internos do contexto salvo em newContext
    makecontext(&newContext, (void*)(*start_func), 1, arg);

    task->context = newContext;
    task->tid = taskId;
    taskId++;

    #ifdef DEBUG
    printf("task_create: criou tarefa %d\n", task->tid);
    #endif

    return task->tid;
}

// Termina a tarefa corrente, indicando um valor de status encerramento
void task_exit (int exitCode)
{
    #ifdef DEBUG
    printf("task_exit: tarefa %d sendo encerrada\n", taskCurrent->tid);
    #endif
    task_switch(&taskMain);
}

// alterna a execução para a tarefa indicada
int task_switch (task_t *task)
{
    #ifdef DEBUG
    printf("task_switch: trocando contexto %d -> %d\n", taskCurrent->tid, task->tid);
    #endif
    ucontext_t *contextCurrent = &taskCurrent->context;
    taskCurrent = task;
    
    swapcontext(contextCurrent, &task->context);
    
    return 0;
}

// retorna o identificador da tarefa corrente (main eh 0)
int task_id ()
{
    // Todas funcoes devem gerar mensagens de depuracao, mas nao
    // sei se é necessário mensagem pra funcao que só mostra o id

    #ifdef DEBUG
    printf("task_id: tarefa tem id = %d\n", taskCurrent->tid);
    #endif
    return (taskCurrent->tid);
}