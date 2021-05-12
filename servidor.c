/******************************************************************************
 ** ISCTE-IUL: Trabalho prático 3 de Sistemas Operativos
 **
 ** Aluno: Nº:       Nome: 
 ** Nome do Módulo: servidor.c v2
 ** Descrição/Explicação do Módulo: 
 **
 **
 ******************************************************************************/
#include "common.h"
#include "utils.h"
#include <sys/stat.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>

/* Variáveis globais */
int msg_id;           // ID da Fila de Mensagens IPC usada
int sem_id;           // ID do array de Semáforos IPC usado
int shm_id;           // ID da Memória Partilhada IPC usada
Database *db;         // Database utilizada, que estará em Memória Partilhada
MsgCliente mensagem;  // Variável que tem a mensagem enviada do Cidadao para o Servidor
MsgServidor resposta; // Variável que tem a mensagem de resposta enviadas do Servidor para o Cidadao
int vaga_ativa;       // Índice da BD de Vagas que foi reservado pela função reserva_vaga()

/* Protótipos de funções */
void init_ipc();                     // Função a ser implementada pelos alunos
void init_database();                // Função a ser implementada pelos alunos
void espera_mensagem_cidadao();      // Função a ser implementada pelos alunos
void trata_mensagem_cidadao();       // Função a ser implementada pelos alunos
void envia_resposta_cidadao();       // Função a ser implementada pelos alunos
void processa_pedido();              // Função a ser implementada pelos alunos
void vacina();                       // Função a ser implementada pelos alunos
void cancela_pedido();               // Função a ser implementada pelos alunos
void servidor_dedicado();            // Função a ser implementada pelos alunos
int reserva_vaga(int, int);          // Função a ser implementada pelos alunos
void liberta_vaga(int);              // Função a ser implementada pelos alunos
void termina_servidor(int);          // Função a ser implementada pelos alunos
void termina_servidor_dedicado(int); // Função a ser implementada pelos alunos

int main()
{                                     // Não é suposto que os alunos alterem nada na função main()
    signal(SIGINT, termina_servidor); // Se receber <CTRL+C>, chama a função que termina o Servidor
    signal(SIGCHLD, SIG_IGN);
    // S1) Chama a função init_ipc(), que tenta criar uma fila de mensagens que tem a KEY IPC_KEY definida em common.h (alterar esta KEY para ter o valor do nº do aluno, como indicado nas aulas). Deve assumir que a fila de mensagens já foi criada. Se tal não aconteceu, dá erro e termina com exit status 1. Esta função, em caso de sucesso, preenche a variável global msg_id;
    init_ipc();
    // S2) Chama a função init_database(), que inicia a base de dados
    init_database();
    while (TRUE)
    {
        // S3) Chama a função espera_mensagem_cidadao(), que espera uma mensagem (na fila de mensagens com o tipo = 1) e preenche a mensagem enviada pelo processo Cidadão na variável global mensagem; em caso de erro, termina com erro e exit status 1;
        espera_mensagem_cidadao();
        // S4) O comportamento do processo Servidor agora irá depender da mensagem enviada pelo processo Cidadão no campo pedido:
        trata_mensagem_cidadao();
    }
}

/**
 * S1) Chama a função init_ipc(), que tenta criar:
 *     • uma fila de mensagens IPC;
 *     • um array de semáforos IPC de dimensão 1;
 *     • uma memória partilhada IPC de dimensão suficiente para conter um elemento Database.
 *     Todos estes elementos têm em comum serem criados com a KEY IPC_KEY definida em common.h (alterar esta KEY para ter o valor do nº do aluno, como indicado nas aulas), e com permissões 0600. 
 *     Se qualquer um destes elementos IPC já existia anteriormente, dá erro e termina com exit status 1. Esta função, em caso de sucesso, preenche as variáveis globais respetivas msg_id, sem_id, e shm_id;
 *     O semáforo em questão será usado com o padrão “Mutex”, pelo que será iniciado com o valor 1;
 */
