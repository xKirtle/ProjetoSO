/******************************************************************************
 ** ISCTE-IUL: Trabalho prático de Sistemas Operativos
 **
 ** Aluno: Nº:       Nome: 
 ** Nome do Módulo: servidor.c
 ** Descrição/Explicação do Módulo: 
 **
 **
 ******************************************************************************/
#include "common.h"

Vaga vagas[NUM_VAGAS];
Enfermeiro enfermeiros;

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

int main() 
{
    registarServidor();
}