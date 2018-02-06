#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include <QApplication>
#include <QWSServer>
#include <QColor>
#include <QBrush>
#include "ctrl.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include "matting.h"
#include "mpp.h"
#include "logrotate.h"
#include "gv7704.h"
#include "gv7704_base.h"

#ifdef __cplusplus
}
#endif

int g_thread_run = 1;

QCtrl *pDlg = NULL;

extern "C" void setInfo(int ts)
{
    if(NULL != pDlg)
        pDlg->setInfo(ts);
}

static void sig_capture(int sig)
{
    switch(sig) {
        case SIGINT:
        case SIGTERM:
            g_thread_run = 0;
            usleep(100000);
            HI_MPI_SYS_Exit();
            HI_MPI_VB_Exit();
            TRACE_PRT("sig_capture signal = %d\n", sig);
            exit(0);
            break;
    }
}

int main(int argc, char *argv[])
{
    signal(SIGINT,sig_capture);
    signal(SIGTERM,sig_capture);
    signal(SIGPIPE,sig_capture);

    open_log(LOG_FILE_PATH);
    set_log_level(LOG_LEVEL);
    set_log_output(LOG_OUTPUT);
    set_logrotate(LOG_FILE_PATH, LOG_DIR_PATH, LOG_ROTATE, LOG_MINSIZE);
    LOG_MESSAGE(LOG_INFO, "*********************** BEGIN ***********************");

    LOG_MESSAGE(LOG_INFO, "Build time is %s %s", __DATE__, __TIME__);

    VI_DEV viDev = 0;
    SIZE_S viSize;
    viSize.u32Width = 1920;
    viSize.u32Height = 1080;

    sdi_info_s sdi_info;
    memset(&sdi_info, 0, sizeof(sdi_info));

    SYS_init();
    VO_Start();
    MPI_VPSS_Start(12, 25, 25);
    MPI_VO_BindVpss(0, 0, 12, 0);

    MPI_HIFB_Init(0, 1920, 1080);
#if 0
    // VI SDI0
    gv7704_read(viDev, &sdi_info);
    MPI_VPSS_Start(viDev, sdi_info.frame_rate, 25);
    VI_Start(viDev, viDev<<2, &viSize, sdi_info.frame_rate, VI_SCAN_PROGRESSIVE);
    MPI_VI_BindVpss(viDev<<2, viDev);
#endif
    // VI VGA
    viDev = 6;
    MPI_VPSS_Start(viDev, 60, 25);
    VI_Start(viDev, viDev<<2, &viSize, 60, VI_SCAN_PROGRESSIVE);
    MPI_VI_BindVpss(viDev<<2, viDev);

    pthread_t t_id;
#if 0
    if(pthread_create(&t_id, NULL, mattingProcessThread, NULL) != 0) {
        LOG_MESSAGE(LOG_ERR, "pthread_create mattingProcessThread failed!");
    }
#endif
#if 1
    QWSServer::setBackground(QColor(0,0,0,0));
    QApplication a(argc, argv);
    QCtrl dlg;
    pDlg = &dlg;
    dlg.show();
    a.exec();
#else
    char c;
    do{
        c=getchar();
    }while(c!='q');

#endif
    g_thread_run = 0;
    usleep(100000);
    SYS_Exit();
    LOG_MESSAGE(LOG_INFO, "*********************** E N D ***********************");
    return 0;
}
