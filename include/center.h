#ifndef _CENTER_H_
#define _CENTER_H_

#include <curses.h>

/* Sets up a signal handler for SIGWINCH and allocates memory */
void center_init(void);

/*
 * Adds a window to the queue to get centered once the terminal is resized
 * Windows will be drawn from lowest to highest z-index
 */
void center_win_add(WINDOW *win, int z_index, int y_off, int x_off);

/* Removes a window from the queue */
void center_win_remove(WINDOW *win);

/* Centers the windows without a signal */
void center_win_trigger(void);

#endif /* _CENTER_H_ */
