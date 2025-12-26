#include "buffer.h"
#include <string.h>

void buffer_init(Buffer* buf) {
    buf->num_lines = 1;
    buf->lines[0][0] = '\0';
    buf->cursor_y = 0;
    buf->cursor_x = 0;
    buf->dirty = 0;
    buf->scroll_offset = 0;
}

void buffer_insert_newline(Buffer* buf) {
    if (buf->num_lines >= MAX_LINES) return;

    char* line = buf->lines[buf->cursor_y];
    for (int i = buf->num_lines; i > buf->cursor_y + 1; i--) {
        strcpy(buf->lines[i], buf->lines[i - 1]);
    }

    strcpy(buf->lines[buf->cursor_y + 1], &line[buf->cursor_x]);

    line[buf->cursor_x] = '\0';

    buf->num_lines++;
    buf->cursor_y++;
    buf->cursor_x = 0;
}

void buffer_insert_char(Buffer* buf, char c) {
    if (c == '\n') {
        buffer_insert_newline(buf);
        return;
    }

    char* line = buf->lines[buf->cursor_y];
    int len = strlen(line);

    if (len >= MAX_LINE_LEN - 1) return;

    if (buf->cursor_x > len) {
        buf->cursor_x = len;
    }

    memmove(&line[buf->cursor_x + 1], &line[buf->cursor_x], len - buf->cursor_x + 1);
    line[buf->cursor_x] = c;
    buf->cursor_x++;
    buf->dirty = 1;
}

void buffer_delete_char(Buffer* buf) {
    if (buf->cursor_x == 0 && buf->cursor_y == 0) return; // cannot go before start

    if (buf->cursor_x == 0) {
        // join with previous line
        int prev_len = strlen(buf->lines[buf->cursor_y - 1]);
        char* line = buf->lines[buf->cursor_y];

        if (prev_len + strlen(line) >= MAX_LINE_LEN) return;
        strcat(buf->lines[buf->cursor_y - 1], line);

        for (int i = buf->cursor_y; i < buf->num_lines - 1; i++) {
            strcpy(buf->lines[i], buf->lines[i + 1]);
        }

        buf->num_lines--;
        buf->cursor_y--;
        buf->cursor_x = prev_len;
    } else {
        char* line = buf->lines[buf->cursor_y];
        int len = strlen(line);
        memmove(&line[buf->cursor_x - 1], &line[buf->cursor_x], len - buf->cursor_x + 1);
        buf->cursor_x--;
    }
    buf->dirty = 1;
}

void buffer_render(Buffer* buf, WINDOW* win) {
    int height, width;
    getmaxyx(win, height, width);

    werase(win);
    for (int i = 0; i < height && (i + buf->scroll_offset) < buf->num_lines; i++) {
        wmove(win, i, 0);
        waddstr(win, buf->lines[i + buf->scroll_offset]);
    }
    int screen_y = buf->cursor_y - buf->scroll_offset;
    wmove(win, screen_y, buf->cursor_x);
    wrefresh(win);
}

void buffer_scroll(Buffer* buf, int height) {
    if (buf->cursor_y < buf->scroll_offset) {
        buf->scroll_offset = buf->cursor_y;
    }

    if (buf->cursor_y >= buf->scroll_offset + height) {
        buf->scroll_offset = buf->cursor_y - height + 1;
    }

    if (buf->scroll_offset < 0) {
        buf->scroll_offset = 0;
    }
}

void buffer_clamp_cursor(Buffer* buf) {
    if (buf->cursor_y >= buf->num_lines) {
        buf->cursor_y = buf->num_lines - 1;
    }

    if (buf->cursor_y < 0) {
        buf->cursor_y = 0;
    }

    int len = strlen(buf->lines[buf->cursor_y]);
    if (buf->cursor_x > len) {
        buf->cursor_x = len;
    }
    if (buf->cursor_x < 0) {
        buf->cursor_x = 0;
    }
}

void buffer_load(Buffer* buf, char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return;

    int numlines = 0;
    while (numlines < MAX_LINES && fgets(buf->lines[numlines], MAX_LINE_LEN, file) != NULL) {
        int len = strlen(buf->lines[numlines]);
        if (len > 0 && buf->lines[numlines][len - 1] == '\n') {
            buf->lines[numlines][len - 1] = '\0';
        }
        numlines++;
    }
    fclose(file);
    buf->num_lines = numlines;

    if (buf->num_lines == 0) {
        buf->num_lines = 1;
        buf->lines[0][0] = '\0';
    }

}

void buffer_save(Buffer* buf, char* filename) {
    if (!buf->dirty) return;
    FILE* file = fopen(filename, "w");
    if (!file) return;

    for (int i = 0; i < buf->num_lines; i++) {
        fputs(buf->lines[i], file);
        if (i < buf->num_lines - 1) {
            fputc('\n', file);
        }
    }

    fclose(file);
    buf->dirty = 0;
}

void buffer_copy(Buffer* buf) {
    strcpy(buf->clipboard, buf->lines[buf->cursor_y]);
}

int buffer_paste(Buffer* buf) {
    if (!strlen(buf->clipboard)) return -1;
    strcpy(buf->lines[buf->cursor_y], buf->clipboard);
    return 1;
}