void init_ipc()
{
    debug("<");

    msg_id = msgget(IPC_KEY, IPC_EXCL | 0600);
    exit_on_error(msg_id, "init_ipc) Fila de Mensagens com a Key definida já existe ou não pode ser criada");

    sem_id = semget(IPC_KEY, 1, IPC_EXCL | 0600);
    exit_on_error(sem_id, "init_ipc) Semáforo com a Key definida já existe ou não pode ser criada");

    int status = semctl(sem_id, 0, SETVAL, 1);
    exit_on_error(status, "init_ipc) Semáforo com a Key definida não pode ser iniciado com o valor 1");

    shm_id = shmget(IPC_KEY, sizeof(Database), IPC_EXCL | 0600);
    exit_on_error(shm_id, "init_ipc) Memória Partilhada com a Key definida já existe ou não pode ser criada");

    sucesso("S1) Criados elementos IPC com a Key %x: MSG %d, SEM %d, SHM %d", IPC_KEY, msg_id, sem_id, shm_id);
    debug(">");
}

/**
 * Lê um ficheiro binário
 * @param   filename    Nome do ficheiro a ler
 * @param   buffer      Ponteiro para o buffer onde armazenar os dados
 * @param   maxsize     Tamanho máximo do ficheiro a ler
 * @return              Número de bytes lidos, ou 0 em caso de erro
 */
size_t read_binary(char *filename, void *buffer, const size_t maxsize)
{
    struct stat st;
    // A função stat() preenche uma estrutura com dados do ficheiro, incluindo o tamanho do ficheiro.
    // stat() retorna -1 se erro
    exit_on_error(stat(filename, &st), "read_binary) Erro no cálculo do tamanho do ficheiro");
    // O tamanho do ficheiro é maior do que o tamanho do buffer alocado?
    if (st.st_size > maxsize)
        exit_on_error(-1, "read_binary) O buffer não tem espaço para o ficheiro");

    FILE *f = fopen(filename, "r");
    // fopen retorna NULL se erro
    exit_on_null(f, "read_binary) Erro na abertura do ficheiro");

    // fread está a ler st.st_size elementos, logo retorna um valor < st.st_size se erro
    if (fread(buffer, 1, st.st_size, f) < st.st_size)
        exit_on_error(-1, "read_binary) Erro na leitura do ficheiro");

    fclose(f);
    return st.st_size; // retorna o tamanho do ficheiro
}

/**
 * Grava um ficheiro binário
 * @param   filename    Nome do ficheiro a escrever
 * @param   buffer      Ponteiro para o buffer que contém os dados
 * @param   size        Número de bytes a escrever
 * @return              Número de bytes escrever, ou 0 em caso de erro
 */
size_t save_binary(char *filename, void *buffer, const size_t size)
{
    FILE *f = fopen(filename, "w");
    // fopen retorna NULL se erro
    exit_on_null(f, "save_binary) Erro na abertura do ficheiro");

    // fwrite está a escrever size elementos, logo retorna um valor < size se erro
    if (fwrite(buffer, 1, size, f) < size)
        exit_on_error(-1, "save_binary) Erro na escrita do ficheiro");

    fclose(f);
    return size;
}

/**
 * S2) Inicia a base de dados:
 *     • Associa a variável global db com o espaço de Memória Partilhada alocado para shm_id; se não o conseguir, dá erro e termina com exit status 1;
 *     • Lê o ficheiro FILE_CIDADAOS e armazena o seu conteúdo na base de dados usando a função read_binary(), assim preenchendo os campos db->cidadaos e db->num_cidadaos. Se não o conseguir, dá erro e termina com exit status 1;
 *     • Lê o ficheiro FILE_ENFERMEIROS e armazena o seu conteúdo na base de dados usando a função read_binary(), assim preenchendo os campos db->enfermeiros e db->num_enfermeiros. Se não o conseguir, dá erro e termina com exit status 1;
 *     • Inicia o array db->vagas, colocando todos os campos de todos os elementos com o valor -1.
 */
