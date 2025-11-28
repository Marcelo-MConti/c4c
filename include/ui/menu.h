#ifndef _MENU_H_
#define _MENU_H_

#include <curses.h>

/* ENTRY_SELECTABLE: Creates a selectable entry
 * The index of the entry will be returned by
 * `run_menu` when it is selected
 *
 * ENTRY_TEXT: Creates a basic text entry
 *
 * ENTRY_ROULETTE: Creates a "roulette" of options
 *
 * ENTRY_INPUT: Text input that can be validated
 * Won't be validated if @validate is NULL
 *
 * ENTRY_CONDITIONAL: Only show the containing entry
 * if specified condition is true
 * It cannot contain another ENTRY_CONDITIONAL
 */
enum entry_type {
    ENTRY_SELECTABLE,
    ENTRY_TEXT,
    ENTRY_ROULETTE,
    ENTRY_INPUT,
    ENTRY_CONDITIONAL
};

union entry_un;

struct menu {
    int cur_entry;
    /* Must be NULL-terminated */
    union entry_un *(*entries)[];
    WINDOW *win;
};

struct entry {
    enum entry_type type;
};

/* ENTRY_SELECTABLE | ENTRY_TEXT */
struct text_ent {
    enum entry_type type;
    char *text;
};

/* ENTRY_ROULETTE */
struct roul_ent {
    enum entry_type type;
    int cur_option;
    char *text;
    /* @alt corresponds to the legends for each value */
    char **alt;
};

/* ENTRY_INPUT */
struct in_ent {
    enum entry_type type;
    int bufsize;
    char *text, *buf, *description;
    /* @ret: NULL if validated succesfully, error message otherwise */
    const char *(*validate)(char *buf);
};

/* ENTRY_CONDITIONAL */
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

/* Shows a menu on the specified window
 * Returns when an option is selected
 */
int run_menu(struct menu *menu, void (*redraw)(WINDOW *menu_win, void *ctx), void *ctx);

#endif /* _MENU_H_ */
