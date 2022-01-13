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
union entryun;

struct menu {
    int curentry;
    /* Must be NULL-terminated */
    union entryun **entries;
    WINDOW *win;
};

struct entry {
    int type;
};

/* ENTRY_SELECTABLE | ENTRY_TEXT */
struct textent {
    int type;
    char *text;
};

/* ENTRY_ROULETTE */
struct roulent {
    int type;
    char *text;
    /* @alt corresponds to the legends for each value */
    char **alt;
    int curoption;
};

/* ENTRY_INPUT */
struct inent {
    int type;
    char *text, *buf;
    int (*validate)(char *buf);
};

/* ENTRY_CONDITIONAL */
struct condent {
    int type, condtype;
    union entryun *entry;
    int (*condition)(struct menu *menu);
};

union entryun {
    struct entry common;
    struct textent text;
    struct roulent roulette;
    struct inent input;
    struct condent conditional;
};

/* Shows a menu on the specified window, assumes it
 * has curses borders and that the keypad is enabled.
 * Returns when an option is selected
 */
int domenu(struct menu *menu);

#endif /* _MENU_H_ */
