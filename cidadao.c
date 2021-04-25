/******************************************************************************
 ** ISCTE-IUL: Trabalho prático de Sistemas Operativos
 **
 ** Aluno: Nº: 98420   Nome: Rodrigo Martins 
 ** Nome do Módulo: cidadao.c
 ** Descrição/Explicação do Módulo: 
 **
 **   O módulo começa por pedir ao utilizador as suas informações pessoais para
 ** iniciar o processo de vacinação (onde vai guardar essas informações num 
 ** ficheiro de texto). Depois disso, armamos e tratamos os sinais necessários
 ** para o servidor conseguir informar o cidadão do progresso da vacinação
 **
 ******************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include "common.h"
#include "utils.h"

#define my_gets(buffer, buffer_size) my_fgets(buffer, buffer_size, stdin)

int canSIGINT = 0;
int fileExists = 0;

Cidadao pedirInput()
{
    // #ifdef DEBUG_MODE
    //     return (Cidadao){98420, "Rodrigo Martins", 19, "CSLisboa", "934466374", 0, getpid()};
    // #endif

    Cidadao cidadao;

    char num_utente[6];
    printf("Introduza o seu número de utente: ");
    my_gets(num_utente, 6);
    cidadao.num_utente = atoi(num_utente);

    printf("Introduza o seu nome: ");
    my_gets(cidadao.nome, 100);

    char idade[4];
    printf("Introduza a sua idade: ");
    my_gets(idade, 4);
    sscanf(idade, "%d", &cidadao.idade);

    printf("Introduza a sua localidade: ");
    my_gets(cidadao.localidade, 100);

    printf("Introduza o seu número de telemóvel: ");
    my_gets(cidadao.nr_telemovel, 10);

    cidadao.estado_vacinacao = 0;
    cidadao.PID_cidadao = getpid();

    sucesso("C1) Dados Cidadão: %d; %s; %d; %s; %s; %d;\n", cidadao.num_utente, cidadao.nome,
            cidadao.idade, cidadao.localidade, cidadao.nr_telemovel, cidadao.estado_vacinacao);

    sucesso("C2) PID Cidadão: %d\n", cidadao.PID_cidadao);

    return cidadao;
}

void handleSIGALRM(int signal)
{
    if (access(FILE_PEDIDO_VACINA, F_OK) != 0)
    {
        sucesso("C3) Ficheiro FILE_PEDIDO_VACINA pode ser criado");
        fileExists = 0;
        return;
    }
    else
        erro("C3) Não é possível iniciar o processo de vacinação neste momento");

    alarm(5);
}

void iniciarVacina(Cidadao cidadao)
{
    if (access(FILE_PEDIDO_VACINA, F_OK) == 0)
    {
        erro("C3) Não é possível iniciar o processo de vacinação neste momento");

        signal(SIGALRM, handleSIGALRM);
        alarm(5);
        fileExists = 1;
    }
    else
        sucesso("C3) Ficheiro FILE_PEDIDO_VACINA pode ser criado");

    while (fileExists == 1)
        pause();

    FILE *pv = fopen(FILE_PEDIDO_VACINA, "w");
    if (pv != NULL)
    {
        fprintf(pv, "%d:%s:%d:%s:%s:%d:%d", cidadao.num_utente, cidadao.nome, cidadao.idade,
                cidadao.localidade, cidadao.nr_telemovel, cidadao.estado_vacinacao, cidadao.PID_cidadao);

        sucesso("C4) Ficheiro FILE_PEDIDO_VACINA criado e preenchido");
    }
    else
        erro("C4) Não é possível criar o ficheiro FILE_PEDIDO_VACINA");
    fclose(pv);

    canSIGINT = 1;
}

void handleSIGINT(int signal)
{
    if (canSIGINT == 0)
        return;
    sucesso("C5) O cidadão cancelou a vacinação, o pedido nº%d foi cancelado", getpid());
    remove(FILE_PEDIDO_VACINA);
    exit(0);
}

void handleSIGUSRone(int signal)
{
    sucesso("C7) Vacinação do cidadão com o pedido nº%d em curso", getpid());
    remove(FILE_PEDIDO_VACINA);
}

void handleSIGUSRtwo(int signal)
{
    sucesso("C8) Vacinação do cidadão com o pedido nº%d concluída", getpid());
    exit(0);
}

void handleSIGTERM(int signal)
{
    sucesso("C9) Não é possível vacinar o cidadão no pedido nº%d", getpid());
    remove(FILE_PEDIDO_VACINA);
    exit(0);
}

void handleSignals()
{
    signal(SIGUSR1, handleSIGUSRone);
    signal(SIGUSR2, handleSIGUSRtwo);
    signal(SIGTERM, handleSIGTERM);
}

int lerSPID()
{
    if (access("servidor.pid", F_OK) == 0)
    {
        char PID_Servidor[10];
        FILE *spid = fopen("servidor.pid", "r");
        if (spid != NULL)
            my_fgets(PID_Servidor, 10, spid);
        fclose(spid);

        kill(atoi(PID_Servidor), SIGUSR1);
        sucesso("C6) Sinal enviado ao Servidor: %s", PID_Servidor);
    }
    else
        erro("C6) Não existe ficheiro FILE_PEDIDO_VACINA!");
}

int main()
{
    signal(SIGINT, handleSIGINT);
    Cidadao cidadao = pedirInput();
    iniciarVacina(cidadao);
    handleSignals();
    lerSPID();

    while (1)
        pause();
}