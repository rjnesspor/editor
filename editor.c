#include "buffer.h"
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define CTRL(x) ((x) & 0x1f)
#define HEADER_PADDING_UNITS 2
#define HEADER_LEFT_MSG "ryans editor!"
#define HEADER_RIGHT_MSG "Editing: "

void draw_commands(WINDOW* win, int height, int width);
void show_message(char* msg);

typedef struct {
    char* key;
    char* description;
} Command;

int main(int ac, char** av) {

    if (ac != 2) {
        fprintf(stderr, "Usage: %s filename\n", av[0]);
        return 1;
    }

    if (access(av[1], F_OK) != 0) {
        fclose(fopen(av[1], "w")); // lol
    }

    struct stat st;
    stat(av[1], &st);
    int filesize = st.st_size;

    /* Ncurses initialization */
    int height, width;
    int textheight, textwidth;
    int optsheight, optswidth;

    initscr();
    raw();
    noecho();

    if (has_colors()) start_color();
    init_pair(1, COLOR_BLUE, COLOR_BLACK);
    init_pair(2, COLOR_CYAN, COLOR_BLACK);
    
    getmaxyx(stdscr, height, width);

    attron(COLOR_PAIR(1));
    box(stdscr, 0, 0);
    attroff(COLOR_PAIR(1));
    attron(COLOR_PAIR(2));
    mvprintw(0, 2, HEADER_LEFT_MSG);
    mvprintw(0, width - strlen(HEADER_RIGHT_MSG) - strlen(av[1]) - HEADER_PADDING_UNITS, "%s%s", HEADER_RIGHT_MSG, av[1]);
    mvprintw(height - 1, HEADER_PADDING_UNITS, "%d bytes", filesize);
    attroff(COLOR_PAIR(2));

    refresh();

    WINDOW* textw = newwin(height - 3, width - 2, 1, 1);
    WINDOW* optsw = newwin(1, width - 2, height - 2, 1);

    getmaxyx(textw, textheight, textwidth);
    getmaxyx(optsw, optsheight, optswidth);

    draw_commands(optsw, optsheight, optswidth);

    keypad(textw, TRUE);
    
    /* Buffer */
    Buffer* buf = malloc(sizeof(Buffer));
    buffer_init(buf);

    buffer_load(buf, av[1]);
    buffer_render(buf, textw);

    bool quit = false;
    while (!quit) {
        int ch = wgetch(textw);
        switch (ch) {
            // ts doesn't work yet
            // vvv
            case CTRL('c'):
                buffer_copy(buf);
                break;
            case CTRL('v'):
                mvprintw(0, 0, "ctrlv ");
                //buffer_paste(buf);
                break;
            case CTRL('x'):
                buffer_copy(buf);
                memset(buf->lines[buf->cursor_y], '\0', MAX_LINE_LEN);
                break;
            // ^^^
            case CTRL('d'):
                buffer_save(buf, av[1]);
                quit = true;
                break;
            case CTRL('s'):
                buffer_save(buf, av[1]);
                break;
            case CTRL('a'):
                quit = true;
                break;
            case KEY_BACKSPACE:
            case 127:
                buffer_delete_char(buf);
                break;
            case '\n':
            case KEY_ENTER:
                buffer_insert_newline(buf);
                break;
            case KEY_LEFT:
                buf->cursor_x--;
                buffer_clamp_cursor(buf);
                break;
            case KEY_RIGHT:
                buf->cursor_x++;
                buffer_clamp_cursor(buf);
                break;
            case KEY_UP:
                buf->cursor_y--;
                buffer_clamp_cursor(buf);
                break;
            case KEY_DOWN:
                buf->cursor_y++;
                buffer_clamp_cursor(buf);
                break;
            default:
                buffer_insert_char(buf, ch);
                break;
        }

        buffer_scroll(buf, height - 3);
        buffer_render(buf, textw);

    }

    endwin();
    return 0;

}

void draw_commands(WINDOW* win, int height, int width) {
    // defn commands
    Command commands[] = {
        {"^X", "Cut"},
        {"^C", "Copy"},
        {"^V", "Paste"},
        {"^D", "Save+Exit"},
        {"^S", "Save"},
        {"^A", "Exit"}
    };
    int numcmds = sizeof(commands) / sizeof(commands[0]);

    int per_row = 6;
    int num_rows = (numcmds + per_row - 1) / per_row;

    int col_width = width / per_row;
    int start_y = (height - num_rows) / 2;

    for (int i = 0; i < numcmds; i++) {
        int row = i / per_row;
        int col = i % per_row;

        char formatted[64];
        snprintf(formatted, sizeof(formatted), "%s %-10s", commands[i].key, commands[i].description);
        int text_len = strlen(formatted);

        int col_start = col * col_width;
        int x_pos = col_start + (col_width - text_len) / 2;
        if (x_pos < 0) x_pos = col_start;
        
        int y_pos = start_y + row;

        mvwprintw(win, y_pos, x_pos, "%s", formatted);
    }

    wrefresh(win);
}
