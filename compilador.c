/*
 * ============================================================================
 * TRABALHO 1 - COMPILADORES: FASE 1 (ANALISADOR LÉXICO E SINTÁTICO)
 * Linguagem: Portugol
 *
 * Integrantes do Grupo:
 * Nome 1: Bruno Germanetti Ramalho - RA: 10426491
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

/* --- Enumeração dos Átomos --- */
typedef enum {
    ATOMO_ERRO = 0,
    ATOMO_FIM_ARQUIVO,
    ATOMO_COMENTARIO,
    ATOMO_IDENTIFICADOR,
    ATOMO_CONSTINT,
    ATOMO_CONSTCHAR,

    /* Palavras Reservadas */
    ATOMO_ALGORITMO,
    ATOMO_CARACTERE,
    ATOMO_DIV,
    ATOMO_E,
    ATOMO_ENQUANTO,
    ATOMO_ENTAO,
    ATOMO_ESCREVA,
    ATOMO_FACA,
    ATOMO_FALSO,
    ATOMO_FIM,
    ATOMO_FUNCAO,
    ATOMO_INICIO,
    ATOMO_INTEIRO,
    ATOMO_LEIA,
    ATOMO_LOGICO,
    ATOMO_MOD,
    ATOMO_NAO,
    ATOMO_OU,
    ATOMO_PROCEDIMENTO,
    ATOMO_SE,
    ATOMO_SENAO,
    ATOMO_VAR,
    ATOMO_VERDADEIRO,

    /* Símbolos e Operadores */
    ATOMO_PONTO,            /* . */
    ATOMO_PONTO_VIRGULA,    /* ; */
    ATOMO_VIRGULA,          /* , */
    ATOMO_DOIS_PONTOS,      /* : */
    ATOMO_ATRIBUICAO,       /* := */
    ATOMO_ABRE_PAR,         /* ( */
    ATOMO_FECHA_PAR,        /* ) */
    ATOMO_MAIS,             /* + */
    ATOMO_MENOS,            /* - */
    ATOMO_VEZES,            /* * */
    ATOMO_BARRA,            /* / */
    ATOMO_IGUAL,            /* = */
    ATOMO_DIFERENTE,        /* <> */
    ATOMO_MENOR,            /* < */
    ATOMO_MENOR_IGUAL,      /* <= */
    ATOMO_MAIOR,            /* > */
    ATOMO_MAIOR_IGUAL       /* >= */
} TAtomo;

/* --- Estrutura TInfoAtomo --- */
typedef struct {
    TAtomo atomo;
    int linha;
    union {
        int numero;      /* Atributo do átomo constint */
        char id[16];     /* Atributo identificador (máx 15 chars + \0) */
        char ch;         /* Atributo do átomo constchar */
    } atributo;
} TInfoAtomo;

/* --- Variáveis Globais de Controle --- */
static char *buffer_fonte = NULL;
static int pos = 0;
static int linha_atual = 1;
static int ultima_linha_analisada = 1;
static TInfoAtomo lookahead;

/* --- Protótipos Obrigatórios --- */
TInfoAtomo obter_atomo(void);
void consome(TAtomo atomo);

/* --- Protótipos das Regras da Gramática --- */
void programa(void);
void bloco(void);
void declaracao_variaveis(void);
void lista_variaveis(void);
void tipo(void);
void declaracao_de_rotinas(void);
void declaracao_de_funcao(void);
void declaracao_de_procedimento(void);
void parametros_formais(void);
void parametro_formal(void);
void comando_composto(void);
void comando(void);
void comando_atribuicao(void);
void comando_entrada(void);
void comando_saida(void);
void comando_condicional(void);
void comando_repeticao(void);
void lista_expressao(void);
void expressao(void);
void expressao_simples(void);
void termo(void);
void fator(void);

/* --- Funções Auxiliares de Limpeza e Erro --- */
static void liberar_recursos(void) {
    if (buffer_fonte != NULL) {
        free(buffer_fonte);
        buffer_fonte = NULL;
    }
}

static void finalizar_execucao(int codigo) {
    liberar_recursos();
    exit(codigo);
}

