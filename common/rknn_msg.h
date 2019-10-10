#ifndef __RKNN_MSG_H__
#define __RKNN_MSG_H__

#include "FRLibraryTypes.h"

typedef struct _RKNN_MSG {
    void *out_data0;
    void *out_data1;
    int w;
    int h;
    void *group;
} RKNN_MSG;

int rknn_msg_init();
void rknn_msg_deinit();
int rknn_msg_send(FRIDList* m_msg,int spoof_status);

int rknn_msg_recv(FRIDList* m_msg, int *spoof_status);
#endif
