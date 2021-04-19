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
Enfermeiro* enfermeiros;

void registarServidor()
{
    FILE *servidor = fopen(FILE_PID_SERVIDOR, "w");
    if (servidor != NULL)
    {
        fprintf(servidor, "%d", getpid());
        sucesso("S1) Escrevi no ficheiro %s o PID: %d", FILE_PID_SERVIDOR, getpid());
    }
    else erro("S1) Não consegui registar o servidor!");
    fclose(servidor);
}

void lerEnfermeiros()
{
    Enfermeiro enf[sizeof(Enfermeiro)];
    FILE *enfData = fopen(FILE_ENFERMEIROS, "rb");
    if (enfData != NULL)
    {
        fseek(enfData, 0, SEEK_END );
        long fsize = ftell(enfData);
        int nr_enf = fsize / sizeof(Enfermeiro);
        enfermeiros = (Enfermeiro *)malloc(nr_enf * sizeof(Enfermeiro));

        for (int i = 0; i < nr_enf; i++)
            if (fread(enf, sizeof(Enfermeiro), 1, enfData) == 1)
                enfermeiros[i] = *enf;

        free(enfermeiros);

        sucesso("S2) Ficheiro %s tem %ld bytes, ou seja, %d enfermeiros", FILE_ENFERMEIROS, fsize, nr_enf);

        for (int i = 0; i < nr_enf; i++)
            vagas[i].index_enfermeiro = -1;
        sucesso("S3) Iniciei a lista de %d vagas", NUM_VAGAS);
    }
    else erro("S2) Não consegui ler o ficheiro %s!", FILE_ENFERMEIROS);
}

void handleSIGUSRone(int signal)
{
    //S5
}

int main() 
{
    registarServidor();
    lerEnfermeiros();

    signal(SIGUSR1, handleSIGUSRone);
    sucesso("S4) Servidor espera pedidos");
}