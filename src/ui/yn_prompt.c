#include <string.h>
#include <curses.h>

#include <libintl.h>

#include "util.h"
#include "ui/yn_prompt.h"

#define _(x) gettext(x)
#define N_(x) x

#define N 0
#define Y 1

#define MAX_LABELS 2

const char *labels[][MAX_LABELS] = {
    [YN_LABEL_YES_NO] = { N_("No"), N_("Yes") },
    [YN_LABEL_OK_CANCEL] = { N_("Cancel"), N_("OK") }
};

/**
  Exibe uma janela de confirmação do tipo "Yes/No" ou "OK/Cancel" usando ncurses.
 
  A função cria uma pequena janela centralizada contendo um texto e duas opções
  selecionáveis (rótulos variam conforme o enum yn_prompt_label). O usuário pode
  navegar pelas opções com as setas esquerda/direita ou Tab, e confirmar com
  Enter. A janela é redesenhada automaticamente em caso de redimensionamento.

  Parâmetros:
 *   - text: Texto exibido na linha superior da janela (mensagem do prompt).
 *   - type: Tipo de rótulos usados (YES/NO, OK/CANCEL), definidos em yn_prompt_label.
 *   - on_redraw: Função callback chamada quando ocorre KEY_RESIZE, permitindo
 *                que o chamador redesenhe a tela conforme necessário.
    - ctx: Ponteiro opcional passado para o callback on_redraw.
 
  Retorno:
 *   - 0 (false)  → se o usuário selecionou o primeiro rótulo (NO ou CANCEL).
 *   - 1 (true)   → se o usuário selecionou o segundo rótulo (YES ou OK).
 
 */

bool show_yn_prompt(char *text, enum yn_prompt_label type, void (*on_redraw)(WINDOW *, void *ctx), void *ctx)
{
    const int width = 35;
    const int height = 5;
    WINDOW *win = newwin(height, width, 1, 1);// Cria a janela do prompt

    box(win, 0, 0);
    keypad(win, TRUE);

    // Calcula o tamanho dos rótulos traduzidos
    size_t y_label_len = utf8len(_(labels[type][Y]));
    size_t n_label_len = utf8len(_(labels[type][N]));

    // Calcula espaço total ocupado
    size_t total_len = y_label_len + n_label_len + 4;

    // Espaçamento para deixar os botões centralizados
    size_t space_around = (width - 2 - total_len) / 3;
    size_t space_between = width - 2 - total_len - 2 * space_around;

    int selected = -1;// Começa sem nada selecionado

    mvwaddstr(win, 1, 2, text);

    bool first = true;
    int ch;

    while (true) {
        ch = first ? 0 : wgetch(win);// Na primeira iteração não espera entrada
        
        switch (ch) {
            case KEY_LEFT:
                if (selected == 0)
                    break;

                selected = (selected + MAX_LABELS - 1) % MAX_LABELS;
                break;
            case KEY_RIGHT:
                if (selected == MAX_LABELS - 1)
                    break;

                selected++;
                break;
            case '\t':
                selected = (selected + MAX_LABELS - 1) % MAX_LABELS;
                break;
            case '\n':
            case KEY_ENTER:
                if (selected != -1)
                    return selected;// Retorna a opção selecionada

                break;
            case KEY_RESIZE:
                on_redraw(win, ctx);
                break;
        }

        // Move até onde começa o botão da esquerda
        wmove(win, 3, space_around);

        if (selected == N)// Destaca No ou Cancel
            wattrset(win, A_REVERSE);

        // <No> 
        waddch(win, '<');
        waddstr(win, _(labels[type][N]));
        waddch(win, '>');

        wattrset(win, A_NORMAL);

        // Move até o botão direito
        wmove(win, 3, space_around + n_label_len + 2 + space_between);

        if (selected == Y) // Destaca as opções Yes ou OK
            wattrset(win, A_REVERSE);
        
        waddch(win, '<');
        waddstr(win, _(labels[type][Y]));
        waddch(win, '>');

        wattrset(win, A_NORMAL);

        if (first)
            on_redraw(win, NULL);// Redesenha na primeira exibição

        touchwin(win);
        wsyncup(win);

        wnoutrefresh(win);
        wnoutrefresh(stdscr);

        doupdate();

        first = false;
    }

    return selected;
}
