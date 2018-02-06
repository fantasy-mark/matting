#ifndef __LOG_H__
#define __LOG_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#define MAX_LOG_SIZE 4096

/**
 * log recorder mode.
 */
enum log_mode_e {
        NULL_MODE = 0, /* do not record log */
        TERMINAL_MODE = 1, /* print to stdout */
        FILE_MODE = 2,  /* record to file */
};

enum log_level_e {
	LOG_CRIT = 0,
	LOG_ERR = 1,
	LOG_WARNING = 2,
	LOG_NOTICE = 3,
	LOG_INFO = 4,
	LOG_DEBUG = 5
};

#define LOG_LEVEL_MIN LOG_CRIT
#define LOG_LEVEL_MAX LOG_DEBUG

int open_log(const char* path);
void close_log(void);
int set_log_level(int new_level);
int set_log_output(int new_output);
int write_log_content(int level, const char *fmt, ...);


#define LOG_MESSAGE(level, fmt, args...) write_log_content(level, "[%s:%s:%d]---" fmt"\n", __FILE__, __FUNCTION__, __LINE__, ##args)
#define TRACE_PRT(fmt, args...) write_log_content(LOG_DEBUG, "[%s:%s:%d]---" fmt"\n", __FILE__, __FUNCTION__, __LINE__, ##args)
#define SAMPLE_PRT(fmt, args...) write_log_content(LOG_DEBUG, "[%s:%s:%d]---" fmt"\n", __FILE__, __FUNCTION__, __LINE__, ##args)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif
