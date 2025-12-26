#ifndef BUFFER_H
#define BUFFER_H

#include <ncurses.h>

#define MAX_LINES 1000
#define MAX_LINE_LEN 256

typedef struct {
    char lines[MAX_LINES][MAX_LINE_LEN];
    int num_lines;
    int cursor_y;
    int cursor_x;
    int dirty;
    int scroll_offset;
    char clipboard[MAX_LINE_LEN];
} Buffer;

void buffer_init(Buffer* buf);
void buffer_insert_char(Buffer* buf, char c);
void buffer_delete_char(Buffer* buf);
void buffer_render(Buffer* buf, WINDOW* win);
void buffer_clamp_cursor(Buffer* buf);
void buffer_insert_newline(Buffer* buf);
void buffer_scroll(Buffer* buf, int height);

void buffer_load(Buffer* buf, char* filename);
void buffer_save(Buffer* buf, char* filename);

void buffer_copy(Buffer* buf);
int buffer_paste(Buffer* buf);

#endif