void init_database()
{
    debug("<");

    //shmat(shm_id, &db, 0);
    db = (Database *)shmat(shm_id, 0, 0);
    exit_on_null(db, "init_database) Erro a ligar a Memória Dinâmica ao projeto");

    int size = read_binary(FILE_CIDADAOS, db->cidadaos, sizeof(Cidadao));
    db->num_cidadaos = size / sizeof(Cidadao);

    size = read_binary(FILE_ENFERMEIROS, db->cidadaos, sizeof(Enfermeiro));
    db->num_enfermeiros = size / sizeof(Enfermeiro);

    for (int i = 0; i < MAX_VAGAS; i++)
        db->vagas[i] = (Vaga){-1, -1, -1};

    sucesso("S2) Base de dados carregada com %d cidadãos e %d enfermeiros", db->num_cidadaos, db->num_enfermeiros);
    debug(">");
}

/**
 * S3) Espera uma mensagem (na fila de mensagens com o tipo = 1) e preenche a mensagem enviada pelo processo Cidadão na variável global mensagem;
 *     em caso de erro, termina com erro e exit status 1;
 */
void espera_mensagem_cidadao()
{
    debug("<");

    int receiveStatus = msgrcv(msg_id, &mensagem, sizeof(MsgCliente), 1, 0);
    exit_on_error(receiveStatus, "Não é possível ler a mensagem do Cidadao");
    sucesso("Cidadão enviou mensagem");

    debug(">");
}

/**
 * S4) O comportamento do processo Servidor agora irá depender da variável global mensagem enviada pelo processo Cidadão no campo pedido
 */
void trata_mensagem_cidadao()
{
    debug("<");

    TipoPedido pedido = mensagem.dados.pedido;

    if (pedido == PEDIDO) 
    {
        sucesso("S4.1) Novo pedido de vacinação de %d para %d", mensagem.dados.PID_cidadao, mensagem.dados.num_utente);
        processa_pedido();
    }
    else if (pedido == CANCELAMENTO)
    {
        sucesso("S4.2) Cancelamento de vacinação de %d para %d", mensagem.dados.PID_cidadao, mensagem.dados.num_utente);
        debug(".");
        cancela_pedido();
    }

    debug(">");
}

/**
 * Estando a mensagem de resposta do processo Servidor na variável global resposta, envia essa mensagem para a fila de mensagens com o tipo = PID_Cidadao 
 */
void envia_resposta_cidadao()
{
    debug("<");

    resposta.tipo = resposta.dados.cidadao.PID_cidadao;
    int sendStatus = msgsnd(msg_id, &resposta, sizeof(MsgServidor), 0);
    exit_on_error(sendStatus, "Não é possível enviar resposta para o cidadão");
    sucesso("Resposta para o cidadão enviada");

    debug(">");
}

/**
 * S5) Processa um pedido de vacinação e envia uma resposta ao processo Cidadão. Para tal, essa função faz vários checks, atualizando o campo status da resposta:
 */
