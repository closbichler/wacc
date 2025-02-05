#ifndef GAME_H_
#define GAME_H_

void fill_rect(int x, int y, int w, int h, unsigned int color);
void clear_rect(int x, int y, int w, int h);
void fill_text(int x, int y, const char* text, int size, unsigned int color);
void fill_text_centered(int x, int y, const char* text, int size, unsigned int color);
void fill_path(int x, int y, const char* path, float size, unsigned int color);
void log_console(const char* text);

#endif // GAME_H_