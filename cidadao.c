/******************************************************************************
 ** ISCTE-IUL: Trabalho prático de Sistemas Operativos
 **
 ** Aluno: Nº:       Nome: 
 ** Nome do Módulo: cidadao.c
 ** Descrição/Explicação do Módulo: 
 **
 **
 ******************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include "common.h"
#include "utils.h"

#define my_gets(buffer, buffer_size) my_fgets(buffer, buffer_size, stdin)

Cidadao pedirInput()
{
    Cidadao cidadao;

    char num_utente[6];
    printf("Introduza o seu número de utente: ");
    my_gets(num_utente, 6);
    sscanf(num_utente, "%d", &cidadao.num_utente);

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

void iniciarVacina(Cidadao cidadao) 
{
    if (access("pedidovacina.txt", F_OK) == 0) //File already exists
    {
        erro("C3) Não é possível iniciar o processo de vacinação neste momento");
        exit(0);
    }
    else sucesso("C3) Ficheiro FILE_PEDIDO_VACINA pode ser criado");

    FILE *pv = fopen("pedidovacina.txt", "w");
    if (pv != NULL)
    {
        fprintf(pv, "%d:%s:%d:%s:%s:%d:%d", cidadao.num_utente, cidadao.nome, cidadao.idade,
                cidadao.localidade, cidadao.nr_telemovel, cidadao.estado_vacinacao, cidadao.PID_cidadao);

        sucesso("C4) Ficheiro FILE_PEDIDO_VACINA criado e preenchido");
    }
    else erro("C4) Não é possível criar o ficheiro FILE_PEDIDO_VACINA");
    fclose(pv);
}

void handleSIGINT(int signal)
{
    sucesso("C5) O cidadão cancelou a vacinação, o pedido nº%d foi cancelado", getpid());
    remove("pedidovacina.txt");
    exit(0);
}

void handleSIGUSRone(int signal)
{
    sucesso("C7) Vacinação do cidadão com o pedido no <PID Cidadão> em curso");
    remove("pedidovacina.txt");
}

void handleSIGUSRtwo(int signal)
{
    sucesso("C8) Vacinação do cidadão com o pedido nº%d concluída", getpid());
    exit(0);
}

void handleSIGTERM(int signal)
{
    sucesso("C9) Não é possível vacinar o cidadão no pedido nº%d", getpid());
    remove("pedidovacina.txt");
    exit(0);
}

int lerSPID()
{
    if (access("servidor.pid", F_OK) == 0)
    {
        char PID_Servidor[10];
        FILE *spid = fopen("servidor.pid", "r");
        if (spid != NULL)
            my_fgets(PID_Servidor, 1024, spid);
        fclose(spid);

        kill(atoi(PID_Servidor), SIGUSR1);
        sucesso("C6) Sinal enviado ao Servidor: %s", PID_Servidor);
    }
    else erro("C6) Não existe ficheiro FILE_PID_SERVIDOR!");
}

int main()
{
    Cidadao cidadao = pedirInput();
    iniciarVacina(cidadao);
    signal(SIGINT, handleSIGINT);
    signal(SIGUSR1, handleSIGUSRone);
    signal(SIGUSR2, handleSIGUSRtwo);
    signal(SIGTERM, handleSIGTERM);
}
