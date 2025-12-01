#ifndef _UI_YN_PROMPT_
#define _UI_YN_PROMPT_

#include <curses.h>

/*
 Enumeração que define os tipos de rótulos do prompt.
 
   YN_LABEL_YES_NO   → exibe botões "Yes" e "No".
   YN_LABEL_OK_CANCEL → exibe botões "OK" e "Cancel".
 */
enum yn_prompt_label {
    YN_LABEL_YES_NO,
    YN_LABEL_OK_CANCEL,
};

/* 
 Exibe uma janela de confirmação do tipo "Yes/No" ou "OK/Cancel" usando ncurses.
 Retorna: 0 se escolheu 0 primerio ou 1 se escolheu o segundo rótulo
*/ 
bool show_yn_prompt(char *text, enum yn_prompt_label type, void (*on_redraw)(WINDOW *, void *ctx), void *ctx);

#endif /* _UI_YN_PROMPT_ */