/* Representação textual do átomo em mensagens de erro sintático */
static const char* atomo_para_simbolo(TAtomo atomo) {
    switch (atomo) {
        case ATOMO_PONTO: return ".";
        case ATOMO_PONTO_VIRGULA: return ";";
        case ATOMO_VIRGULA: return ",";
        case ATOMO_DOIS_PONTOS: return ":";
        case ATOMO_ATRIBUICAO: return ":=";
        case ATOMO_ABRE_PAR: return "(";
        case ATOMO_FECHA_PAR: return ")";
        case ATOMO_MAIS: return "+";
        case ATOMO_MENOS: return "-";
        case ATOMO_VEZES: return "*";
        case ATOMO_BARRA: return "/";
        case ATOMO_IGUAL: return "=";
        case ATOMO_DIFERENTE: return "<>";
        case ATOMO_MENOR: return "<";
        case ATOMO_MENOR_IGUAL: return "<=";
        case ATOMO_MAIOR: return ">";
        case ATOMO_MAIOR_IGUAL: return ">=";
        case ATOMO_ALGORITMO: return "algoritmo";
        case ATOMO_VAR: return "var";
        case ATOMO_INICIO: return "inicio";
        case ATOMO_FIM: return "fim";
        case ATOMO_SE: return "se";
        case ATOMO_ENTAO: return "entao";
        case ATOMO_SENAO: return "senao";
        case ATOMO_ENQUANTO: return "enquanto";
        case ATOMO_FACA: return "faca";
        case ATOMO_LEIA: return "leia";
        case ATOMO_ESCREVA: return "escreva";
        case ATOMO_FUNCAO: return "funcao";
        case ATOMO_PROCEDIMENTO: return "procedimento";
        case ATOMO_INTEIRO: return "inteiro";
        case ATOMO_CARACTERE: return "caractere";
        case ATOMO_LOGICO: return "logico";
        case ATOMO_VERDADEIRO: return "verdadeiro";
        case ATOMO_FALSO: return "falso";
        case ATOMO_DIV: return "div";
        case ATOMO_MOD: return "mod";
        case ATOMO_E: return "e";
        case ATOMO_OU: return "ou";
        case ATOMO_NAO: return "nao";
        case ATOMO_IDENTIFICADOR: return "identificador";
        case ATOMO_CONSTINT: return "constint";
        case ATOMO_CONSTCHAR: return "constchar";
        case ATOMO_FIM_ARQUIVO: return "fim de arquivo";
        default: return "desconhecido";
    }
}

