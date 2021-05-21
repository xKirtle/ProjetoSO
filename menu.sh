#!/bin/bash

###############################################################################
## ISCTE-IUL: Trabalho prático de Sistemas Operativos
##
## Aluno: Nº:98420       Nome:Rodrigo Martins
## Nome do Módulo: menu.sh
## Descrição/Explicação do Módulo:
##
##      Este script permite escolher diferentes opções num menu. O menu irá
##   fazer perguntas pertinentes para a execução da opção escolhida quando
##   necessário.
##
###############################################################################

function addNurse() {
    if [ ! -f adiciona_enfermeiros.sh ]; then
        echo "adiciona_enfermeiros.sh não existe!"
        return
    fi

    echo "------------------------"
    echo -n "Introduza o nome: "
    read -r name
    echo -n "Introduza o No. de cédula profissional: "
    read -r id
    echo -n "Introduza o centro de saúde associado: "
    read -r loc
    echo -n "Introduza a disponibilidade: "
    read -r disp

    ./adiciona_enfermeiros.sh "$name" "$id" "$loc" "$disp"
}

function showStats() {
    if [ ! -f stats.sh ]; then
        echo "stats.sh não existe!"
        return
    fi

    echo "------------------------"
    echo "1. Cidadãos"
    echo "2. Registados"
    echo "3. Enfermeiros"

    echo -n "Opção: "
    read -r opt

    case $opt in
        1) 
        echo "------------------------"
        echo -n "Qual a localidade desejada? "
        read -r loc
        ./stats.sh cidadaos "$loc";;
        2) ./stats.sh registados;;
        3) ./stats.sh enfermeiros;;
    esac
}

while :; do
    echo "------------------------"
    echo "1. Listar cidadãos"
    echo "2. Adicionar enfermeiro"
    echo "3. Stats"
    echo "4. Agendar vacinação"
    echo "0. Sair"

    echo "------------------------"
    echo -n "Opção: "
    read -r option

    case $option in
        1) 
        if [ ! -f lista_cidadaos.sh ]; then
            echo "lista_cidadaos.sh não existe!"
        else
            ./lista_cidadaos.sh
        fi
        ;;
        2) addNurse;;
        3) showStats;;
        4) 
        if [ ! -f agendamento.sh ]; then
            echo "agendamento.sh não existe!"
        else
            ./agendamento.sh
        fi
        ;;
        0) exit 1;;
        *) echo "Insira uma opção válida";;
    esac
done