void processa_pedido()
{
    debug("<");
    int index_cidadao, index_enfermeiro = -1;

    int cidFound = 0;
    for (int i = 0; i < db->num_cidadaos; i++)
    {
        Cidadao cidadao = db->cidadaos[i];
        if (mensagem.dados.num_utente == cidadao.num_utente 
        && strcmp(mensagem.dados.nome, cidadao.nome))
        {
            cidFound = 1;
            index_cidadao = i;

            if (cidadao.estado_vacinacao == 2)
                resposta.dados.status = VACINADO;
            else if (cidadao.PID_cidadao > 0)
                resposta.dados.status = EMCURSO;
            else //TODO: Último ponto do S5.1?
                db->cidadaos[i].PID_cidadao = mensagem.dados.PID_cidadao;

            resposta.dados.cidadao = cidadao;

            sucesso("S5.1) Cidadão %d, %s encontrado, estado_vacinacao=%d, status=%d", 
                cidadao.num_utente, cidadao.nome, cidadao.estado_vacinacao, resposta.dados.status);

            break;
        }
    }

    if (cidFound == 0)
    {
        resposta.dados.status = DESCONHECIDO;
        erro("S5.1) Cidadão %d, %s  não foi encontrado na BD Cidadãos", mensagem.dados.num_utente, mensagem.dados.nome);
    }

    if (resposta.dados.status != DESCONHECIDO && resposta.dados.status != VACINADO && resposta.dados.status != EMCURSO)
    {
        int enfFound = 0;
        for (int i = 0; i < db->num_enfermeiros; i++)
        {
            Enfermeiro enfermeiro = db->enfermeiros[i];

            if (strcmp(enfermeiro.CS_enfermeiro, resposta.dados.cidadao.localidade))
            {
                enfFound = 1;
                
                if (enfermeiro.disponibilidade == 1)
                {
                    index_enfermeiro = i;

                    sucesso("S5.2) Enfermeiro do CS %s encontrado, disponibilidade=%d, status=%d", 
                        enfermeiro.CS_enfermeiro, enfermeiro.disponibilidade, resposta.dados.status);
                    
                    debug(".");

                    int vagaDisp = reserva_vaga(index_cidadao, index_enfermeiro);

                    if (vagaDisp != -1)
                    {
                        resposta.dados.status = OK;
                        sucesso("S5.3) Foi reservada a vaga %d para vacinação, status=%d", vagaDisp, resposta.dados.status);
                    }
                    else
                    {
                        resposta.dados.status = AGUARDAR;
                        erro("S5.3) Não foi encontrada nenhuma vaga livre para vacinação");
                    }
                }
                else if (enfermeiro.disponibilidade == 0)
                    resposta.dados.status = AGUARDAR;
            }

        }

        if (enfFound == 0)
        {
            resposta.dados.status = NAOHAENFERMEIRO;
            erro("S5.2) Enfermeiro do CS %s não foi encontrado na BD Cidadãos", resposta.dados.cidadao.localidade);
        }
    }
    
    if (OK == resposta.dados.status)
       vacina();

    debug(">");
}

/**
 * S6) Processa a vacinação
 */
void vacina()
{
    debug("<");

    int value = fork();

    if (value == -1)
        erro("S6.1) Não foi possível criar um novo processo");
    else if (value == 0) //Child
        servidor_dedicado();
    else //Parent
    {
        sucesso("S6.1) Criado um processo filho com PID_filho=%d", value);
        db->vagas[vaga_ativa].PID_filho = value;
    }

    debug(">");
}

/**
 * S7) Servidor Dedicado
 */
void servidor_dedicado()
{
    debug("<");

    signal(SIGTERM, termina_servidor_dedicado);
    envia_resposta_cidadao();

    db->enfermeiros[db->vagas[vaga_ativa].index_enfermeiro].disponibilidade = 0;
    sucesso("S7.3) Enfermeiro associado à vaga %d indisponível", vaga_ativa);

    Cidadao cidadao = db->cidadaos[db->vagas[vaga_ativa].index_cidadao];
    Enfermeiro enfermeiro = db->enfermeiros[db->vagas[vaga_ativa].index_enfermeiro];
    sucesso("S7.4) Vacina em curso para o cidadão %d, %s, e com o enfermeiro %d, %s na vaga %d", 
    cidadao.num_utente, cidadao.nome, enfermeiro.ced_profissional, enfermeiro.nome, vaga_ativa);
    sleep(TEMPO_CONSULTA);

    resposta.dados.status = TERMINADA;
    envia_resposta_cidadao();

    db->cidadaos[db->vagas[vaga_ativa].index_cidadao].estado_vacinacao = cidadao.estado_vacinacao + 1;
    db->enfermeiros[db->vagas[vaga_ativa].index_enfermeiro].nr_vacinas_dadas = enfermeiro.nr_vacinas_dadas + 1;
    db->enfermeiros[db->vagas[vaga_ativa].index_enfermeiro].disponibilidade = 1;

    cidadao = db->cidadaos[db->vagas[vaga_ativa].index_cidadao];
    enfermeiro = db->enfermeiros[db->vagas[vaga_ativa].index_enfermeiro];
    sucesso("S7.6) Cidadão atualizado na BD para estado_vacinacao=%d, Enfermeiro atualizado na BD para nr_vacinas_dadas=%d e disponibilidade=%d", 
    cidadao.estado_vacinacao, enfermeiro.nr_vacinas_dadas, enfermeiro.disponibilidade);

    liberta_vaga(vaga_ativa);
    exit(0);

    debug(">");
}