/* Impressão dos átomos conforme padrão exigido nas especificações */
static void imprimir_atomo(TInfoAtomo info) {
    switch (info.atomo) {
        case ATOMO_COMENTARIO:
            printf("# %d: comentario\n", info.linha);
            break;
        case ATOMO_IDENTIFICADOR:
            printf("# %d: identificador: %s\n", info.linha, info.atributo.id);
            break;
        case ATOMO_CONSTINT:
            printf("# %d: constint: %d\n", info.linha, info.atributo.numero);
            break;
        case ATOMO_CONSTCHAR:
            printf("# %d: constchar: %c\n", info.linha, info.atributo.ch);
            break;
        case ATOMO_ALGORITMO:
            printf("# %d: algoritmo\n", info.linha);
            break;
        case ATOMO_CARACTERE:
            printf("# %d: caractere\n", info.linha);
            break;
        case ATOMO_DIV:
            printf("# %d: div\n", info.linha);
            break;
        case ATOMO_E:
            printf("# %d: e\n", info.linha);
            break;
        case ATOMO_ENQUANTO:
            printf("# %d: enquanto\n", info.linha);
            break;
        case ATOMO_ENTAO:
            printf("# %d: entao\n", info.linha);
            break;
        case ATOMO_ESCREVA:
            printf("# %d: escreva\n", info.linha);
            break;
        case ATOMO_FACA:
            printf("# %d: faca\n", info.linha);
            break;
        case ATOMO_FALSO:
            printf("# %d: falso\n", info.linha);
            break;
        case ATOMO_FIM:
            printf("# %d: fim\n", info.linha);
            break;
        case ATOMO_FUNCAO:
            printf("# %d: funcao\n", info.linha);
            break;
        case ATOMO_INICIO:
            printf("# %d: inicio\n", info.linha);
            break;
        case ATOMO_INTEIRO:
            printf("# %d: inteiro\n", info.linha);
            break;
        case ATOMO_LEIA:
            printf("# %d: leia\n", info.linha);
            break;
        case ATOMO_LOGICO:
            printf("# %d: logico\n", info.linha);
            break;
        case ATOMO_MOD:
            printf("# %d: mod\n", info.linha);
            break;
        case ATOMO_NAO:
            printf("# %d: nao\n", info.linha);
            break;
        case ATOMO_OU:
            printf("# %d: ou\n", info.linha);
            break;
        case ATOMO_PROCEDIMENTO:
            printf("# %d: procedimento\n", info.linha);
            break;
        case ATOMO_SE:
            printf("# %d: se\n", info.linha);
            break;
        case ATOMO_SENAO:
            printf("# %d: senao\n", info.linha);
            break;
        case ATOMO_VAR:
            printf("# %d: var\n", info.linha);
            break;
        case ATOMO_VERDADEIRO:
            printf("# %d: verdadeiro\n", info.linha);
            break;
        case ATOMO_PONTO:
            printf("# %d: ponto\n", info.linha);
            break;
        case ATOMO_PONTO_VIRGULA:
            printf("# %d: ponto_virgula\n", info.linha);
            break;
        case ATOMO_VIRGULA:
            printf("# %d: virgula\n", info.linha);
            break;
        case ATOMO_DOIS_PONTOS:
            printf("# %d: dois_pontos\n", info.linha);
            break;
        case ATOMO_ATRIBUICAO:
            printf("# %d: atribuicao\n", info.linha);
            break;
        case ATOMO_ABRE_PAR:
            printf("# %d: abre_par\n", info.linha);
            break;
        case ATOMO_FECHA_PAR:
            printf("# %d: fecha_par\n", info.linha);
            break;
        case ATOMO_MAIS:
            printf("# %d: mais\n", info.linha);
            break;
        case ATOMO_MENOS:
            printf("# %d: menos\n", info.linha);
            break;
        case ATOMO_VEZES:
            printf("# %d: vezes\n", info.linha);
            break;
        case ATOMO_BARRA:
            printf("# %d: barra\n", info.linha);
            break;
        case ATOMO_IGUAL:
            printf("# %d: igual\n", info.linha);
            break;
        case ATOMO_DIFERENTE:
            printf("# %d: diferente\n", info.linha);
            break;
        case ATOMO_MENOR:
            printf("# %d: menor\n", info.linha);
            break;
        case ATOMO_MENOR_IGUAL:
            printf("# %d: menor_igual\n", info.linha);
            break;
        case ATOMO_MAIOR:
            printf("# %d: maior\n", info.linha);
            break;
        case ATOMO_MAIOR_IGUAL:
            printf("# %d: maior_igual\n", info.linha);
            break;
        default:
            break;
    }
}

/* ============================================================================
 * ANALISADOR LÉXICO: obter_atomo()
 * ============================================================================ */
