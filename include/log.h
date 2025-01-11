#ifndef __LOG_H__
#define __LOG_H__
#define LOG_LEVEL 4

void log_success(char *s, ...);
void log_info(char *s, ...);
void log_warning(char *s, ...);
void log_error(char *s, ...);
void log_debug(char *s, ...);

#endif