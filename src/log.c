#include "log.h"

static int _log_level = LOG_LEVEL_INFO;

void set_log_level(int lv)
{
    _log_level = lv;
}

int get_log_leval()
{
    return _log_level;
}