TInfoAtomo obter_atomo(void) {
    TInfoAtomo info;
    info.atomo = ATOMO_ERRO;
    info.linha = linha_atual;

    while (buffer_fonte[pos] != '\0') {
        /* Ignora caracteres delimitadores mantendo a contagem de linhas */
        if (buffer_fonte[pos] == ' ' || buffer_fonte[pos] == '\t' || buffer_fonte[pos] == '\r') {
            pos++;
            continue;
        }
        if (buffer_fonte[pos] == '\n') {
            linha_atual++;
            pos++;
            continue;
        }

        /* Comentários de múltiplas linhas: {- ... -} */
        if (buffer_fonte[pos] == '{') {
            if (buffer_fonte[pos + 1] == '-') {
                int linha_inicio_comentario = linha_atual;
                pos += 2;
                while (buffer_fonte[pos] != '\0') {
                    if (buffer_fonte[pos] == '-' && buffer_fonte[pos + 1] == '}') {
                        pos += 2;
                        info.atomo = ATOMO_COMENTARIO;
                        info.linha = linha_inicio_comentario;
                        ultima_linha_analisada = linha_atual;
                        return info;
                    }
                    if (buffer_fonte[pos] == '\n') {
                        linha_atual++;
                    }
                    pos++;
                }
                /* Erro léxico: comentário não fechado */
                printf("# %d: erro lexico, comentario nao finalizado\n", linha_inicio_comentario);
                finalizar_execucao(0);
            } else {
                printf("# %d: erro lexico, caractere invalido [%c]\n", linha_atual, buffer_fonte[pos]);
                finalizar_execucao(0);
            }
        }

        /* Constante caractere: constchar ('a') */
        if (buffer_fonte[pos] == '\'') {
            int linha_char = linha_atual;
            pos++;
            if (buffer_fonte[pos] == '\0' || buffer_fonte[pos] == '\n') {
                printf("# %d: erro lexico, constante caractere mal formada\n", linha_char);
                finalizar_execucao(0);
            }
            char c = buffer_fonte[pos++];
            if (buffer_fonte[pos] != '\'') {
                printf("# %d: erro lexico, constante caractere mal formada\n", linha_char);
                finalizar_execucao(0);
            }
            pos++; /* Pula aspas de fechamento */
            info.atomo = ATOMO_CONSTCHAR;
            info.linha = linha_char;
            info.atributo.ch = c;
            ultima_linha_analisada = linha_atual;
            return info;
        }

        /* Constante inteira: constint */
        if (isdigit((unsigned char)buffer_fonte[pos])) {
            int linha_num = linha_atual;
            long long valor = 0;
            while (isdigit((unsigned char)buffer_fonte[pos])) {
                valor = valor * 10 + (buffer_fonte[pos] - '0');
                pos++;
            }
            if (buffer_fonte[pos] == 'E' || buffer_fonte[pos] == 'e') {
                int saved_pos = pos;
                pos++;
                int sinal = 1;
                if (buffer_fonte[pos] == '+') {
                    pos++;
                } else if (buffer_fonte[pos] == '-') {
                    sinal = -1;
                    pos++;
                }
                if (isdigit((unsigned char)buffer_fonte[pos])) {
                    int exp = 0;
                    while (isdigit((unsigned char)buffer_fonte[pos])) {
                        exp = exp * 10 + (buffer_fonte[pos] - '0');
                        pos++;
                    }
                    if (sinal > 0) {
                        for (int k = 0; k < exp; k++) valor *= 10;
                    } else {
                        for (int k = 0; k < exp; k++) valor /= 10;
                    }
                } else {
                    /* Notação exponencial incompleta */
                    pos = saved_pos;
                }
            }
            info.atomo = ATOMO_CONSTINT;
            info.linha = linha_num;
            info.atributo.numero = (int)valor;
            ultima_linha_analisada = linha_atual;
            return info;
        }

        /* Identificadores e Palavras Reservadas */
        if (isalpha((unsigned char)buffer_fonte[pos])) {
            int linha_id = linha_atual;
            char lexema[128];
            int len = 0;

            while (isalnum((unsigned char)buffer_fonte[pos]) || buffer_fonte[pos] == '_') {
                if (len < 127) {
                    lexema[len++] = buffer_fonte[pos];
                }
                pos++;
            }
            lexema[len] = '\0';

            /* Validação do limite de 15 caracteres */
            if (len > 15) {
                printf("# %d: erro lexico, identificador excede 15 caracteres [%s]\n", linha_id, lexema);
                finalizar_execucao(0);
            }

            /* Comparação insensível a maiúsculas e minúsculas */
            char lower[128];
            for (int i = 0; i <= len; i++) {
                lower[i] = (char)tolower((unsigned char)lexema[i]);
            }

            if (strcmp(lower, "algoritmo") == 0) info.atomo = ATOMO_ALGORITMO;
            else if (strcmp(lower, "caractere") == 0) info.atomo = ATOMO_CARACTERE;
            else if (strcmp(lower, "div") == 0) info.atomo = ATOMO_DIV;
            else if (strcmp(lower, "e") == 0) info.atomo = ATOMO_E;
            else if (strcmp(lower, "enquanto") == 0) info.atomo = ATOMO_ENQUANTO;
            else if (strcmp(lower, "entao") == 0) info.atomo = ATOMO_ENTAO;
            else if (strcmp(lower, "escreva") == 0) info.atomo = ATOMO_ESCREVA;
            else if (strcmp(lower, "faca") == 0) info.atomo = ATOMO_FACA;
            else if (strcmp(lower, "falso") == 0) info.atomo = ATOMO_FALSO;
            else if (strcmp(lower, "fim") == 0) info.atomo = ATOMO_FIM;
            else if (strcmp(lower, "funcao") == 0) info.atomo = ATOMO_FUNCAO;
            else if (strcmp(lower, "inicio") == 0) info.atomo = ATOMO_INICIO;
            else if (strcmp(lower, "inteiro") == 0) info.atomo = ATOMO_INTEIRO;
            else if (strcmp(lower, "leia") == 0) info.atomo = ATOMO_LEIA;
            else if (strcmp(lower, "logico") == 0) info.atomo = ATOMO_LOGICO;
            else if (strcmp(lower, "mod") == 0) info.atomo = ATOMO_MOD;
            else if (strcmp(lower, "nao") == 0) info.atomo = ATOMO_NAO;
            else if (strcmp(lower, "ou") == 0) info.atomo = ATOMO_OU;
            else if (strcmp(lower, "procedimento") == 0) info.atomo = ATOMO_PROCEDIMENTO;
            else if (strcmp(lower, "se") == 0) info.atomo = ATOMO_SE;
            else if (strcmp(lower, "senao") == 0) info.atomo = ATOMO_SENAO;
            else if (strcmp(lower, "var") == 0) info.atomo = ATOMO_VAR;
            else if (strcmp(lower, "verdadeiro") == 0) info.atomo = ATOMO_VERDADEIRO;
            else {
                info.atomo = ATOMO_IDENTIFICADOR;
                strcpy(info.atributo.id, lexema); /* Preserva a original */
            }

            info.linha = linha_id;
            ultima_linha_analisada = linha_atual;
            return info;
        }

        /* Operadores e Pontuação */
        info.linha = linha_atual;

        if (buffer_fonte[pos] == ':') {
            if (buffer_fonte[pos + 1] == '=') {
                pos += 2;
                info.atomo = ATOMO_ATRIBUICAO;
            } else {
                pos++;
                info.atomo = ATOMO_DOIS_PONTOS;
            }
            ultima_linha_analisada = linha_atual;
            return info;
        }

        if (buffer_fonte[pos] == '<') {
            if (buffer_fonte[pos + 1] == '>') {
                pos += 2;
                info.atomo = ATOMO_DIFERENTE;
            } else if (buffer_fonte[pos + 1] == '=') {
                pos += 2;
                info.atomo = ATOMO_MENOR_IGUAL;
            } else {
                pos++;
                info.atomo = ATOMO_MENOR;
            }
            ultima_linha_analisada = linha_atual;
            return info;
        }

        if (buffer_fonte[pos] == '>') {
            if (buffer_fonte[pos + 1] == '=') {
                pos += 2;
                info.atomo = ATOMO_MAIOR_IGUAL;
            } else {
                pos++;
                info.atomo = ATOMO_MAIOR;
            }
            ultima_linha_analisada = linha_atual;
            return info;
        }

        if (buffer_fonte[pos] == '=') {
            pos++;
            info.atomo = ATOMO_IGUAL;
            ultima_linha_analisada = linha_atual;
            return info;
        }

        if (buffer_fonte[pos] == ';') {
            pos++;
            info.atomo = ATOMO_PONTO_VIRGULA;
            ultima_linha_analisada = linha_atual;
            return info;
        }

        if (buffer_fonte[pos] == ',') {
            pos++;
            info.atomo = ATOMO_VIRGULA;
            ultima_linha_analisada = linha_atual;
            return info;
        }

        if (buffer_fonte[pos] == '.') {
            pos++;
            info.atomo = ATOMO_PONTO;
            ultima_linha_analisada = linha_atual;
            return info;
        }

        if (buffer_fonte[pos] == '(') {
            pos++;
            info.atomo = ATOMO_ABRE_PAR;
            ultima_linha_analisada = linha_atual;
            return info;
        }

        if (buffer_fonte[pos] == ')') {
            pos++;
            info.atomo = ATOMO_FECHA_PAR;
            ultima_linha_analisada = linha_atual;
            return info;
        }

        if (buffer_fonte[pos] == '+') {
            pos++;
            info.atomo = ATOMO_MAIS;
            ultima_linha_analisada = linha_atual;
            return info;
        }

        if (buffer_fonte[pos] == '-') {
            if (buffer_fonte[pos + 1] == '}') {
                printf("# %d: erro lexico, fecha comentario sem abertura\n", linha_atual);
                finalizar_execucao(0);
            }
            pos++;
            info.atomo = ATOMO_MENOS;
            ultima_linha_analisada = linha_atual;
            return info;
        }

        if (buffer_fonte[pos] == '*') {
            pos++;
            info.atomo = ATOMO_VEZES;
            ultima_linha_analisada = linha_atual;
            return info;
        }

        if (buffer_fonte[pos] == '/') {
            pos++;
            info.atomo = ATOMO_BARRA;
            ultima_linha_analisada = linha_atual;
            return info;
        }

        /* Caractere não reconhecido */
        printf("# %d: erro lexico, caractere invalido [%c]\n", linha_atual, buffer_fonte[pos]);
        finalizar_execucao(0);
    }

    info.atomo = ATOMO_FIM_ARQUIVO;
    info.linha = linha_atual;
    return info;
}

