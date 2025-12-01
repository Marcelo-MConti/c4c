#include "util.h"
#include <stdlib.h>
#include <time.h>

// Conta quantos caracteres UTF-8 reais existem na string
size_t utf8len(const char *str)
{
    size_t len;

    for (len = 0; *str; str++)
        if ((*str & 0xc0) != 0x80) len++;

    return len;
}

/** 
  Preenche uma faixa horizontal da linha atual da janela.
 
  Parâmetros:
   - win: janela ncurses onde o texto será desenhado.
   - from: coluna inicial da faixa a preencher.
   - to: coluna final da faixa a preencher.
   - ch: caractere usado para preencher.
 
  A função move o cursor para a coluna 'from', escreve 'ch' do início ao fim
  da faixa, e ao final volta o cursor para a posição inicial da escrita.
 */
void fill(WINDOW *win, int from, int to, int ch)
{
    int cury, curx;
    getyx(win, cury, curx);

    wmove(win, cury, from);
    for (int i = from; i <= to; i++)
        waddch(win, ch);
    wmove(win, cury, from);
}

/**
  Imprime uma string na janela, truncando-a caso ultrapasse o limite.
 
  Parâmetros:
   - win: janela ncurses onde será escrito.
   - str: texto a ser exibido.
   - len: tamanho real da string (em bytes).
   - trunc: limite máximo de caracteres a serem exibidos.
 
  Se 'len' for maior que 'trunc', a função imprime apenas parte da string
  seguida de "..." (ou "…" em UTF-8). Caso contrário, escreve a string inteira.
 */
void print_truncate(WINDOW *win, char *str, int len, int trunc)
{
    if (len > trunc) {
#ifdef C4C_ASCII
        waddnstr(win, str, trunc - 3);
        waddstr(win, "...");
#else
        waddnstr(win, str, trunc - 1);
        waddstr(win, u8"…");
#endif
    } else {
        waddnstr(win, str, len);
    }
}

// Inicializa o gerador de números aleatórios
__attribute__((constructor))
static void srand32()
{
    srand(time(NULL));
}

/**
  Gera um número aleatório de 32 bits usando rand().
  Retorna: um uint32_t contendo um valor pseudo-aleatório de 32 bits.
 */

uint32_t rand32()
{
    static const int n_bits = __builtin_clz(RAND_MAX);//número de bits realmente úteis retornados por rand()

    uint32_t result = 0;

    for (int shifted = 0; shifted < 16; shifted += n_bits)
        result = (result << n_bits) + rand();

    return result;
}
