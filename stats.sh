#!/bin/bash

###############################################################################
## ISCTE-IUL: Trabalho prático de Sistemas Operativos
##
## Aluno: Nº:98420       Nome:Rodrigo Martins
## Nome do Módulo: stats.sh
## Descrição/Explicação do Módulo:
##
##   Este script realiza 3 operações:
##      1. Cidadaos - Devolve o número de ocurrências na coluna das localidades
##      2. Registados - Devolve os cidadaos acima de 60 anos por ordem decrescente
##      3. Enfermeiros - Devolve todos os enfermeiros disponíveis
##
##  Para os pontos 2 e 3, percorremos os ficheiros cidadaos.txt e enfermeiros.txt
##  linha a linha para retirar as informações necessárias (e para as comparar)
##
###############################################################################

cidadaos=cidadaos.txt
enfermeiros=enfermeiros.txt

case "$1" in
"cidadaos") 
    if (( $# == 2 )); then
        if [ ! -f $cidadaos ]; then 
            echo 0
        fi
        
        grep -c ":$2:" ${cidadaos}
    fi
 ;;
"registados") 
    while read -r line ; do
    numero=$( echo "$line" | cut -d":" -f1 )
    nome=$( echo "$line" | cut -d":" -f2 )
    idade=$( echo "$line" | cut -d":" -f3 )
    resultado="$numero:$nome"

    if [[ $idade -gt 60 ]]; then
        echo "$resultado"
    fi

    done < <(sort -r -k3,3 -k2,2 -t":" ${cidadaos})
;;
"enfermeiros") 
    while read -r line ; do
    nome=$( echo "$line" | cut -d":" -f2 )
    disponibilidade=$( echo "$line" | cut -d":" -f5 )

    if [[ $disponibilidade == 1 ]]; then
        echo "$nome"
    fi

    done < ${enfermeiros}
;;
esac