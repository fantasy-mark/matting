#pragma once

extern int g_thread_run;

extern unsigned int y_thresh;
extern unsigned int u_thresh;
extern unsigned int v_thresh;
extern unsigned int i_thresh;
extern unsigned int o_thresh;

void setInfo(int ts);

void *mattingProcessThread(void *argv);

