# novoCompilador
bkp porque deu ruim com o outro projeto

COMPILADOR PORTUGOL (FASE 1: ANÁLISE LÉXICA E SINTÁTICA)


1. IDENTIFICAÇÃO

Integrante 1: Bruno Germanetti Ramalho - RA: 10426491

2. COMPILAR

O código foi desenvolvido em C padrão (C99/C11) e preparado para o GCC no 
Ubuntu 22.04 LTS (ou via WSL).

Comando de compilação:
  gcc -Wall -Wno-unused-result -g -Og compilador.c -o compilador

O programa compila de forma estrita, sem produzir qualquer warning.

3. COMO EXECUTAR

Para executar a análise de um código-fonte Portugol:
  ./compilador <nome_do_arquivo_fonte>

Exemplo:
  ./compilador ex1.pas

4. VERIFICAÇÃO DE VAZAMENTO DE MEMÓRIA

Para verificar a integridade da memória e ausência de vazamentos:
  valgrind --leak-check=yes ./compilador <nome_do_arquivo_fonte>

Resultado esperado:
  All heap blocks were freed -- no leaks are possible (0 errors).

5. ESCOPO CONCLUÍDO

A Fase 1 foi 100% implementada e testada com sucesso:
- Analisador Léxico (obter_atomo):
  * Reconhecimento de todas as palavras reservadas (insensível a maiúsculas e minúsculas).
  * Reconhecimento de identificadores (até 15 caracteres; rejeita maiores com erro léxico).
  * Reconhecimento de comentários de múltiplas linhas ({- ... -}) preservando número de linhas.
  * Reconhecimento de constantes inteiras decimais e notação científica (ex: 42E2, 42E+2).
  * Reconhecimento de constantes caractere ('a').
  * Reconhecimento de todos os operadores relacionais, aritméticos e pontuação.
- Analisador Sintático Preditivo Descendente Recursivo (consome):
  * Todas as produções da gramática EBNF implementadas rigorosamente.
  * Fatoração à esquerda para diferenciar atribuição e chamada de procedimento.
  * Saída formatada linha a linha correspondente aos exemplos do enunciado.
  * Tratamento e emissão de mensagens de erro léxico e sintático com parada imediata e código 0.

6. DECISÕES DE DESIGN E IMPLEMENTAÇÃO

- O arquivo fonte é carregado integralmente em um buffer dinâmico de memória (buffer_fonte),
  conforme arquitetura da Figura 1 da especificação.
- Conforme regra, os comentários lidos pelo analisador léxico
  são devolvidos como átomos ao analisador sintático (ATOMO_COMENTARIO), sendo impressos
  e descartados pelo analisador sintático na função consome() e no fluxo principal.
- Foi implementada uma rotina centralizada de finalização (finalizar_execucao) que garante
  a desalocação completa do buffer_fonte tanto em saídas de sucesso quanto em saídas
  de erro léxico/sintático, garantindo ausência total de leaks no Valgrind.
