/******************************************************************************
 ** ISCTE-IUL: Trabalho prático 3 de Sistemas Operativos
 **
 ** Aluno: Nº:       Nome: 
 ** Nome do Módulo: cidadao.c v3
 ** Descrição/Explicação do Módulo: 
 **
 **
 ******************************************************************************/
#include "common.h"
#include "utils.h"
#include <signal.h>

/* Variáveis globais */
int msg_id;             // ID da Fila de Mensagens usada
MsgCliente mensagem;    // Variável que tem a mensagem enviada do Cidadao para o Servidor
MsgServidor resposta;   // Variável que tem a mensagem de resposta enviadas do Servidor para o Cidadao

/* Protótipos de funções */
void init_ipc();                    // Função a ser implementada pelos alunos
void cria_mensagem();               // Função a ser implementada pelos alunos
void envia_mensagem_servidor();     // Função a ser implementada pelos alunos
void espera_resposta_servidor();    // Função a ser implementada pelos alunos
void trata_resposta_servidor();     // Função a ser implementada pelos alunos
void pedido();                      // Função a ser implementada pelos alunos
void vacina();                      // Função a ser implementada pelos alunos
void cancela_pedido(int);           // Função a ser implementada pelos alunos
void print_info(Cidadao cidadao);

int main() {    // Não é suposto que os alunos alterem nada na função main()
    // C1) Chama a função init_ipc(), que tenta abrir uma fila de mensagens IPC que tem a KEY IPC_KEY definida em common.h (alterar esta KEY para ter o valor do nº do aluno, como indicado nas aulas). Deve assumir que a fila de mensagens já foi criada. Se tal não aconteceu, dá erro e termina com exit status 1. Esta função, em caso de sucesso, preenche a variável global msg_id;
    init_ipc();
    // C2) Chama a função cria_mensagem()
    cria_mensagem();
    // C7) Arma e trata o sinal SIGINT para que, quando o utilizador interromper o processo Cidadão com <CTRL+C>, chame a função cancela_pedido()
    signal(SIGINT, cancela_pedido);
    // Faz o pedido ao servidor e aguarda a resposta do mesmo
    pedido();
    // C6) Inicia o processo de vacinação
    vacina();
}

/**
 * C1) tenta abrir uma fila de mensagens que tem a KEY IPC_KEY
 * definida em common.h (alterar esta KEY para ter o valor do nº do aluno, como indicado nas aulas).
 * Deve assumir que a fila de mensagens já foi criada.
 * Se tal não aconteceu, dá erro e termina com exit status 1.
 * Esta função, em caso de sucesso, preenche a variável global msg_id;
 */
void init_ipc() {
    debug("<");

    int msgId = msgget(IPC_KEY, IPC_EXCL);
    exit_on_error(1, "init_ipc) Fila de Mensagens com a Key definida não existe ou não pode ser aberta");
    sucesso("C1) Fila de Mensagens com a Key %x aberta com o ID %d", IPC_KEY, msg_id);
    msg_id = msgId;

    debug(">");
}

/**
 * C2.1) Pede ao Cidadão (utilizador) os seus dados, nomeadamente o número de utente e nome, obrigatoriamente nessa ordem, preenchendo os dados na variável global mensagem;
 * C2.2) Preenche os campos PID_cidadao da variável global mensagem com o PID deste processo Cidadão, tipo da mensagem com o tipo 1, e pedido = PEDIDO;
 */
void cria_mensagem() {
    debug("<");

    char num_utente[6];
    printf("Introduza o seu número de utente: ");
    my_gets(num_utente, 6);

    char nome[100];
    printf("Introduza o seu nome: ");
    my_gets(nome, 100);

    mensagem.dados.num_utente = atoi(num_utente);
    mensagem.dados.nome = nome;
    sucesso("C2.1) Dados Cidadão: %d, %s", atoi(num_utente), nome);

    debug(".");

    mensagem.tipo = 1;
    mensagem.dados.PID_cidadao = getpid();
    mensagem.dados.pedido = PEDIDO;
    sucesso("C2.2) PID Cidadão: %d", mensagem.dados.PID_cidadao);

    debug(">");
}

/**
 * Envia a mensagem para a fila de mensagens; em caso de erro no envio, termina com erro e exit status 1.
 */
void envia_mensagem_servidor() {
    debug("<");

    int sendStatus = msgsnd(msg_id, mensagem, sizeof(MsgCliente), 0);
    exit_on_error(sendStatus, "Não é possível enviar mensagem para o servidor");
    sucesso("Mensagem para o servidor enviada");

    debug(">");
}

/**
 * Espera a resposta do processo Servidor (na fila de mensagens com o tipo = PID_Cidadao) e preenche a mensagem enviada pelo processo Servidor na variável global resposta; 
 */
void espera_resposta_servidor() {
    debug("<");

    int receiveStatus = msgrcv(msg_id, resposta, sizeof(MsgServidor), getpid(), 0);
    exit_on_error(receiveStatus, "Não é possível ler a resposta do servidor");
    sucesso("Servidor enviou resposta");

    debug(">");
}

/**
 * Trata a resposta devolvida pelo servidor 
 */
