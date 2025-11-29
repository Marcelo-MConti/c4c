#ifndef _UI_MENU_H_
#define _UI_MENU_H_

#include <curses.h>

/** Tipos de entrada que um menu suporta. */
enum entry_type {
    /**
     * Entrada selecionável; ao ser selecionada a função `run_menu`
     * retorna o índice da entrada no array de entradas.
     */
    ENTRY_SELECTABLE,
    /** Uma entrada de texto simples. */
    ENTRY_TEXT,
    /**
     * Uma "roleta" que permite selecionar um valor dentre
     * um conjunto especificado como um array de strings
     */
    ENTRY_ROULETTE,
    /**
     * Entrada de texto que opcionalmente será validada por uma função
     * de validação (pode ser `NULL` para que não haja validação)
     */
    ENTRY_INPUT,
    /**
     * Guarda outra entrada, que apenas é exibida se a condição passada
     * retornar `true`. Não pode conter outra entrada do mesmo tipo.
     */ 
    ENTRY_CONDITIONAL
};

union entry_un;

/**
 * Guarda os parâmetros de um menu: um array de ponteiros para
 * entradas, que deve ser `NULL`-terminado e a janela que será
 * usada pelo menu.
 */
struct menu {
    int cur_entry;
    /* Must be NULL-terminated */
    union entry_un *(*entries)[];
    WINDOW *win;
    bool box;
};

struct entry {
    enum entry_type type;
};

/** ENTRY_SELECTABLE | ENTRY_TEXT */
struct text_ent {
    enum entry_type type;
    char *text;
};

/** ENTRY_ROULETTE */
struct roul_ent {
    enum entry_type type;
    int cur_option;
    char *text;
    /** Legenda para cada valor da roleta */
    char **alt;
};

/** ENTRY_INPUT */
struct in_ent {
    enum entry_type type;
    int bufsize;
    char *text, *buf, *description;
    /** Retorna `NULL` se validar corretamente; uma mensagem de erro caso contrário */
    const char *(*validate)(char *buf);
};

/** ENTRY_CONDITIONAL */
struct cond_ent {
    enum entry_type type;
    union entry_un *entry;
    bool (*condition)(struct menu *menu);
};

union entry_un {
    struct entry common;
    struct text_ent text;
    struct roul_ent roulette;
    struct in_ent input;
    struct cond_ent conditional;
};

/**
 * Mostra um menu com os parâmetros dados por `menu`. `redraw` permite adicionar
 * comportamento adicional à renderização do menu.
 */
int show_menu(struct menu *menu, void (*redraw)(WINDOW *menu_win, void *ctx), void *ctx);

#endif /* _UI_MENU_H_ */
