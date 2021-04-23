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
int index_vagas;
Enfermeiro *enfermeiros;
int nr_enf; //Numero de enfermeiros no sistema

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
    FILE *enfData = fopen(FILE_ENFERMEIROS, "rb");
    if (enfData != NULL)
    {
        fseek(enfData, 0, SEEK_END);
        long fsize = ftell(enfData);
        nr_enf = fsize / sizeof(Enfermeiro);
        //Confirmar que malloc nao retorna um null pointer?
        enfermeiros = (Enfermeiro *)malloc(nr_enf * sizeof(Enfermeiro));
        fseek(enfData, 0, SEEK_SET);

        Enfermeiro enf[sizeof(Enfermeiro)];
        for (int i = 0; i < nr_enf; i++)
            if (fread(enf, sizeof(Enfermeiro), 1, enfData) == 1)
                enfermeiros[i] = *enf;

        // Nao queremos libertar a memoria onde temos os enfermeiros..?
        // free(enfermeiros);

        sucesso("S2) Ficheiro %s tem %ld bytes, ou seja, %d enfermeiros", FILE_ENFERMEIROS, fsize, nr_enf);

        for (int i = 0; i < NUM_VAGAS; i++)
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
    for (int i = 0; i < nr_enf; i++)
    {
        if (strcmp(enfermeiros[i].CS_enfermeiro, cidadao.localidade) == 0)
        {
            if (enfermeiros[i].disponibilidade == 1)
            {
                sucesso("S5.2.1) Enfermeiro <Index %d disponível para o pedido %d", i, cidadao.PID_cidadao);

                for (index_vagas = 0; index_vagas < NUM_VAGAS; index_vagas++)
                    if (vagas[index_vagas].index_enfermeiro == -1)
                        break;

                if (index_vagas != NUM_VAGAS) //existem vagas
                {
                    sucesso("S5.2.2) Há vaga para vacinação para o pedido %d", cidadao.PID_cidadao);

                    enfermeiros[i].disponibilidade = 0;
                    Vaga vaga = {cidadao.PID_cidadao, cidadao, i};
                    vagas[index_vagas] = vaga;

                    sucesso("S5.3) Vaga nº %d preenchida para o pedido %d", index_vagas, cidadao.PID_cidadao);
                }
                else
                {
                    kill(cidadao.PID_cidadao, SIGTERM);
                    erro("S5.2.2) Não há vaga para vacinação para o pedido %d", cidadao.PID_cidadao);
                }
            }
            else
            {
                kill(cidadao.PID_cidadao, SIGTERM);
                erro("S5.2.1) Enfermeiro %d indisponível para o pedido %d para o Centro de Saúde %s", i, cidadao.PID_cidadao, enfermeiros[i].CS_enfermeiro);
            }
        }
    }
}

void handleSIGCHLD(int signal)
{
    //hexdump -e '"%i, %20.100s, %12.100s, %i, %i\n"' enfermeiros.dat

    //Servidor-Filho terminou e chama isto?
    //S5.5.2??

    //Vaga libertada?
    int enfIndex = vagas[index_vagas].index_enfermeiro;
    sucesso("S5.5.3.1) Vaga %d que era do servidor dedicado %d libertada", index_vagas, vagas[index_vagas].PID_filho);
    vagas[index_vagas].index_enfermeiro = -1;

    enfermeiros[enfIndex].disponibilidade = 1;
    sucesso("S5.5.3.2) Enfermeiro %d atualizado para disponível", enfIndex);

    enfermeiros[enfIndex].num_vac_dadas += 1;
    sucesso("S5.5.3.3) Enfermeiro %d atualizado para %d vacinas dadas", enfIndex, enfermeiros[enfIndex].num_vac_dadas);

    //ATUALIZAR ENFERMEIROS.DAT -> S5.5.3.4
    //Modificar apenas o nr de vacinas em vez do ficheiro todo?

    FILE *enfData = fopen(FILE_ENFERMEIROS, "wb+");
    if (enfData != NULL)
    {
        for (int i = 0; i < nr_enf; i++)
            fwrite(&enfermeiros[i], sizeof(Enfermeiro), 1, enfData);

        sucesso("S5.5.3.4) Ficheiro %s %d atualizado para %d vacinas dadas", FILE_ENFERMEIROS, enfIndex, enfermeiros[enfIndex].num_vac_dadas);
    }
    fclose(enfData);

    sucesso("S5.5.3.5) Retorna");
}

void handleSIGTERM(int signal) //Chamado pelo filho
{
    kill(SIGTERM, vagas[index_vagas].cidadao.PID_cidadao);
    sucesso("S5.6.1) SIGTERM recebido, servidor dedicado termina Cidadão");
    exit(0);
}

void handleSIGINT(int signal) //Chamado pelo pai
{
    for (int i = 0; i < NUM_VAGAS; i++)
        kill(SIGTERM, vagas[i].PID_filho);

    remove(FILE_PID_SERVIDOR);
    sucesso("S6) Servidor terminado");
    exit(0);
}

void criarFilho(Cidadao cidadao)
{
    int value = fork(); // =0 -> child, >0 -> parent (value->childPID), =-1 error
    if (value == -1)
        erro("S5.4) Não foi possível criar o servidor dedicado");
    else if (value != 0) //Parent
    {
        sucesso("S5.4) Servidor dedicado %d criado para o pedido %d", value, cidadao.PID_cidadao);
        vagas[index_vagas].PID_filho = value;
        sucesso("S5.5.1) Servidor dedicado %d na vaga %d", value, index_vagas);
        signal(SIGCHLD, handleSIGCHLD); //TODO: S5.5.2?????????????????
        sucesso("S5.5.2) Servidor aguarda fim do servidor dedicado %d", value);
    }
    else //value == 0 -> Child
    {
        signal(SIGTERM, handleSIGTERM);
        kill(SIGUSR1, cidadao.PID_cidadao);
        sucesso("S5.6.2) Servidor dedicado inicia consulta de vacinação");
        sleep(TEMPO_CONSULTA);
        sucesso("S5.6.3) Vacinação terminada para o cidadão com o pedido nº %d", cidadao.PID_cidadao);
        kill(SIGUSR2, cidadao.PID_cidadao);
        sucesso("S5.6.4) Servidor dedicado termina consulta de vacinação");
        exit(0);
    }
}

void handleSIGUSRone(int signal)
{
    Cidadao cidadao = lerCidadao();
    arranjarEnfermeiro(cidadao);
    criarFilho(cidadao);
}

int main()
{
    registarServidor();
    lerEnfermeiros();

    signal(SIGUSR1, handleSIGUSRone);
    signal(SIGINT, handleSIGINT);
    sucesso("S4) Servidor espera pedidos");

    while (1)
        pause();
}