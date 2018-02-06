#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>

#define FILE_BUFSIZE 256
#define COPY_BUFSIZE 4096

static char log_dir[FILE_BUFSIZE] = {0};
static char log_filename[FILE_BUFSIZE] = {0};
static char log_filepath[FILE_BUFSIZE] = {0};
static int logrotate_rotate = 5;
static int logrotate_minsize = 10;
static pthread_mutex_t logrotate_lock = PTHREAD_MUTEX_INITIALIZER;

static int get_file_number();
static ssize_t get_logfile_size(const char *file_path);
static int copy_file(const char *old_file, const char *new_file);

int set_logrotate(const char *path, const char *dir, int rotate, int minsize)
{
	if (NULL == path || NULL == dir || rotate < 0 || minsize < 0) {
		return -1;
	} else {
		memset(log_filepath, 0x0, sizeof(log_filepath));
		strncpy(log_filepath, path, sizeof(log_filepath));
	}

	memset(log_dir, 0x0, sizeof(log_dir));
	strncpy(log_dir, dir, sizeof(log_dir));

	logrotate_rotate = rotate;
	logrotate_minsize = minsize;

	return 0;
}

int start_logrotate()
{
	int file_num;
	char old_file_name[FILE_BUFSIZE] = {0};
	char new_file_name[FILE_BUFSIZE] = {0};
	int i = 1;

	if (get_logfile_size(log_filepath) < logrotate_minsize * 1024 * 1024) {
		return 0;
	} else {
		if (pthread_mutex_trylock(&logrotate_lock)) {
			return 0;
		}
		file_num = get_file_number();
		if (file_num <= logrotate_rotate) {
			sprintf(new_file_name, "%s.%d", log_filepath, file_num);
			if (!copy_file(log_filepath, new_file_name)) {
				truncate(log_filepath, 0);
			}
		}
		else if (file_num > logrotate_rotate) {
			sprintf(old_file_name, "%s.%d", log_filepath, 1);
			unlink(old_file_name);
			for(; i < logrotate_rotate; i++) {
				memset(old_file_name, 0x0, FILE_BUFSIZE);
				memset(new_file_name, 0x0, FILE_BUFSIZE);
				sprintf(old_file_name, "%s.%d", log_filepath, i+1);
				sprintf(new_file_name, "%s.%d", log_filepath, i);
				rename(old_file_name, new_file_name);
			}

			if (!copy_file(log_filepath, old_file_name)) {
				truncate(log_filepath, 0);
			}
		}
		pthread_mutex_unlock(&logrotate_lock);
		return 1;
	}
	return 0;
}

static ssize_t get_logfile_size(const char *file_path)
{
	struct stat statbuf;
	if (stat(file_path, &statbuf) < 0) {
		return -1;
	}
	return (statbuf.st_size);
}

static int get_file_number()
{
	int i = 0;
	char *tmp;
	char buf[FILE_BUFSIZE] = {0};
	char *pbuf[10] = {0};
	int file_num = 0;
	struct dirent *dirp;
	DIR *dp;

	if (0 == strlen(log_filepath)) {
		return -1;
	}

	strncpy(buf, log_filepath, FILE_BUFSIZE);
	tmp = strtok(buf, "/");
	while (tmp != NULL) {
		pbuf[i++] = tmp;
		tmp = strtok(NULL, "/");
	}
	strncpy(log_filename, pbuf[i-1], FILE_BUFSIZE);

	if ((dp = opendir(log_dir)) == NULL) {
		return -1;
	}
	while((dirp = readdir(dp)) != NULL) {
		if (strncmp(log_filename, dirp->d_name, strlen(log_filename)) == 0) {
			file_num ++;
		}
	}
	return file_num;
}

static int copy_file(const char *old_file, const char *new_file)
{
	int from_fd, to_fd;
	ssize_t bytes_read = 0, bytes_write = 0;
	char buf[COPY_BUFSIZE] = {0};
	int err = 0;

	if (old_file == NULL || new_file == NULL || strlen(old_file) == 0 || strlen(new_file) == 0) {
		return -1;
	}
	
	from_fd = open(old_file, O_RDONLY);
	if (from_fd < 0) {
		return -1;
	}

	to_fd = open(new_file, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);
	if (to_fd < 0) {
		close(from_fd);
		return -1;
	}

	while((bytes_read = read(from_fd, buf, sizeof(buf))) > 0) {
		bytes_write = write(to_fd, buf, bytes_read);
		if (bytes_write != bytes_read) {
			err = -1;
			break;
		}
	}
	if (bytes_read < 0) {
		err = -1;
	}
	
	close(from_fd);
	close(to_fd);
	return err;
}

