#ifndef _MENU_H_
#define _MENU_H_

#include <ncurses.h>

/* ENTRY_SELECTABLE: Creates a selectable entry 
 * that will run the specified function when selected
 *
 * ENTRY_TEXT: Creates a basic text entry
 *
 * ENTRY_ROULETTE: Creates a "roulette" of options
 */
enum entry_type {
    ENTRY_SELECTABLE,
    ENTRY_TEXT,
    ENTRY_ROULETTE
};

/* Makes selectable entry return from menu function 
 * and destroy menu resources when called
 */ 
#define SELENT_MENU_RETURN (1 << 0)

struct entry {
    int type;
    char *text;
    /* @data shall be a pointer to the corresponding
     * struct for the entry according to its type     
     */
    void *data;
};

/* ENTRY_SELECTABLE */
struct selent {
    void (*func)(void *arg);
    void *arg;
    int attr;
};

/* ENTRY_ROULETTE */
struct rlent {
    /* @alt corresponds to the legends for each value in @val */
    char **alt;
    /* @store is the pointer to the storage location, has to be a pointer to short */
    void *store;
    int curoption, entrycnt;
};

struct menu {
    struct entry *entries;
    int curentry, entrycnt;
};

/* Returns the index of the entry in case a returnable selectable entry is selected */
int domenu(WINDOW *win, struct menu *menu, int x, int y);

#endif /* _MENU_H_ */
