#!/bin/bash

###############################################################################
## ISCTE-IUL: Trabalho prático de Sistemas Operativos
##
## Aluno: Nº:98420       Nome:Rodrigo Martins
## Nome do Módulo: adiciona_enfermeiros.sh
## Descrição/Explicação do Módulo:
##
##      Este script começa por validar todos os argumentos recebidos e acaba 
##   o programa ao encontrar um erro de formatação (deixando uma mensagem de
##   erro informativa). Se não existir nenhum erro de formatação, procura no
##   ficheiro se já existe algum enfermeiro com o centro de saúde introduzido e,
##   se não, confirma que não existe nenhum enfermeiro com o mesmo número de
##   cédula profissional. Se cumprir todas as condições, o enfermeiro é 
##   adicionado ao ficheiro enfermeriros.txt
##
###############################################################################

enfermeiros=enfermeiros.txt

if (( $# != 4)); then
    echo "Erro: Síntaxe: $0 <nome>:<número cédula profissional>:<centro saúde associado>:<disponibilidade>"
    exit 1
fi

# Compara o resultado obtido pelo regex e o argumento recebido. Se foCrem diferentes, quer dizer que o argumento está formatado incorretamente
regexResult=$( echo "$2" | grep -Eo "^[0-9]{5}" )
if [[ $regexResult != "$2" ]]; then
    echo "Erro: O Número de Cédula Profissional tem de ser composto por 5 dígitos"
    exit 1
fi

# Compara o resultado obtido pelo regex e o argumento recebido. Se foCrem diferentes, quer dizer que o argumento está formatado incorretamente
regexResult=$( echo "$3" | grep -Eo "^CS[A-Z][a-zA-Z]+" )
if [[ $regexResult != "$3" ]]; then
    echo "Erro: A localidade introduzida está no formato errado <\""CS"\"Localidade>"
    exit 1
fi

case "$4" in
0|1) : ;;
*) 
    echo "Erro: A disponibilidade introduzida está no formato errado <0/1>"
    exit 1
;;
esac

# Confirmar que $enfermeiros existe antes da próxima condição que faz uso dele
if [ ! -f $enfermeiros ]; then 
    true > ${enfermeiros} 
fi

if grep -q "$3" ${enfermeiros}; then
    echo "Erro: O Centro de Saúde introduzido já tem um enfermeiro registado"
    exit 1
else
    if grep -q "$2" ${enfermeiros}; then 
        echo "Erro: O Número de Cédula Profissional introduzido já existe noutro centro de saúde"
        exit 1
    else
        echo "$2:$1:$3:0:$4" >> ${enfermeiros}
    fi
fi

cat ${enfermeiros}