/* ============================================================================
 * ANALISADOR SINTÁTICO: consome()
 * ============================================================================ */
void consome(TAtomo atomo) {
    if (lookahead.atomo == atomo) {
        imprimir_atomo(lookahead);
        lookahead = obter_atomo();
        /* Processa e descarta comentários intercalados */
        while (lookahead.atomo == ATOMO_COMENTARIO) {
            imprimir_atomo(lookahead);
            lookahead = obter_atomo();
        }
    } else {
        printf("# %d: erro sintatico, esperado [%s] encontrado [%s]\n",
               lookahead.linha,
               atomo_para_simbolo(atomo),
               atomo_para_simbolo(lookahead.atomo));
        finalizar_execucao(0);
    }
}

/* ============================================================================
 * REGRAS DA GRAMÁTICA (ASDR)
 * ============================================================================ */

/* <programa> ::= algoritmo identificador ';' <bloco> '.' */
void programa(void) {
    consome(ATOMO_ALGORITMO);
    consome(ATOMO_IDENTIFICADOR);
    consome(ATOMO_PONTO_VIRGULA);
    bloco();
    consome(ATOMO_PONTO);
}

/* <bloco> ::= <declaração_variáveis> <declaração_de_rotinas> <comando_composto> */
void bloco(void) {
    declaracao_variaveis();
    declaracao_de_rotinas();
    comando_composto();
}

