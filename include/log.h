#ifndef _LOG_H_
#define _LOG_H_

#define LOG_LEVEL_ERR   1
#define LOG_LEVEL_WARN  2
#define LOG_LEVEL_INFO  3
#define LOG_LEVEL_DBG   4

void set_log_level();
int get_log_leval();

#define LOG_PRINT(format, ...) printf("%s:%d  "format"\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define LOG_DEBUG(format, ...) \
    if(get_log_leval() >= LOG_LEVEL_DBG) { \
        LOG_PRINT("[DBG]"format, ##__VA_ARGS__); \
    }

#define LOG_INFO(format, ...) \
    if(get_log_leval() >= LOG_LEVEL_INFO) { \
        LOG_PRINT("[INFO]"format, ##__VA_ARGS__); \
    }

#define LOG_WARN(format, ...) \
    if(get_log_leval() >= LOG_LEVEL_WARN) { \
        LOG_PRINT("[WARN]"format, ##__VA_ARGS__); \
    }

#define LOG_ERROR(format, ...) \
    if(get_log_leval() >= LOG_LEVEL_ERR) { \
        LOG_PRINT("[ERR]"format, ##__VA_ARGS__); \
    }
#endif
