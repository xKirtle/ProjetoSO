#!/bin/bash

###############################################################################
## ISCTE-IUL: Trabalho prático de Sistemas Operativos
##
## Aluno: Nº:98420       Nome:Rodrigo Martins
## Nome do Módulo: lista_cidadaos.sh
## Descrição/Explicação do Módulo:
##
##      Este script lê o ficheiro listagem.txt linha a linha, reorganiza e
##   adiciona informações em cada cidadão (linha) e adiciona no fim do ficheiro
##   cidadaos.txt
##
###############################################################################

listagem=listagem.txt
numero=10000

true > cidadaos.txt

while read -r line ; do
    line=$( echo "$line" | sed "s/ | /|/g")

    numero=$(($numero+1))
    nome=$( echo "$line" | cut -d"|" -f1 | cut -d":" -f2 )
    idade=$(( $( date +"%Y" ) - $( echo "$line" | cut -d"|" -f2 | cut -d":" -f2 | cut -d"-" -f3 ) ))
    localidade=$( echo "$line" | cut -d"|" -f3 | cut -d":" -f2 )
    telefone=$( echo "$line" | cut -d"|" -f4 | cut -d":" -f2 )
    vacinado=0
    
    resultado="$numero:$nome:$idade:$localidade:$telefone:$vacinado"
    echo "$resultado"
done < ${listagem} >> cidadaos.txt

cat cidadaos.txt
