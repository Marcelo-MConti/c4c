#include <curses.h>
#include <stdio.h>
#include <string.h>

#include <err.h>

#include "util.h"
#include "ui/menu.h"

enum {
    PREVIOUS,
    NEXT
};

/**
  Desenha uma única entrada do menu na linha atual da janela.
 
  A função posiciona o cursor, limpa a linha e renderiza o conteúdo da entrada
  conforme seu tipo (texto, selecionável, roleta, campo de input ou condicional).
 
  Retorno:
   - true  → a entrada foi desenhada com sucesso
   - false → a entrada não deve ser exibida (entrada condicional não atendida)
   - -1    → usado internamente para sinalizar que não há entrada válida para desenhar
 */
static bool draw_entry(struct menu *menu, int index)
{
    int winx;
    int cury, curx;
    // pega o tamanho da largura da tela e armazena em winx
    getmaxyx(menu->win, (int){0}, winx);
    getyx(menu->win, cury, curx);
    // Recebe a entrada do menu respondente ao index
    union entry_un *ent = (*menu->entries)[index];

    // Verifica se a entrada é uma entrada que depende de uma condição
    if (ent->common.type == ENTRY_CONDITIONAL) {
        // Caso a condição seja atendida
        if (ent->conditional.condition(menu))
            ent = ent->conditional.entry;// Recebe a entrada que será desenhada na tela
        else
            return false;// Caso contrário não desenha nada
    }

    // Se estiver desenhando o item que está atualmente selecionado 
    if (index == menu->cur_entry)
        wattrset(menu->win, A_REVERSE);

    fill(menu->win, curx, winx - 2, ' ');// Apaga a linha atual
    wmove(menu->win, cury, curx);// Leva o ponteiro para o começo para desenhar denovo

    switch (ent->common.type) {
        case ENTRY_TEXT:
        case ENTRY_SELECTABLE: ;
            /* -2: bordas */
            int newx = winx - utf8len(ent->text.text) - 2;// Calcula a posição do texto para ficar alinhado a direita
            mvwaddstr(menu->win, cury, newx, ent->text.text);

            break;
        case ENTRY_ROULETTE:
            wprintw(menu->win, " %s: ", ent->roulette.text);
            waddstr(menu->win, "< ");

            int len = utf8len(ent->roulette.alt[ent->roulette.cur_option]);

            /* -1: borda, -2: "> " no final */
            mvwaddstr(menu->win, cury, winx - 1 - 2 - len,
                    ent->roulette.alt[ent->roulette.cur_option]);// Escreve a opção de jogo selecionada
            waddstr(menu->win, "> ");
            break;
        case ENTRY_INPUT:
            mvwprintw(menu->win, cury, 1, " %s: ", ent->input.text);// Escreve o input text seguido de :

            if (!ent->input.buf[0]) {
                mvwaddch(menu->win, cury, winx - 3, '_');
            } else {
                // O sistema de input não suporta UTF-8, então usar `utf8len`
                // aqui seria semanticamente incorreto
                int len = strlen(ent->input.buf);

                wmove(menu->win, cury, winx - ((len < 15) ? len : 15) - 2);
                print_truncate(menu->win, ent->input.buf, len, 15);
            }

            break;
        case ENTRY_CONDITIONAL:
            return -1;
    }

    wattrset(menu->win, A_NORMAL);// Volta ao atributo normal

    return true;
}

/**
  Desenha todo o menu na janela associada.
 
  A função limpa a janela, desenha a moldura (se habilitada), posiciona o cursor
  após a borda e então renderiza todas as entradas do menu em sequência.
  Enquanto desenha, registra a linha visual (cury) correspondente à entrada
  atualmente selecionada.
 
  Retorno:
 *  - ≥ 0 → linha vertical (y) onde a entrada selecionada foi desenhada;
 *  - -1  → falha ao desenhar alguma entrada (ex.: entrada condicional não exibida).
 */
