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

/* Variáveis globais */
int msg_id;             // ID da Fila de Mensagens IPC usada
int sem_id;             // ID do array de Semáforos IPC usado
int shm_id;             // ID da Memória Partilhada IPC usada
Database* db;           // Database utilizada, que estará em Memória Partilhada
MsgCliente mensagem;    // Variável que tem a mensagem enviada do Cidadao para o Servidor
MsgServidor resposta;   // Variável que tem a mensagem de resposta enviadas do Servidor para o Cidadao
int vaga_ativa;         // Índice da BD de Vagas que foi reservado pela função reserva_vaga()

/* Protótipos de funções */
void init_ipc();                    // Função a ser implementada pelos alunos
void init_database();               // Função a ser implementada pelos alunos
void espera_mensagem_cidadao();     // Função a ser implementada pelos alunos
void trata_mensagem_cidadao();      // Função a ser implementada pelos alunos
void envia_resposta_cidadao();      // Função a ser implementada pelos alunos
void cria_pedido();                 // Função a ser implementada pelos alunos
void vacina();                      // Função a ser implementada pelos alunos
void cancela_pedido();              // Função a ser implementada pelos alunos
void servidor_dedicado();           // Função a ser implementada pelos alunos
int reserva_vaga(int, int);         // Função a ser implementada pelos alunos
void liberta_vaga(int);             // Função a ser implementada pelos alunos
void termina_servidor(int);         // Função a ser implementada pelos alunos
void termina_servidor_dedicado(int);// Função a ser implementada pelos alunos

int main() {    // Não é suposto que os alunos alterem nada na função main()
    signal(SIGINT, termina_servidor);   // Se receber <CTRL+C>, chama a função que termina o Servidor
    signal(SIGCHLD, SIG_IGN);
    // S1) Chama a função init_ipc(), que tenta criar uma fila de mensagens que tem a KEY IPC_KEY definida em common.h (alterar esta KEY para ter o valor do nº do aluno, como indicado nas aulas). Deve assumir que a fila de mensagens já foi criada. Se tal não aconteceu, dá erro e termina com exit status 1. Esta função, em caso de sucesso, preenche a variável global msg_id;
    init_ipc();
    // S2) Chama a função init_database(), que inicia a base de dados
    init_database();
    while (TRUE) {
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
 *     Todos estes elementos têm em comum serem criados com a KEY IPC_KEY definida em common.h (alterar esta KEY para ter o valor do nº do aluno, como indicado nas aulas), e com permissões 0600. Se qualquer um destes elementos IPC já existia anteriormente, dá erro e termina com exit status 1. Esta função, em caso de sucesso, preenche as variáveis globais respetivas msg_id, sem_id, e shm_id;
 *     O semáforo em questão será usado com o padrão “Mutex”, pelo que será iniciado com o valor 1;
 */
void init_ipc() {
    debug("<");

    // S1) Tenta criar:
    // Todos estes elementos têm em comum serem criados com a KEY IPC_KEY definida em common.h (alterar esta KEY para ter o valor do nº do aluno, como indicado nas aulas), e com permissões 0600. Se qualquer um destes elementos IPC já existia anteriormente, dá erro e termina com exit status 1. Esta função, em caso de sucesso, preenche as variáveis globais respetivas msg_id, sem_id, e shm_id;
    // • uma fila de mensagens IPC;
    // exit_on_error(<var>, "init_ipc) Fila de Mensagens com a Key definida já existe ou não pode ser criada");

    debug(".");
    // • um array de semáforos IPC de dimensão 1;
    // exit_on_error(<var>, "init_ipc) Semáforo com a Key definida já existe ou não pode ser criada");

    debug(".");
    // O semáforo em questão será usado com o padrão “Mutex”, pelo que será iniciado com o valor 1;
    // exit_on_error(<var>, "init_ipc) Semáforo com a Key definida não pode ser iniciado com o valor 1");

    debug(".");
    // • uma memória partilhada IPC de dimensão suficiente para conter um elemento Database.
    // exit_on_error(<var>, "init_ipc) Memória Partilhada com a Key definida já existe ou não pode ser criada");

    // sucesso("S1) Criados elementos IPC com a Key %x: MSG %d, SEM %d, SHM %d", IPC_KEY, msg_id, sem_id, shm_id);
    debug(">");
}

/**
 * Lê um ficheiro binário
 * @param   filename    Nome do ficheiro a ler
 * @param   buffer      Ponteiro para o buffer onde armazenar os dados
 * @param   maxsize     Tamanho máximo do ficheiro a ler
 * @return              Número de bytes lidos, ou 0 em caso de erro
 */
size_t read_binary( char* filename, void* buffer, const size_t maxsize ) {
    struct stat st;
    // A função stat() preenche uma estrutura com dados do ficheiro, incluindo o tamanho do ficheiro.
    // stat() retorna -1 se erro
    exit_on_error(stat(filename, &st), "read_binary) Erro no cálculo do tamanho do ficheiro");
    // O tamanho do ficheiro é maior do que o tamanho do buffer alocado?
    if (st.st_size > maxsize)
        exit_on_error(-1, "read_binary) O buffer não tem espaço para o ficheiro");

    FILE* f = fopen(filename, "r");
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
size_t save_binary(char* filename, void* buffer, const size_t size) {
    FILE* f = fopen(filename, "w");
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
void init_database() {
    debug("<");

    // S2) Inicia a base de dados:
    // • Associa a variável global db com o espaço de Memória Partilhada alocado para shm_id; se não o conseguir, dá erro e termina com exit status 1;
    // exit_on_null(<var>, "init_database) Erro a ligar a Memória Dinâmica ao projeto");

    debug(".");
    // • Lê o ficheiro FILE_CIDADAOS e armazena o seu conteúdo na base de dados usando a função read_binary(), assim preenchendo os campos db->cidadaos e db->num_cidadaos. Se não o conseguir, dá erro e termina com exit status 1;
 
    debug(".");
    // • Lê o ficheiro FILE_ENFERMEIROS e armazena o seu conteúdo na base de dados usando a função read_binary(), assim preenchendo os campos db->enfermeiros e db->num_enfermeiros. Se não o conseguir, dá erro e termina com exit status 1;
 
    debug(".");
    // • Inicia o array db->vagas, colocando todos os campos de todos os elementos com o valor -1.

    // sucesso("S2) Base de dados carregada com %d cidadãos e %d enfermeiros", <num_cidadaos>, <num_enfermeiros>);
    debug(">");
}

/**
 * S3) Espera uma mensagem (na fila de mensagens com o tipo = 1) e preenche a mensagem enviada pelo processo Cidadão na variável global mensagem;
 *     em caso de erro, termina com erro e exit status 1;
 */
void espera_mensagem_cidadao() {
    debug("<");

    // S3) Espera uma mensagem (na fila de mensagens com o tipo = 1) e preenche a mensagem enviada pelo processo Cidadão na variável global mensagem; em caso de erro, termina com erro e exit status 1;
    // Outputs esperados (itens entre <> substituídos pelos valores correspondentes):
    // exit_on_error(<var>, "Não é possível ler a mensagem do Cidadao");
    // sucesso("Cidadão enviou mensagem");

    debug(">");
}

/**
 * S4) O comportamento do processo Servidor agora irá depender da variável global mensagem enviada pelo processo Cidadão no campo pedido
 */
void trata_mensagem_cidadao() {
    debug("<");

    // if (...) {
        // S4.1) Se o pedido for PEDIDO, imprime uma mensagem e avança para o passo S5;
        // Outputs esperados (itens entre <> substituídos pelos valores correspondentes):
        // sucesso("S4.1) Novo pedido de vacinação de %d para %s", <PID_cidadao>, <num_utente>);
        debug(".");
        cria_pedido();
    // } else if (...) {
        // S4.2) Se o estado for CANCELAMENTO, imprime uma mensagem, e avança para o passo S10;
        // Outputs esperados (itens entre <> substituídos pelos valores correspondentes):
        // sucesso("S4.2) Cancelamento de vacinação de %d para %s", <PID_cidadao>, <num_utente>);
        debug(".");
        cancela_pedido();
    // }

    debug(">");
}

/**
 * Estando a mensagem de resposta do processo Servidor na variável global resposta, envia essa mensagem para a fila de mensagens com o tipo = PID_Cidadao 
 */
void envia_resposta_cidadao() {
    debug("<");

    // Outputs esperados (itens entre <> substituídos pelos valores correspondentes):
    // exit_on_error(<var>, "Não é possível enviar resposta para o cidadão");
    // sucesso("Resposta para o cidadão enviada");

    debug(">");
}

/**
 * S5) Processa um pedido de vacinação e envia uma resposta ao processo Cidadão. Para tal, essa função faz vários checks, atualizando o campo status da resposta:
 */
void cria_pedido() {
    debug("<");

    // S5.1) Procura o num_utente e nome na base de dados (BD) de Cidadãos:
    //       • Se o num_utente e nome do utilizador for encontrado na BD Cidadãos, os dados do cidadão deverão ser copiados da BD Cidadãos para o campo cidadao da resposta;
    //       • Se o utilizador (Cidadão) não for encontrado na BD Cidadãos => status = DESCONHECIDO;
    //       • Se o Cidadão na BD Cidadãos tiver estado_vacinacao >= 2 => status = VACINADO;
    //       • Se o Cidadão na BD Cidadãos tiver estado_vacinacao < 0 => status = EMCURSO.
    // Outputs esperados (itens entre <> substituídos pelos valores correspondentes):
    // erro("S5.1) Cidadão %d, %s  não foi encontrado na BD Cidadãos", <num_utente>, <nome>);
    // sucesso("S5.1) Cidadão %d, %s encontrado, estado_vacinacao=%d, status=%d", <num_utente>, <nome>, <estado_vacinacao>, <status>);

    debug(".");
    // S5.2) Caso o Cidadão esteja em condições de ser vacinado (i.e., se status não for DESCONHECIDO, VACINADO nem EMCURSO), procura o enfermeiro correspondente na BD Enfermeiros:
    //       • Se não houver centro de saúde, ou não houver nenhum enfermeiro no centro de saúde correspondente  status = NAOHAENFERMEIRO;
    //       • Se há enfermeiro, mas este não tiver disponibilidade => status = AGUARDAR.
    // Outputs esperados (itens entre <> substituídos pelos valores correspondentes):
    // erro("S5.2) Enfermeiro do CS %s não foi encontrado na BD Cidadãos", <localidade>);
    // sucesso("S5.2) Enfermeiro do CS %s encontrado, disponibilidade=%d, status=%d", <localidade>, <disponibilidade>, <status>);

    debug(".");
    // S5.3) Caso o enfermeiro esteja disponível, procura uma vaga para vacinação na BD Vagas. Para tal, chama a função reserva_vaga(Index_Cidadao, Index_Enfermeiro) usando os índices do Cidadão e do Enfermeiro nas respetivas BDs:
    //      • Se essa função tiver encontrado e reservado uma vaga => status = OK;
    //      • Se essa função não conseguiu encontrar uma vaga livre => status = AGUARDAR.
    // Outputs esperados (itens entre <> substituídos pelos valores correspondentes):
    // erro("S5.3) Não foi encontrada nenhuma vaga livre para vacinação");
    // sucesso("S5.3) Foi reservada a vaga %d para vacinação, status=%d", <index_vaga>, <status>);

    debug(".");
    // S5.4) Se no final de todos os checks, status for OK, chama a função vacina().
    // if (OK == <status>)
    //    vacina();

    debug(">");
}

/**
 * S6) Processa a vacinação
 */
void vacina() {
    debug("<");

    // S6.1) Cria um processo filho através da função fork();
    // Outputs esperados (itens entre <> substituídos pelos valores correspondentes):
    // erro("S6.1) Não foi possível criar um novo processo");
    // sucesso("S6.1) Criado um processo filho com PID_filho=%d", <PID_filho>);

    debug(".");
    // if (...) {   // Processo FILHO
        // S6.2) O processo filho chama a função servidor_dedicado();
        // servidor_dedicado();
    // } else {     // Processo PAI
        debug(".");
        // S6.3) O processo pai regista o process ID do processo filho no campo PID_filho na BD de Vagas com o índice da variável global vaga_ativa;
    //}

    debug(">");
}

/**
 * S7) Servidor Dedicado
 */
void servidor_dedicado() {
    debug("<");

    // S7.1) Arma o sinal SIGTERM;
    signal(SIGTERM, termina_servidor_dedicado);

    debug(".");
    // S7.2) Envia a resposta para o Cidadao, chamando a função envia_resposta_cidadao(). Implemente também esta função, que envia a mensagem resposta para o cidadao, contendo os dados do Cidadao preenchidos em S5.1 e o campo status = OK;
    envia_resposta_cidadao();

    debug(".");
    // S7.3) Coloca a disponibilidade do enfermeiro afeto à vaga_ativa com o valor 0 (Indisponível);
    // Outputs esperados (itens entre <> substituídos pelos valores correspondentes):
    // sucesso("S7.3) Enfermeiro associado à vaga %d indisponível", <vaga_ativa>);

    debug(".");
    // S7.4) Imprime uma mensagem;
    // Outputs esperados (itens entre <> substituídos pelos valores correspondentes):
    // sucesso("S7.4) Vacina em curso para o cidadão %d, %s, e com o enfermeiro %d, %s na vaga %d", <num_utente>, <nome cidadao>, <ced_profissional>, <nome enfermeiro>, <vaga_ativa>);
    // S7.4) Aguarda (em espera passiva!) TEMPO_CONSULTA segundos;

    debug(".");
    // S7.5) Envia nova resposta para o Cidadao, chamando a função envia_resposta_cidadao() contendo os dados do Cidadao preenchidos em S5.1 e o campo status = OK, para indicar que a consulta terminou com sucesso;
    envia_resposta_cidadao();

    debug(".");
    // S7.6) Atualiza os dados do cidadão (estado_vacinacao) na BD de Cidadãos
    // S7.6) Atualiza os dados do enfermeiro (incrementa nr_vacinas_dadas) na BD de Enfermeiros;
    // S7.6) Atualiza os dados do enfermeiro (coloca disponibilidade=1) na BD de Enfermeiros;
    // Outputs esperados (itens entre <> substituídos pelos valores correspondentes):
    // sucesso("S7.6) Cidadão atualizado na BD para estado_vacinacao=%d, Enfermeiro atualizado na BD para nr_vacinas_dadas=%d e disponibilidade=%d", <estado_vacinacao>, <nr_vacinas_dadas>, <disponibilidade>);

    debug(".");
    // S7.7) Liberta a vaga vaga_ativa da BD de Vagas, invocando a função liberta_vaga(vaga_ativa);
    liberta_vaga(vaga_ativa);

    debug(".");
    // S7.8) Termina o processo Servidor Dedicado (filho) com exit status 0.
    exit(0);

    debug(">");
}

/**
 * S8) Tenta reservar uma vaga livre na BD de Vagas
 */
int reserva_vaga(int index_cidadao, int index_enfermeiro) {
    debug("<");

    vaga_ativa = -1;
    // S8.1) Procura uma vaga livre (index_cidadao < 0) na BD de Vagas. Se encontrar uma entrada livre:

    debug(".");
    // S8.1.1) Atualiza o valor da variável global vaga_atual com o índice da vaga encontrada;
    // Outputs esperados (itens entre <> substituídos pelos valores correspondentes):
    // sucesso("S8.1.1) Encontrou uma vaga livre com o index %d", <vaga_ativa>);

    debug(".");
    // S8.1.2) Atualiza a entrada de Vagas vaga_ativa com o índice do cidadão e do enfermeiro

    debug(".");
    // S8.1.3) Retorna o valor do índice de vagas vaga_ativa ou -1 se não encontrou nenhuma vaga
    return vaga_ativa;

    debug(">");
}

/**
 * S9) Tenta libertar uma vaga na BD de Vagas, liberta a vaga da BD de Vagas, colocando o campo index_cidadao dessa entrada da BD de Vagas com o valor -1
 */
void liberta_vaga(int index_vaga) {
    debug("<");

    //  S9) Tenta libertar uma vaga na BD de Vagas, liberta a vaga da BD de Vagas, colocando o campo index_cidadao dessa entrada da BD de Vagas com o valor -1

    debug(">");
}

void termina_servidor(int sinal) {}         // Função a ser implementada pelos alunos
void termina_servidor_dedicado(int sinal) {} // Função a ser implementada pelos alunos
void cancela_pedido() {}              // Função a ser implementada pelos alunos