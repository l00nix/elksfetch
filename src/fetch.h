#ifndef FETCH_H
#define FETCH_H
#define FETCH_VERSION "0.1"
#define FETCH_TEXT 96
#define FETCH_ROWS 16
struct fetch_row { const char *label; char value[FETCH_TEXT]; };
struct fetch_info { struct fetch_row rows[FETCH_ROWS]; unsigned count; };
void fetch_add(struct fetch_info *info, const char *label, const char *value);
void fetch_collect(struct fetch_info *info, const char *model);
void fetch_network(struct fetch_info *info);
unsigned fetch_columns(void);
int fetch_is_terminal(void);
void fetch_render(const struct fetch_info *info, unsigned width, int logo, int color);
#endif
