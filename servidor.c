/******************************************************************************
 ** ISCTE-IUL: Trabalho prático de Sistemas Operativos
 **
 ** Aluno: Nº: 98420   Nome: Rodrigo Martins 
 ** Nome do Módulo: servidor.c
 ** Descrição/Explicação do Módulo: 
 **
 **   O módulo começa por se "registar", ou seja, guardar num ficheiro o seu
 ** process ID e ler todos os enfermeiros presentes no ficheiro dat. Depois,
 ** arma o sinal necessário para receber pedidos de vacinação de cidadãos e o
 ** sinal para terminar o servidor.
 **
 **   Quando receber um pedido de vacinação, o servidor vai ler a informação
 ** do cidadão presente no ficheiro pedidovacina.txt e vai tentar arranjar
 ** um enfermeiros no mesmo Centro de Saúde que esteja disponível para vacinar.
 ** 
 **   Se conseguir arranjar um enfermeiro que cumpra os requerimentos, o servidor
 ** vai criar um novo processo para tratar dessa vacinação, de modo a poder
 ** continuar à espera de novos pedidos de vacinação de outros utentes.
 ******************************************************************************/
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>
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
        sucesso("S1) Escrevi no ficheiro FILE_PID_SERVIDOR o PID: %d", getpid());
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

        sucesso("S2) Ficheiro FILE_ENFERMEIROS tem %ld bytes, ou seja, %d enfermeiros", fsize, nr_enf);

        for (int i = 0; i < NUM_VAGAS; i++)
            vagas[i].index_enfermeiro = -1;
        sucesso("S3) Iniciei a lista de %d vagas", NUM_VAGAS);
    }
    else
        erro("S2) Não consegui ler o ficheiro FILE_ENFERMEIROS!");
}

Cidadao lerCidadao()
{
    Cidadao cidadao;

    if (access(FILE_PEDIDO_VACINA, F_OK) != 0)
        erro("S5.1) Não foi possível abrir o ficheiro FILE_PEDIDO_VACINA");

    FILE *cidData = fopen(FILE_PEDIDO_VACINA, "r");
    if (cidData != NULL)
    {
        char buffer[1024];
        my_fgets(buffer, sizeof(buffer), cidData);
        int scan = sscanf(buffer, "%d:%[^:]:%d:%[^:]:%[^:]:%d:%d", &cidadao.num_utente, cidadao.nome, &cidadao.idade,
                          cidadao.localidade, cidadao.nr_telemovel, &cidadao.estado_vacinacao, &cidadao.PID_cidadao);

        printf("Chegou o cidadão com o pedido nº %d, com nº utente %d, para ser vacinado no Centro de Saúde %s\n",
               cidadao.PID_cidadao, cidadao.num_utente, cidadao.localidade);
        sucesso("S5.1) Dados Cidadão: %d; %s; %d; %s; %s; %d", cidadao.num_utente, cidadao.nome, cidadao.idade,
                cidadao.localidade, cidadao.nr_telemovel, cidadao.estado_vacinacao);
    }
    else
        erro("S5.1) Não foi possível abrir o ficheiro FILE_PEDIDO_VACINA");

    return cidadao;
}

int arranjarEnfermeiro(Cidadao cidadao)
{
    for (int i = 0; i < nr_enf; i++)
    {
        if (strcmp(enfermeiros[i].CS_enfermeiro, cidadao.localidade) == 0)
        {
            if (enfermeiros[i].disponibilidade == 1)
            {
                sucesso("S5.2.1) Enfermeiro %d disponível para o pedido %d", i, cidadao.PID_cidadao);

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
                    return -1;
                }
            }
            else
            {
                kill(cidadao.PID_cidadao, SIGTERM);
                erro("S5.2.1) Enfermeiro %d indisponível para o pedido %d para o Centro de Saúde %s", i, cidadao.PID_cidadao, enfermeiros[i].CS_enfermeiro);
                return -1;
            }
        }
    }

    return 0;
}

void handleSIGCHLD(int signal)
{
    //hexdump -e '"%i, %20.100s, %12.100s, %i, %i\n"' enfermeiros.dat

    //Servidor-Filho terminou e chama isto?
    //S5.5.2??
    
    int pid = wait(NULL);

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

    FILE *enfData = fopen(FILE_ENFERMEIROS, "wb");
    if (enfData != NULL)
    {
        for (int i = 0; i < nr_enf; i++)
            fwrite(&enfermeiros[i], sizeof(Enfermeiro), 1, enfData);

        sucesso("S5.5.3.4) Ficheiro FILE_ENFERMEIROS %d atualizado para %d vacinas dadas", enfIndex, enfermeiros[enfIndex].num_vac_dadas);
    }
    fclose(enfData);

    kill(vagas[index_vagas].cidadao.PID_cidadao, SIGUSR2);
    sucesso("S5.5.3.5) Retorna");
}

void handleSIGTERM(int signal) //Chamado pelo filho
{
    kill(vagas[index_vagas].cidadao.PID_cidadao, SIGTERM);
    kill(vagas[index_vagas].PID_filho, SIGTERM); //S5.6.1?
    sucesso("S5.6.1) SIGTERM recebido, servidor dedicado termina Cidadão");
    exit(0);
}

void handleSIGINT(int signal) //Chamado pelo pai
{
    for (int i = 0; i < NUM_VAGAS; i++)
        kill(vagas[i].PID_filho, SIGTERM);

    remove(FILE_PID_SERVIDOR);
    sucesso("S6) Servidor terminado");
    exit(0);
}

void vacinarPedido(Cidadao cidadao)
{
    int value = fork(); // =0 -> child, >0 -> parent (value->childPID), =-1 error
    if (value == -1)
        erro("S5.4) Não foi possível criar o servidor dedicado");
    else if (value != 0) //Parent
    {
        sucesso("S5.4) Servidor dedicado %d criado para o pedido %d", value, cidadao.PID_cidadao);
        vagas[index_vagas].PID_filho = value;
        sucesso("S5.5.1) Servidor dedicado %d na vaga %d", value, index_vagas);
        signal(SIGCHLD, handleSIGCHLD);
        sucesso("S5.5.2) Servidor aguarda fim do servidor dedicado %d", value);
    }
    else //value == 0 -> Child
    {
        signal(SIGTERM, handleSIGTERM);
        kill(cidadao.PID_cidadao, SIGUSR1);
        sucesso("S5.6.2) Servidor dedicado inicia consulta de vacinação");
        sleep(TEMPO_CONSULTA);
        sucesso("S5.6.3) Vacinação terminada para o cidadão com o pedido nº %d", cidadao.PID_cidadao);
        kill(cidadao.PID_cidadao, SIGUSR2);
        sucesso("S5.6.4) Servidor dedicado termina consulta de vacinação");
        exit(0); //Chama o SIGCHLD automaticamente no pai
    }
}

void handleSIGUSRone(int signal)
{
    Cidadao cidadao = lerCidadao();
    int available = arranjarEnfermeiro(cidadao);

    if (available == 0)
        vacinarPedido(cidadao);
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