/* <declaração_variáveis> ::= [ var <lista_variaveis> ';' { <lista_variaveis> ';' } ] */
void declaracao_variaveis(void) {
    if (lookahead.atomo == ATOMO_VAR) {
        consome(ATOMO_VAR);
        lista_variaveis();
        consome(ATOMO_PONTO_VIRGULA);
        while (lookahead.atomo == ATOMO_IDENTIFICADOR) {
            lista_variaveis();
            consome(ATOMO_PONTO_VIRGULA);
        }
    }
}

/* <lista_variaveis> ::= identificador { ',' identificador } ':' <tipo> */
void lista_variaveis(void) {
    consome(ATOMO_IDENTIFICADOR);
    while (lookahead.atomo == ATOMO_VIRGULA) {
        consome(ATOMO_VIRGULA);
        consome(ATOMO_IDENTIFICADOR);
    }
    consome(ATOMO_DOIS_PONTOS);
    tipo();
}

/* <tipo> ::= caractere | inteiro | logico */
void tipo(void) {
    if (lookahead.atomo == ATOMO_CARACTERE) {
        consome(ATOMO_CARACTERE);
    } else if (lookahead.atomo == ATOMO_INTEIRO) {
        consome(ATOMO_INTEIRO);
    } else if (lookahead.atomo == ATOMO_LOGICO) {
        consome(ATOMO_LOGICO);
    } else {
        printf("# %d: erro sintatico, esperado [tipo] encontrado [%s]\n",
               lookahead.linha, atomo_para_simbolo(lookahead.atomo));
        finalizar_execucao(0);
    }
}

/* <declaração_de_rotinas> ::= { <declaração_de_função> | <declaração_de_procedimento> } */
void declaracao_de_rotinas(void) {
    while (lookahead.atomo == ATOMO_FUNCAO || lookahead.atomo == ATOMO_PROCEDIMENTO) {
        if (lookahead.atomo == ATOMO_FUNCAO) {
            declaracao_de_funcao();
        } else {
            declaracao_de_procedimento();
        }
    }
}

/* <declaração_de_função> ::= funcao <tipo> identificador <parametros_formais> <declaracao_variaveis> <comando_composto> */
void declaracao_de_funcao(void) {
    consome(ATOMO_FUNCAO);
    tipo();
    consome(ATOMO_IDENTIFICADOR);
    parametros_formais();
    declaracao_variaveis();
    comando_composto();
}

/* <declaração_de_procedimento> ::= procedimento identificador <parametros_formais> <declaracao_variaveis> <comando_composto> */
void declaracao_de_procedimento(void) {
    consome(ATOMO_PROCEDIMENTO);
    consome(ATOMO_IDENTIFICADOR);
    parametros_formais();
    declaracao_variaveis();
    comando_composto();
}

/* <parâmetros_formais> ::= '(' <parâmetro_formal> { ';' <parâmetro_formal> } ')' | '(' ')' */
void parametros_formais(void) {
    consome(ATOMO_ABRE_PAR);
    if (lookahead.atomo != ATOMO_FECHA_PAR) {
        parametro_formal();
        while (lookahead.atomo == ATOMO_PONTO_VIRGULA) {
            consome(ATOMO_PONTO_VIRGULA);
            parametro_formal();
        }
    }
    consome(ATOMO_FECHA_PAR);
}