static int draw_menu(struct menu *menu)
{
    wclrtobot(menu->win);// Limpa todos os caracteres da janela

    if (menu->box)
        box(menu->win, 0, 0);// Desenha as bordas da janela

    wmove(menu->win, 1, 1);

    int cury, curx, tmp = -1;

    for (int i = 0; (*menu->entries)[i]; i++) {
        getyx(menu->win, cury, curx);// Pega a posição do cursor em x e y e armazena em curx e cury

        if (draw_entry(menu, i)) {// Para todas as entradas do menu chama-se draw_entry para desenhar
            if (i == menu->cur_entry)
                tmp = cury;
            wmove(menu->win, cury + 1, 1);
        } else {
            return -1;
        }
    }

    return tmp;
}

/** Retorna o índice da próxima/última entrada visível no menu */
static int seek_to_entry(struct menu *menu, int whence)
{
    int inc = menu->cur_entry;

    while (true) {
        union entry_un *ent;

        if (whence == PREVIOUS && inc == 0)
            return 0;

        ent = (*menu->entries)[whence == PREVIOUS ? --inc : ++inc];

        if (whence == NEXT && !ent)
            return menu->cur_entry;

        if (ent->common.type == ENTRY_CONDITIONAL && !ent->conditional.condition(menu))
            continue;

        return inc;
    }
}

/** Desenha a barra de entrada */
static void draw_input(WINDOW *win, int x, int y, int width,
        struct in_ent *input, int str_idx, int curs_pos, int len)
{
    // Move o cursor para a posição onde o campo de input começa
    wmove(win, y, x);
    // Preenche toda a área do campo com espaços usando atributos invertidos (A_REVERSE)
    // e apagados (A_DIM)
    fill(win, x, width, ' ' | A_REVERSE | A_DIM);

    for (int i = str_idx - curs_pos;
            i - (str_idx - curs_pos) < width && i < len; i++)
        waddch(win, input->buf[i] | A_REVERSE | A_DIM); // Desenha o caractere na tela usando os mesmos atributos do fundo
    
    // Se a posição do cursor for maior que a largura visível,
    // ele é colocado no último caractere (width - 1).
    wmove(win, y, x + (curs_pos >= width ? width - 1 : curs_pos));
}


/** Lê uma entrada do usuário e chama a função de validação */
static void get_input(struct in_ent *input, void (*on_redraw)(WINDOW *, void *), WINDOW *menu_win, void *ctx)
{
    init_pair(1, COLOR_RED, COLOR_BLACK);// Cor para texto

    int inwidth = 30;// Tamanho para input

    int winx = inwidth + 2, winy = 5; // Calcula tamanho da janela do input
    int offx = (COLS - winx) / 2, offy = (LINES - winy) / 2; // Cálculo para colocar a janela no centro
    int curx;

    WINDOW *inbox = newwin(winy, winx, offy, offx);

    keypad(inbox, TRUE);
    curs_set(1);
    box(inbox, 0, 0);// Desenha bordas

    mvwaddstr(inbox, 1, 1, input->description);// Escreve o texto descrito no campo

    int str_idx = 0, curs_pos = 0;
    int len = input->buf[0] ? strlen(input->buf) : 0;
    draw_input(inbox, 1, 3, inwidth, input, str_idx, curs_pos, len);// Desenha o campo de entrada 

    bool redraw = false;

    int ch;
    while ((ch = wgetch(inbox))) {// Lê teclas digitadas na área de input
        getyx(inbox, (int){0}, curx);// Pega posição atual do cursor dentro do input

        if (redraw) {
            refresh();
            redraw = false;            
        }
        
        switch (ch) {
            case KEY_LEFT:
                if (str_idx > 0) {
                    if (curs_pos > 0)
                        curs_pos--;

                    draw_input(inbox, 1, 3, inwidth, input, --str_idx, curs_pos, len);
                }

                break;
            case KEY_RIGHT:
                if (str_idx < len) {
                    if (curs_pos < inwidth)
                        curs_pos++;

                    draw_input(inbox, 1, 3, inwidth, input, ++str_idx, curs_pos, len);
                }

                break;
            case KEY_HOME:
                if (str_idx != 0 && curs_pos != 0) {
                    str_idx = 0;
                    curs_pos = 0;

                    draw_input(inbox, 1, 3, inwidth, input, str_idx, curs_pos, len);
                }

                break;
            case KEY_END:
                if (str_idx != len && curs_pos != len % inwidth) {
                    str_idx = len;
                    curs_pos = len % inwidth;

                    draw_input(inbox, 1, 3, inwidth, input, str_idx, curs_pos, len);
                }

                break;
            case '\n':
            case KEY_ENTER:
                if (input->validate && input->buf[0]) {
                    const char *err = input->validate(input->buf);

                    if (err) { // Se a validação der erro
                        wattrset(stdscr, COLOR_PAIR(1));
                        mvwaddstr(stdscr, LINES - 2, (COLS - utf8len(err)) / 2, err);
                        wattrset(stdscr, COLOR_PAIR(0));

                        redraw = true;

                        draw_input(inbox, 1, 3, inwidth, input, str_idx, curs_pos, len);
                        continue;
                    }
                }
                //Se ocorrer tudo bem fecha a janela de input
                delwin(inbox);
                curs_set(0);

                return;
            case KEY_DC:
                if (str_idx < len) {
                    memmove(&input->buf[str_idx], &input->buf[str_idx + 1], len-- - str_idx);
                    input->buf[len] = 0;

                    draw_input(inbox, 1, 3, inwidth, input, str_idx, curs_pos, len);
                }

                break;
            // Caracteres ASCII: alfanuméricos + símbolos
            case ' ' ... '~':
                if (len < input->bufsize) {
                    memmove(&input->buf[str_idx + 1], &input->buf[str_idx], len++ - str_idx);
                    input->buf[str_idx++] = ch;

                    if (curs_pos < inwidth)
                        curs_pos++;

                    draw_input(inbox, 1, 3, inwidth, input, str_idx, curs_pos, len);
                }

                break;
            // Várias formas diferentes de representar backspace...
            // Espero que uma delas funcione no seu terminal.
            case KEY_BACKSPACE:
            case '8' & 0x1f:
            case '?' & 0x1f:
            case 127:
            case '\b':
                if (str_idx > 0 && curs_pos > 0) {
                    memmove(&input->buf[str_idx - 1], &input->buf[str_idx], len-- - str_idx + 1);
                    input->buf[len] = 0;

                    draw_input(inbox, 1, 3, inwidth, input, --str_idx, --curs_pos, len);
                }
            case KEY_RESIZE:
                on_redraw(menu_win, ctx);

                //Calcula nova posição central da tela ao ser redimensionada
                offx = (COLS - winx) / 2;// Calcula
                offy = (LINES - winy) / 2;
                
                mvwin(inbox, offy, offx);
                draw_input(inbox, 1, 3, inwidth, input, str_idx, curs_pos, len);

                // Chamadas para sincronizas e atualizar janelas
                touchwin(inbox);
                wsyncup(inbox);

                wnoutrefresh(stdscr);
                wnoutrefresh(inbox);

                doupdate();

                break;
        }
    }

    reset_color_pairs();
}

/*
 A função show_menu exibe e controla a navegação de um menu interativo usando ncurses, lidando com entrada do usuário
 para mover a seleção ou ativar itens. Ela redesenha o menu conforme necessário e retorna o índice da entrada selecionada
 ou -1 em caso de erro.
 */