/**
 * S8) Tenta reservar uma vaga livre na BD de Vagas
 */
int reserva_vaga(int index_cidadao, int index_enfermeiro)
{
    debug("<");

    vaga_ativa = -1;
    for (int i = 0; i < MAX_VAGAS; i++)
    {
        if (db->vagas[i].index_cidadao < 0)
        {
            vaga_ativa = i;
            sucesso("S8.1.1) Encontrou uma vaga livre com o index %d", vaga_ativa);
            break;
        }
    }

    if (vaga_ativa != -1)
    {
        db->vagas[vaga_ativa].index_cidadao = index_cidadao;
        db->vagas[vaga_ativa].index_enfermeiro = index_enfermeiro;

        sucesso("S8.1.3) Foi reservada a vaga livre com o index %d", vaga_ativa);
    }
    else 
        erro("S8.1.3) Não foi encontrada nenhuma vaga livre");

    return vaga_ativa;

    debug(">");
}

/**
 * S9) Tenta libertar uma vaga na BD de Vagas, liberta a vaga da BD de Vagas, colocando o campo index_cidadao dessa entrada da BD de Vagas com o valor -1
 */
void liberta_vaga(int index_vaga)
{
    debug("<");

    //TODO: Confirmar que index_vaga é um índice válido?
    db->vagas[index_vaga].index_cidadao = -1;
    sucesso("S9) A vaga com o index %d foi libertada", index_vaga);

    debug(">");
}

void termina_servidor(int sinal) 
{
    for (int i = 0; i < MAX_VAGAS; i++)
        kill(db->vagas[i].PID_filho, SIGTERM);


    for (int i = 0; i < db->num_enfermeiros; i++)
        save_binary(FILE_ENFERMEIROS, db->enfermeiros, sizeof(Enfermeiro));
    
    for (int i = 0; i < db->num_cidadaos; i++)
        save_binary(FILE_CIDADAOS, db->cidadaos, sizeof(Cidadao));

    //TODO: NAO ESTA CERTO
    //ipcrm(shm_id);
    //ipcrm(sem_id);
    //ipcrm(msg_id);

    sucesso("S11.4) Servidor Terminado");
    exit(0);
}

void termina_servidor_dedicado(int sinal) 
{
    //TODO: CANCELADA EM VEZ DE CANCEL?
    resposta.dados.status = CANCEL;
    envia_resposta_cidadao();

    liberta_vaga(vaga_ativa);
    sucesso("S12.3) Servidor Dedicado Terminado");
    exit(0);
}
void cancela_pedido() 
{
    int PID_filho = -1;
    int index_vaga = -1;
    Cidadao cid;
    for (int i = 0; i < MAX_VAGAS; i++)
    {
        Vaga vaga = db->vagas[i];
        if (vaga.index_cidadao == -1) break;
        Cidadao cidadao = db->cidadaos[vaga.index_cidadao];

        if (mensagem.dados.num_utente == cidadao.num_utente 
        && strcmp(mensagem.dados.nome, cidadao.nome))
        {
            PID_filho = vaga.PID_filho;
            index_vaga = i;
            cid = cidadao;

            break;
        }
    }

    if (PID_filho != -1 && index_vaga != -1)
    {
        sucesso("S10.1) Foi encontrada a sessão do cidadão %d, %s na sala com o index %d", cid.num_utente, cid.nome, index_vaga);
        kill(PID_filho, SIGTERM);
        sucesso("S10.2) Enviado sinal SIGTERM ao Servidor Dedicado com PID=%d", PID_filho);
    }
    else
        erro("S10.1) Não foi encontrada nenhuma sessão de vacinação com o cidadão %d, %s", mensagem.dados.num_utente, mensagem.dados.nome);
}