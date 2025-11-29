#ifndef _UI_YN_PROMPT_
#define _UI_YN_PROMPT_

#include <curses.h>

enum yn_prompt_label {
    YN_LABEL_YES_NO,
    YN_LABEL_OK_CANCEL,
};

bool show_yn_prompt(char *text, enum yn_prompt_label type, void (*on_redraw)(WINDOW *, void *ctx), void *ctx);

#endif /* _UI_YN_PROMPT_ */
