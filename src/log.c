#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

#include "log.h"


#define NOCOLOR "\033[0;0m"
#define GREEN "\033[0;32m"
#define BLUE "\033[1;34m"
#define YELLOW "\033[1;33m"
#define RED "\033[0;31m"
#define MAGENTA "\033[1;35m"

/**
 * @brief Log the given message to this format with the given color in the console
 * Only log if the given level is inferior or equal to the current application logging level
 */
void _log(int level, char *message, char *head, char *color)
{

    if(level < LOG_LEVEL)
        return;
    // Get current time
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    // Construct resulting string to log
    char *s;
    asprintf(&s, "%s[%d-%02d-%02d %02d:%02d:%02d] [%s]\t %s%s\n", color, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, head, message, NOCOLOR);

    // Log to the console
    printf("%s", s);

    free(s);
}

/**
 * @brief Log the given message as a "success" in the console and in file if settings allow it
 * Note : This is a variadic function that you can use in the same way as "printf"
 * Minimal required logging level : 0
 */
void log_success(char *format, ...)
{
    va_list args;
    va_start(args, format);

    char *s;
    vasprintf(&s, format, args);

    _log(0, s, "SUCCESS", GREEN);
    free(s);
}

/**
 * @brief Log the given message as an "info" in the console and in file if settings allow it
 * Note : This is a variadic function that you can use in the same way as "printf"
 * Minimal required logging level : 1
 */
void log_info(char *format, ...)
{
    va_list args;
    va_start(args, format);

    char *s;
    vasprintf(&s, format, args);

    _log(1, s, "INFO", BLUE);
    free(s);
}

/**
 * @brief Log the given message as a "warning" in the console and in file if settings allow it
 * Note : This is a variadic function that you can use in the same way as "printf"
 * Minimal required logging level : 2
 */
void log_warning(char *format, ...)
{
    va_list args;
    va_start(args, format);

    char *s;
    vasprintf(&s, format, args);

    _log(2, s, "WARNING", YELLOW);
    free(s);
}

/**
 * @brief Log the given message as an "error" in the console and in file if settings allow it
 * Note : This is a variadic function that you can use in the same way as "printf"
 * Minimal required logging level : 3
 */
void log_error(char *format, ...)
{
    va_list args;
    va_start(args, format);

    char *s;
    vasprintf(&s, format, args);

    _log(3, s, "ERROR", RED);
    free(s);
}

/**
 * @brief Log the given message as a "debug" in the console and in file if settings allow it
 * Note : This is a variadic function that you can use in the same way as "printf"
 * Minimal required logging level : 4
 */
void log_debug(char *format, ...)
{
    va_list args;
    va_start(args, format);

    char *s;
    vasprintf(&s, format, args);

    _log(4, s, "DEBUG", MAGENTA);
    free(s);
}
