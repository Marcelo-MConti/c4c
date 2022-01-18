#ifndef _MENU_H_
#define _MENU_H_

#include <ncurses.h>

/* ENTRY_SELECTABLE: Creates a selectable entry
 * Menu will return index of the entry
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

/* Generic union for an entry, uses C99
 * common initial sequence to avoid casting
 */
union entry_un;

struct menu {
    int cur_entry;
    /* Must be NULL-terminated */
    union entry_un **entries;
    WINDOW *win;
    /* These windows will be kept centered as the terminal is resized
     * Must be dynamically allocated so that windows created for input
     * entries can also be added to the list
     */
    struct win_off *center;
};

struct entry {
    int type;
};

/* ENTRY_SELECTABLE | ENTRY_TEXT */
struct text_ent {
    int type;
    char *text;
};

/* ENTRY_ROULETTE */
struct roul_ent {
    int type, cur_option;
    char *text;
    /* @alt corresponds to the legends for each value */
    char **alt;
};

/* ENTRY_INPUT */
struct in_ent {
    int type, bufsize;
    char *text, *buf;
    /* @ret: NULL if validated succesfully, error message otherwise */
    char *(*validate)(char *buf);
};

/* ENTRY_CONDITIONAL */
struct cond_ent {
    int type;
    union entry_un *entry;
    int (*condition)(struct menu *menu);
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
int do_menu(struct menu *menu);

#endif /* _MENU_H_ */
