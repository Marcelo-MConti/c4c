#ifndef _UTIL_H_
#define _UTIL_H_

#include <stddef.h>
#include <curses.h>

#include <err.h>

#define MAX(x, y) ((x) > (y) ? (x) : (y))

#define ARR_SIZE(x) sizeof((x)) / sizeof((x)[0])

#define _STR(x) #x
#define STR(x) _STR(x)

#define MIN_LINES 30
#define MIN_COLS 80

/**
 * Calcula o tamanho (quantidade de codepoints)
 * de uma string UTF-8.
 */
size_t utf8len(const char *str);

/**
 * Enche uma linha da janela `win` com o caractere `ch`,
 * a partir da posição `from` e até a posição `to` (inclusivo)
 */
void fill(WINDOW *win, int from, int to, int ch);

/**
 * Imprime uma string de tamanho `len`, truncando-a e
 * exibindo reticências se for maior que `trunc`.
 */
void print_truncate(WINDOW *win, char *str, int len, int trunc);

/** Gera um número aleatório de 32 bits. */
uint32_t rand32(void);

#endif /* _UTIL_H_ */