int trata_resposta_servidor() {
    
    Cidadao cidadao = resposta.dados.cidadao;
    int status = 0;
    //debug(".") statements?
    switch (resposta.dados.status) {
        case DESCONHECIDO:
            erro("C5.1) Não existe registo do utente %d, %s", cidadao.num_utente, cidadao.nome);
            debug(".");
            exit(1);
            break;
        case VACINADO:
            sucesso("C5.2) O utente %d, %s foi vacinado", cidadao.num_utente, cidadao.nome);
            debug(".");
            exit(0);
            break;

        case EMCURSO:
            sucesso("C5.3) A vacinação do utente %d, %s já está em curso", cidadao.num_utente, cidadao.nome);
            debug(".");
            exit(0);
            break;

        case AGUARDAR:
            sucesso("C5.4) Utente %d, %s, por favor aguarde...", cidadao.num_utente, cidadao.nome);
            debug(".");
            sleep(TEMPO_ESPERA);
            break;

        case OK:
            sucesso("C5.5) Utente %d, %s, vai agora ser vacinado", cidadao.num_utente, cidadao.nome);
            debug(".");
            status = 1;
            break;

        default:
            break;
    }

    return status;
}

/**
 * Envia o pedido ao servidor e aguarda a sua resposta
 */
void pedido() {
    debug("<");

    StatusServidor status = DESCONHECIDO;
    do {
        // C3) Envia um pedido de consulta de vacinação para o processo Servidor, chamando a função envia_mensagem_servidor(), que envia uma mensagem para a fila de mensagens com tipo 1, com pedido = PEDIDO e os dados do cidadão; em caso de erro, termina com erro e exit status 1.
        envia_mensagem_servidor();
        // C4) Chama a função espera_resposta_servidor(), que espera a resposta do processo Servidor (na fila de mensagens com o tipo = PID_Cidadao) e preenche a mensagem enviada pelo processo Servidor na variável global resposta; em caso de erro, termina com erro e exit status 1.
        espera_resposta_servidor();
        // C5) O comportamento do processo Cidadão agora irá depender da resposta enviada pelo processo Servidor no campo status:
        if (trata_resposta_servidor() == 1) 
            status = OK;
    } while (OK != status);

    debug(">");
}

/**
 * Imprime informação sobre o utente
 * @param cidadao Cidadao a reportar
 **/
void print_info(Cidadao cidadao) {
    debug("<");
    printf("Número de utente: %d\n", cidadao.num_utente);
    printf("Nome            : %s\n", cidadao.nome);
    printf("Idade           : %d ano%s\n", cidadao.idade, cidadao.idade > 1 ? "s" : "");
    printf("Localidade      : %s\n", cidadao.localidade);
    printf("N.º Telemóvel   : %s\n", cidadao.nr_telemovel);
    printf("Vacina          : %dª dose\n", cidadao.estado_vacinacao + 1);
    debug(">");
}

/**
 * Espera a resposta do processo Servidor (na fila de mensagens com o tipo = PID_Cidadao) e preenche a mensagem enviada pelo processo Servidor na variável global resposta; 
 */
void vacina() {
    debug("<");

    print_info(resposta.dados.cidadao);
    sucesso("C6.1) Dados completos sobre o cidadão a ser vacinado");

    espera_resposta_servidor();

    StatusServidor status = resposta.dados.status;
    if (status == TERMINADA) {
        sucesso("C6.3.1) Utente %d, %s vacinado com sucesso", cidadao.num_utente, cidadao.nome);
        exit(0);
    } else if (status == CANCEL) {
        erro("C6.3.2) O servidor cancelou a vacinação em curso");
        exit(1);
    }

    debug(">");
}

/**
 * C7) Quando o utilizador interromper o processo Cidadão com <CTRL+C>:
 * C7.1) Escreve no ecrã uma mensagem;
 * C7.2) Envia para o processo Servidor uma mensagem (com o tipo 1), com pedido = CANCELAMENTO e os dados do cidadão;
 * C7.3) Chama novamente a função espera_resposta_servidor(), que espera uma nova resposta do processo Servidor (na fila de mensagens com o tipo = PID_Cidadao) e preenche a mensagem enviada pelo processo Servidor na variável global resposta;
 * C7.4) O comportamento do processo Cidadão agora irá depender da resposta enviada pelo processo Servidor no campo status:
 * C7.4.1) Se o status for CANCEL, imprime uma mensagem de erro, e termina com exit status 1;
 * C7.4.2) Se o status for TERMINADA, imprime mensagem sucesso, termina com exit status 0;
 */
void cancela_pedido(int sinal) {
    debug("<");

    sucesso("C7.1) O cidadão cancelou a vacinação no processo %d", getpid());

    debug(".");

    mensagem.dados.pedido = CANCELAMENTO;
    envia_mensagem_servidor();

    debug(".");
    
    espera_resposta_servidor();

    debug(".");

    StatusServidor status = resposta.dados.status;
    if (status == CANCEL) {
        sucesso("C7.4.1) Servidor confirmou cancelamento");
        exit(0);
    } else if (status == TERMINADA) {
        sucesso("C7.4.2) A vacinação já tinha sido concluída");
        exit(0);
    }

    debug(">");
}