int show_menu(struct menu *menu, void (*on_redraw)(WINDOW *, void *ctx), void *ctx)
{
    if (!menu || !menu->entries)// Verifica se o menu é valido
        return -1;

    if (menu->box)
        box(menu->win, 0, 0);// Desenha a borda 

    keypad(menu->win, TRUE);// Habilita captura de teclas especiais do teclado
    wmove(menu->win, 1, 1);// Move o ponteiro para não desenhar na borda

    draw_menu(menu);

    WINDOW *parent = wgetparent(menu->win);// Retorna a janela pai do menu

    int ch;
    while ((ch = wgetch(menu->win)) != ERR) {// Enquanto não der erro lê a tecla do usuário
        union entry_un *ent = (*menu->entries)[menu->cur_entry];

        if (ent->common.type == ENTRY_CONDITIONAL)
            ent = ent->conditional.entry;

        // Atualizar as áreas gráficas sem piscar a tela
        wnoutrefresh(parent);
        wnoutrefresh(stdscr);

        doupdate();

        switch (ch) {
            case KEY_UP: ;
                int prev_ent = seek_to_entry(menu, PREVIOUS);// Move seleção para cima

                if (menu->cur_entry != prev_ent) {
                    menu->cur_entry = prev_ent;// Faz a entrada do cursor ser a posição da seleção a cima

                    wmove(menu->win, 1, 1);// Prepara a tela para ser redesenhada
                    int vis_cur_y = draw_menu(menu); // Redesenha retornando a posição que está selecionada atualmente
                    wmove(menu->win, vis_cur_y - 1, 1);
                }

                break;
            case KEY_DOWN: ;
                int next_ent = seek_to_entry(menu, NEXT);// Move seleção para baixo

                if (menu->cur_entry != next_ent) {
                    menu->cur_entry = next_ent;

                    wmove(menu->win, 1, 1);
                    int vis_cur_y = draw_menu(menu);
                    wmove(menu->win, vis_cur_y, 1);
                }

                break;
            case KEY_LEFT:
                if (ent->common.type == ENTRY_ROULETTE) {// andar para esquerda no ENTRY_ROULETTE
                    struct roul_ent *tmp = &ent->roulette;

                    if (tmp->cur_option != 0) {
                        tmp->cur_option--;

                        wmove(menu->win, 1, 1);
                        int vis_cur_y = draw_menu(menu);
                        wmove(menu->win, vis_cur_y, 1);
                    }
                }

                break;
            case KEY_RIGHT:
                if (ent->common.type == ENTRY_ROULETTE) { // Andar para esquerda no ENTRY_ROULETTE
                    struct roul_ent *tmp = &ent->roulette;

                    if (tmp->alt[tmp->cur_option + 1]) {
                        tmp->cur_option++;

                        wmove(menu->win, 1, 1);
                        int vis_cur_y = draw_menu(menu);
                        wmove(menu->win, vis_cur_y, 1);
                    }
                }

                break;
            case '\n':
            case KEY_ENTER:
            case ' ':
                switch (ent->common.type) {
                    case ENTRY_SELECTABLE:// Caso seja um item selecionavel retorna essa entrada
                        return menu->cur_entry;
                    case ENTRY_CONDITIONAL:// Caso seja um item não selecionavel retorna erro
                        return -1;
                    case ENTRY_INPUT:// Caso seja input
                        get_input(&ent->input, on_redraw, menu->win, ctx);// permite usuário escrever

                        draw_menu(menu);// Redesenha o menu
                        on_redraw(menu->win, ctx);
                    default:
                        putchar('\a');
                }

                break;
            case KEY_RESIZE:
                on_redraw(menu->win, ctx);// Redesenha caso haja um redimensionamento
        }

        touchwin(menu->win);
        wsyncup(menu->win);// Sincroniza a tela com a tela superior

        wnoutrefresh(menu->win); // Copia a tela para o virtualscreen
        wnoutrefresh(parent);// Copia a tela pai para o virtualscreen

        doupdate();
    }

    return -1;
}