/* <parâmetro_formal> ::= [var] <lista_variaveis> */
void parametro_formal(void) {
    if (lookahead.atomo == ATOMO_VAR) {
        consome(ATOMO_VAR);
    }
    lista_variaveis();
}

/* <comando_composto> ::= inicio <comando> { ';' <comando> } fim */
void comando_composto(void) {
    consome(ATOMO_INICIO);
    comando();
    while (lookahead.atomo == ATOMO_PONTO_VIRGULA) {
        consome(ATOMO_PONTO_VIRGULA);
        comando();
    }
    consome(ATOMO_FIM);
}

/*
 * <comando> ::= <comando_atribuição>
 *             | <comando_entrada>
 *             | <comando_saida>
 *             | <comando_condicional>
 *             | <comando_repeticao>
 *             | <chamada_procedimento>
 *             | <comando_composto>
 */
void comando(void) {
    if (lookahead.atomo == ATOMO_IDENTIFICADOR) {
        /* Fatoração: diferencia atribuição de chamada de procedimento */
        consome(ATOMO_IDENTIFICADOR);
        if (lookahead.atomo == ATOMO_ATRIBUICAO) {
            consome(ATOMO_ATRIBUICAO);
            expressao();
        } else if (lookahead.atomo == ATOMO_ABRE_PAR) {
            consome(ATOMO_ABRE_PAR);
            lista_expressao();
            consome(ATOMO_FECHA_PAR);
        }
        /* Caso sem argumentos: chamada de procedimento concluída */
    } else if (lookahead.atomo == ATOMO_LEIA) {
        comando_entrada();
    } else if (lookahead.atomo == ATOMO_ESCREVA) {
        comando_saida();
    } else if (lookahead.atomo == ATOMO_SE) {
        comando_condicional();
    } else if (lookahead.atomo == ATOMO_ENQUANTO) {
        comando_repeticao();
    } else if (lookahead.atomo == ATOMO_INICIO) {
        comando_composto();
    } else {
        printf("# %d: erro sintatico, esperado [comando] encontrado [%s]\n",
               lookahead.linha, atomo_para_simbolo(lookahead.atomo));
        finalizar_execucao(0);
    }
}

/* <comando_entrada> ::= leia '(' identificador { ',' identificador } ')' */
void comando_entrada(void) {
    consome(ATOMO_LEIA);
    consome(ATOMO_ABRE_PAR);
    consome(ATOMO_IDENTIFICADOR);
    while (lookahead.atomo == ATOMO_VIRGULA) {
        consome(ATOMO_VIRGULA);
        consome(ATOMO_IDENTIFICADOR);
    }
    consome(ATOMO_FECHA_PAR);
}

/* <comando_saida> ::= escreva '(' <lista_expressão> ')' */
void comando_saida(void) {
    consome(ATOMO_ESCREVA);
    consome(ATOMO_ABRE_PAR);
    lista_expressao();
    consome(ATOMO_FECHA_PAR);
}

/* <comando_condicional> ::= se <expressão> entao <comando> [senao <comando>] */
void comando_condicional(void) {
    consome(ATOMO_SE);
    expressao();
    consome(ATOMO_ENTAO);
    comando();
    if (lookahead.atomo == ATOMO_SENAO) {
        consome(ATOMO_SENAO);
        comando();
    }
}

/* <comando_repeticao> ::= enquanto <expressão> faca <comando> */
void comando_repeticao(void) {
    consome(ATOMO_ENQUANTO);
    expressao();
    consome(ATOMO_FACA);
    comando();
}

/* <lista_expressão> ::= <expressão> { ',' <expressão> } */
void lista_expressao(void) {
    expressao();
    while (lookahead.atomo == ATOMO_VIRGULA) {
        consome(ATOMO_VIRGULA);
        expressao();
    }
}

/* <expressão> ::= <expressão_simples> [ <operador_relacional> <expressão_simples> ] */
void expressao(void) {
    expressao_simples();
    if (lookahead.atomo == ATOMO_DIFERENTE ||
        lookahead.atomo == ATOMO_MENOR ||
        lookahead.atomo == ATOMO_MENOR_IGUAL ||
        lookahead.atomo == ATOMO_MAIOR_IGUAL ||
        lookahead.atomo == ATOMO_MAIOR ||
        lookahead.atomo == ATOMO_IGUAL) {
        consome(lookahead.atomo);
        expressao_simples();
    }
}

