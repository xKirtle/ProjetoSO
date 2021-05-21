#!/bin/bash

###############################################################################
## ISCTE-IUL: Trabalho prático de Sistemas Operativos
##
## Aluno: Nº:98420       Nome:Rodrigo Martins
## Nome do Módulo: agendamento.sh
## Descrição/Explicação do Módulo:
##
##      Este script lê o ficheiro enfermeiros.txt linha a linha. Se o enfermeiro 
##   atual estiver disponível, lemos linha a linha o ficheiro dos cidadaos e
##   encontramos cidadaos da mesma localidade para adicionar ao ficheiro agenda.txt
##
###############################################################################

cidadaos=cidadaos.txt
enfermeiros=enfermeiros.txt
agenda=agenda.txt

if [ ! -f $enfermeiros ] || [ ! -f $cidadaos ]; then 
    exit 1
fi

true > ${agenda}

while read -r line ; do
    enfID=$( echo "$line" | cut -d":" -f1 )
    enfNome=$( echo "$line" | cut -d":" -f2 )
    enfLocalidade=$( echo "$line" | cut -d":" -f3 )
    enfLoc=$( echo $enfLocalidade | sed "s/CS//g" )
    disponibilidade=$( echo "$line" | cut -d":" -f5 )

    if [[ $disponibilidade == 0 ]]; then 
        continue 
    fi

    while read -r line2 ; do
        cidID=$( echo "$line2" | cut -d":" -f1 )
        cidNome=$( echo "$line2" | cut -d":" -f2 )
        cidLocalidade=$( echo "$line2" | cut -d":" -f4)

        if [[ $enfLoc == "$cidLocalidade" ]]; then
            echo "$enfNome:$enfID:$cidNome:$cidID:$enfLocalidade:$(date +"%Y-%m-%d")" >> ${agenda}
        fi

    done < ${cidadaos}

done < ${enfermeiros}