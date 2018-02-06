#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <limits.h>
#include "log.h"
#include "logrotate.h"

static int log_fd = -1;
static int log_level = LOG_INFO;
static int log_output = FILE_MODE;

static char *get_cur_time(time_t t)
{
	static char time[128];
	struct tm *tmp = NULL;

	tmp = localtime(&t);
	strftime(time, 127, "%Y/%m/%d %T", tmp);
	return time;
}

static const char *get_log_level_str(int level)
{
	static const char *log_level_str[] = {
		"[critical]",
		"[error]   ",
		"[warning] ",
		"[notice] ",
		"[info]    ",
		"[debug]   "
	};

	if (level < LOG_LEVEL_MIN || level > LOG_LEVEL_MAX) {
		return "UNKNOWN LEVEL";
	}
	return log_level_str[level];
}

static ssize_t writen(int fd, const void *vptr, size_t n)
{
	size_t	nleft;
	ssize_t	nwritten;
	const char	*ptr;

	ptr = (char *)vptr;
	nleft = n;
	while (nleft > 0) {
		if ((nwritten = write(fd, ptr, nleft)) <= 0) {
			if (errno == EINTR) {
				nwritten = 0;
			} else {
				return -1;
			}
		}
		nleft -= nwritten;
		ptr += nwritten;
	}
	return n;
}

int open_log(const char* path)
{

	if ((log_fd = open(path, O_RDWR | O_CREAT | O_APPEND, S_IRUSR |S_IWUSR)) < 0) {
		return -1;
	}

	fcntl(log_fd, F_SETFD, FD_CLOEXEC);
	
	return log_fd;
}

void close_log(void)
{
	if (log_fd >= 0) {
		close(log_fd);
		log_fd = -1;
	}
	return;
}

int set_log_level(int new_level)
{
	if (new_level > LOG_LEVEL_MAX || new_level < LOG_LEVEL_MIN) {
		/* invalid input log level */
		fprintf(stderr, "[ERROR]: new log level is invalid\n");
		return -1;
	}
	int old_level = log_level;
	log_level = new_level;
	return old_level;
}

int set_log_output(int new_output)
{
	if (new_output > FILE_MODE || new_output < NULL_MODE) {
		/* invalid input log mode */
		fprintf(stderr, "[ERROR]: new output type is invalid\n");
		return -1;
	}
	int old_output = log_output;
	log_output = new_output;
	return old_output;
}

int write_log_content(int level, const char *fmt, ...)
{
	int     size;
	char    *t = NULL, *ptr = NULL;
	char    tmp[32], buf[MAX_LOG_SIZE];
	va_list ap;
	const char* current_log_level_str;

	if (level > LOG_LEVEL_MAX || level < LOG_LEVEL_MIN) {
		/* invalid input log level */
		fprintf(stderr, "[ERROR]: log level [%d] is invalid, record log failed!\n", level);
		return -1;
	}

	if (log_level < level || log_output == NULL_MODE) {
		return 0;
	}

	ptr = buf;
	size = MAX_LOG_SIZE;
	current_log_level_str = get_log_level_str(level);
	snprintf(ptr, size, "%s ", current_log_level_str);
	ptr += strlen(current_log_level_str) + 1;
	size -= strlen(current_log_level_str) + 1;

	t = get_cur_time(time(NULL));
	snprintf(ptr, size, "%s ", t);
	ptr += strlen(t) + 1;
	size -= strlen(t) + 1;

	snprintf(tmp, 32, "[%d]", getpid());
	snprintf(ptr, size, "%s: ", tmp);
	ptr += strlen(tmp) + 2;
	size -= strlen(tmp) + 2;

	va_start(ap, fmt);
	vsnprintf(ptr, size, fmt, ap);
	va_end(ap);

	if (log_output == FILE_MODE) {
		if (log_fd < 0) {
			fprintf(stderr, "debug has not initialized\n");
			return -1;
		}
		//进行日志转储
		start_logrotate();

		ssize_t buf_size = strlen(buf);
		if (writen(log_fd, buf, strlen(buf)) != buf_size) {
			return -1;
		}
	} else {
		if (level <= LOG_WARNING) {
			fprintf(stdout, "\033[1;31;40m%s\033[0m", buf); //red
		} else if (level == LOG_DEBUG) {
			fprintf(stdout, "\033[1;32;40m%s\033[0m", buf); //greeen
		} else {
			fprintf(stdout, "%s", buf); //normal
		}
	}

	return 0;
}