/* <expressão_simples> ::= <termo> { <operador_adição> <termo> } */
void expressao_simples(void) {
    termo();
    while (lookahead.atomo == ATOMO_MAIS ||
           lookahead.atomo == ATOMO_MENOS ||
           lookahead.atomo == ATOMO_MOD ||
           lookahead.atomo == ATOMO_OU) {
        consome(lookahead.atomo);
        termo();
    }
}

/* <termo> ::= <fator> { <operador_multiplicação> <fator> } */
void termo(void) {
    fator();
    while (lookahead.atomo == ATOMO_VEZES ||
           lookahead.atomo == ATOMO_BARRA ||
           lookahead.atomo == ATOMO_DIV ||
           lookahead.atomo == ATOMO_E) {
        consome(lookahead.atomo);
        fator();
    }
}

/*
 * <fator> ::= identificador [ '(' <lista_expressão> ')' ]
 *           | constint
 *           | constchar
 *           | '(' <expressão> ')'
 *           | ('+' | '-' | nao) <fator>
 *           | verdadeiro
 *           | falso
 */
void fator(void) {
    if (lookahead.atomo == ATOMO_IDENTIFICADOR) {
        consome(ATOMO_IDENTIFICADOR);
        if (lookahead.atomo == ATOMO_ABRE_PAR) {
            consome(ATOMO_ABRE_PAR);
            lista_expressao();
            consome(ATOMO_FECHA_PAR);
        }
    } else if (lookahead.atomo == ATOMO_CONSTINT) {
        consome(ATOMO_CONSTINT);
    } else if (lookahead.atomo == ATOMO_CONSTCHAR) {
        consome(ATOMO_CONSTCHAR);
    } else if (lookahead.atomo == ATOMO_ABRE_PAR) {
        consome(ATOMO_ABRE_PAR);
        expressao();
        consome(ATOMO_FECHA_PAR);
    } else if (lookahead.atomo == ATOMO_MAIS ||
               lookahead.atomo == ATOMO_MENOS ||
               lookahead.atomo == ATOMO_NAO) {
        consome(lookahead.atomo);
        fator();
    } else if (lookahead.atomo == ATOMO_VERDADEIRO) {
        consome(ATOMO_VERDADEIRO);
    } else if (lookahead.atomo == ATOMO_FALSO) {
        consome(ATOMO_FALSO);
    } else {
        printf("# %d: erro sintatico, esperado [fator] encontrado [%s]\n",
               lookahead.linha, atomo_para_simbolo(lookahead.atomo));
        finalizar_execucao(0);
    }
}

/* ============================================================================
 * FUNÇÃO PRINCIPAL: main()
 * ============================================================================ */
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <nome_do_arquivo_fonte>\n", argv[0]);
        return 1;
    }

    FILE *arquivo = fopen(argv[1], "rb");
    if (!arquivo) {
        perror("Erro ao abrir arquivo fonte");
        return 1;
    }

    /* Lê todo o arquivo fonte para a memória */
    fseek(arquivo, 0, SEEK_END);
    long tamanho = ftell(arquivo);
    fseek(arquivo, 0, SEEK_SET);

    buffer_fonte = (char *)malloc(tamanho + 1);
    if (!buffer_fonte) {
        fclose(arquivo);
        return 1;
    }

    size_t lidos = fread(buffer_fonte, 1, tamanho, arquivo);
    buffer_fonte[lidos] = '\0';
    fclose(arquivo);

    /* Inicializa o analisador léxico */
    pos = 0;
    linha_atual = 1;
    ultima_linha_analisada = 1;

    lookahead = obter_atomo();

    /* Processa e exibe comentários iniciais */
    while (lookahead.atomo == ATOMO_COMENTARIO) {
        imprimir_atomo(lookahead);
        lookahead = obter_atomo();
    }

    /* Inicia a análise sintática pelo símbolo inicial <programa> */
    programa();

    /* Processa comentários restantes após o ponto final */
    while (lookahead.atomo == ATOMO_COMENTARIO) {
        imprimir_atomo(lookahead);
        lookahead = obter_atomo();
    }

    printf("%d linhas analisadas, programa sintaticamente correto\n", ultima_linha_analisada);

    finalizar_execucao(0);
    return 0;
}