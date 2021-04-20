/******************************************************************************
 ** ISCTE-IUL: Trabalho prático de Sistemas Operativos
 **
 ** Aluno: Nº:       Nome: 
 ** Nome do Módulo: servidor.c
 ** Descrição/Explicação do Módulo: 
 **
 **
 ******************************************************************************/
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include "common.h"

Vaga vagas[NUM_VAGAS];
Enfermeiro *enfermeiros;

void registarServidor()
{
    FILE *servidor = fopen(FILE_PID_SERVIDOR, "w");
    if (servidor != NULL)
    {
        fprintf(servidor, "%d", getpid());
        sucesso("S1) Escrevi no ficheiro %s o PID: %d", FILE_PID_SERVIDOR, getpid());
    }
    else
        erro("S1) Não consegui registar o servidor!");
    fclose(servidor);
}

void lerEnfermeiros()
{
    Enfermeiro enf[sizeof(Enfermeiro)];
    FILE *enfData = fopen(FILE_ENFERMEIROS, "rb");
    if (enfData != NULL)
    {
        fseek(enfData, 0, SEEK_END);
        long fsize = ftell(enfData);
        int nr_enf = fsize / sizeof(Enfermeiro);
        enfermeiros = (Enfermeiro *)malloc(nr_enf * sizeof(Enfermeiro));

        //TODO: NOT READING ENF DATA PROPERLY HERE
        for (int i = 0; i < nr_enf; i++)
            if (fread(enf, sizeof(Enfermeiro), 1, enfData) == 1)
            {
                printf("NOME: %s\n", enf->nome);
                enfermeiros[i] = *enf;
            }

        free(enfermeiros);

        sucesso("S2) Ficheiro %s tem %ld bytes, ou seja, %d enfermeiros", FILE_ENFERMEIROS, fsize, nr_enf);

        for (int i = 0; i < nr_enf; i++)
            vagas[i].index_enfermeiro = -1;
        sucesso("S3) Iniciei a lista de %d vagas", NUM_VAGAS);
    }
    else
        erro("S2) Não consegui ler o ficheiro %s!", FILE_ENFERMEIROS);
}

Cidadao lerCidadao()
{
    Cidadao cidadao;
    FILE *cidData = fopen(FILE_PEDIDO_VACINA, "r");
    if (cidData != NULL)
    {
        char buffer[1024];
        my_fgets(buffer, sizeof(buffer), cidData);
        int scan = sscanf(buffer, "%d:%[^:]:%d:%[^:]:%[^:]:%d:%d", &cidadao.num_utente, cidadao.nome, &cidadao.idade,
                          cidadao.localidade, cidadao.nr_telemovel, &cidadao.estado_vacinacao, &cidadao.PID_cidadao);

        //TODO: Better way of checking if file can be read without strlen(buffer)?
        //Maybe access(file, R_OK)?
        if (strlen(buffer) != 0 && scan == 7)
        {
            printf("Chegou o cidadão com o pedido nº %d, com nº utente %d, para ser vacinado no Centro de Saúde %s\n",
                   cidadao.PID_cidadao, cidadao.num_utente, cidadao.localidade);
            sucesso("S5.1) Dados Cidadão: %d; %s; %d; %s; %s; %d", cidadao.num_utente, cidadao.nome, cidadao.idade,
                    cidadao.localidade, cidadao.nr_telemovel, cidadao.estado_vacinacao);
        }
        else
            erro("S5.1) Não foi possível ler o ficheiro %s", FILE_PEDIDO_VACINA);
    }
    else
        erro("S5.1) Não foi possível abrir o ficheiro %s", FILE_PEDIDO_VACINA);

    return cidadao;
}

void arranjarEnfermeiro(Cidadao cidadao)
{
    for (size_t i = 0; i < 10; i++)
    {
        printf("%s", enfermeiros[i].nome);
    }

    size_t nr_enf = sizeof(enfermeiros) / sizeof(enfermeiros[0]);
    printf("SIZE: %ld\n", nr_enf);
    fflush(stdout);
}

void handleSIGUSRone(int signal)
{
    lerCidadao();
}

int main()
{
    registarServidor();
    lerEnfermeiros();
    Cidadao cidadao = lerCidadao();
    arranjarEnfermeiro(cidadao);

    signal(SIGUSR1, handleSIGUSRone);
    sucesso("S4) Servidor espera pedidos");

    while (1)
        pause();
}