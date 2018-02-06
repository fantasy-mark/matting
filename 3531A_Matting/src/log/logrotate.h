#ifndef _LOGROTATE_H_
#define _LOGROTATE_H_

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

int set_logrotate(const char *path, const char *dir, int rotate, int minsize);
int start_logrotate();

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif
