#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "sample_comm_nnie.h"
#include "sample_media_ai.h"
#include "ai_infer_process.h"
#include "yolov2_fridge_detect.h"
#include "vgs_img.h"
#include "ive_img.h"
#include "misc_util.h"
#include "hisignalling.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define HAND_FRM_WIDTH     640
#define HAND_FRM_HEIGHT    384
#define DETECT_OBJ_MAX     32
#define RET_NUM_MAX        4
#define DRAW_RETC_THICK    2     //2    // Draw the width of the line
#define WIDTH_LIMIT        32
#define HEIGHT_LIMIT       32
#define IMAGE_WIDTH        224  // The resolution of the model IMAGE sent to the classification is 224*224
#define IMAGE_HEIGHT       224
#define MODEL_FILE_GESTURE    "/userdata/models/fridge_classify/fridge_gesture.wk" // darknet framework wk model

static int biggestBoxIndex;
static IVE_IMAGE_S img;
static DetectObjInfo objs[DETECT_OBJ_MAX] = {0};
static RectBox boxs[DETECT_OBJ_MAX] = {0};
static RectBox objBoxs[DETECT_OBJ_MAX] = {0};
static RectBox remainingBoxs[DETECT_OBJ_MAX] = {0};
static RectBox cnnBoxs[DETECT_OBJ_MAX] = {0}; // Store the results of the classification network
static RecogNumInfo numInfo[RET_NUM_MAX] = {0};
static IVE_IMAGE_S imgIn;
static IVE_IMAGE_S imgDst;
static VIDEO_FRAME_INFO_S frmIn;
static VIDEO_FRAME_INFO_S frmDst;
int uartFd = 0;

/*
 * 加载检测模型
 * Load fridge detect and classify model
 */
HI_S32 Yolo2FridgeDetectResnetClassifyLoad(uintptr_t* model)
{
    SAMPLE_SVP_NNIE_CFG_S *self = NULL;
    HI_S32 ret;
    *model = 1; //ztong 
    FridgeDetectInit(); // Initialize the fridge detection model
    SAMPLE_PRT("Load fridge detect claasify model success\n");
    /*
     * Uart串口初始化
     * Uart open init
     */
    uartFd = UartOpenInit();
    if (uartFd < 0) {
        printf("uart1 open failed\r\n");
    } else {
        printf("uart1 open successed\r\n");
    }
    return ret;
}

/*
 * 卸载检测模型
 * Unload fridge detect and classify model
 */
HI_S32 Yolo2FridgeDetectResnetClassifyUnload(uintptr_t model)
{
    FridgeDetectExit(); // Uninitialize the fridge detection model
    close(uartFd);
    SAMPLE_PRT("Unload fridge detect claasify model success\n");

    return 0;
}

/*
 * 获得最大的框
 * Get the maximum fridge
 */
static HI_S32 GetBiggestFridgeIndex(RectBox boxs[], int detectNum)
{
    HI_S32 fridgeIndex = 0;
    HI_S32 biggestBoxIndex = fridgeIndex;
    HI_S32 biggestBoxWidth = boxs[fridgeIndex].xmax - boxs[fridgeIndex].xmin + 1;
    HI_S32 biggestBoxHeight = boxs[fridgeIndex].ymax - boxs[fridgeIndex].ymin + 1;
    HI_S32 biggestBoxArea = biggestBoxWidth * biggestBoxHeight;

    for (fridgeIndex = 1; fridgeIndex < detectNum; fridgeIndex++) {
        HI_S32 boxWidth = boxs[fridgeIndex].xmax - boxs[fridgeIndex].xmin + 1;
        HI_S32 boxHeight = boxs[fridgeIndex].ymax - boxs[fridgeIndex].ymin + 1;
        HI_S32 boxArea = boxWidth * boxHeight;
        if (biggestBoxArea < boxArea) {
            biggestBoxArea = boxArea;
            biggestBoxIndex = fridgeIndex;
        }
        biggestBoxWidth = boxs[biggestBoxIndex].xmax - boxs[biggestBoxIndex].xmin + 1;
        biggestBoxHeight = boxs[biggestBoxIndex].ymax - boxs[biggestBoxIndex].ymin + 1;
    }

    if ((biggestBoxWidth == 1) || (biggestBoxHeight == 1) || (detectNum == 0)) {
        biggestBoxIndex = -1;
    }

    return biggestBoxIndex;
}

//zt start 0703
int flag_send = 0;
void send_fridge_result(DetectObjInfo *objs,HI_S32 objnum)
{
    unsigned char apple[4] ={0,'a',0,1};
    unsigned char banana[4] ={0,'b',0,1};
    unsigned char eggplant[4] ={0,'e',0,1};
    unsigned char end_send[4] ={0,0,0,'b'};

    HI_CHAR *foodname = NULL;
    for(HI_S32 i = 0;i<objnum;i++)
    {
        // printf("wait key for uart send\n");
        // (void)getchar();

        if(objs[i].score > 0.2) //这里可以设置置信度高于一定值的结果才发送
        {
            printf("send food class is %d\n",objs[i].cls);
            printf("send food class is %d\n",objs[i].cls);
            printf("send food class is %d\n",objs[i].cls);
            printf("send food class is %d\n",objs[i].cls);

            switch (objs[i].cls)
            {
            case 1u:
                /* code */
                sleep(5);
                foodname = "apple";
                HisignallingMsgSend(uartFd, apple,sizeof(apple)/sizeof(apple[0])); // 大拇指 + 小拇指
                SAMPLE_PRT("----food name----:%s\n", foodname);
                break;
            case 2u:
                /* code */
                sleep(5);
                foodname = "banana";
                HisignallingMsgSend(uartFd, banana,sizeof(banana)/sizeof(banana[0])); // 大拇指 + 小拇指
                SAMPLE_PRT("----food name----:%s\n", foodname);
                break;
            case 3u:
                /* code */
                sleep(5);
                foodname = "eggplant";
                HisignallingMsgSend(uartFd, eggplant,sizeof(eggplant)/sizeof(eggplant[0])); // 大拇指 + 小拇指
                SAMPLE_PRT("----food name----:%s\n", foodname);
                break;
            default:
                break;
            }
        }
    }
    sleep(5);
    HisignallingMsgSend(uartFd, end_send,sizeof(end_send)/sizeof(end_send[0])); // 大拇指 + 小拇指
    SAMPLE_PRT("----end send----");
}

//zt end 0703
/*
 * 冰箱物品检测推理
 * Fridge detect and classify calculation
 */
HI_S32 Yolo2FridgeDetectResnetClassifyCal(uintptr_t model, VIDEO_FRAME_INFO_S *srcFrm, VIDEO_FRAME_INFO_S *dstFrm)
{
    // 串口接收
    char recbuf[3] = {0};
    printf("start receive uart\r\n");
    HisignallingMsgReceive(uartFd,&recbuf,3);
    int count_rec = 0;
    while(recbuf[0] == 0)
    {
        count_rec++;
        printf("receive uart in while,count = %d\r\n",count_rec);
        HisignallingMsgReceive(uartFd,&recbuf,3);
    }

    SAMPLE_SVP_NNIE_CFG_S *self = (SAMPLE_SVP_NNIE_CFG_S*)model;
    HI_S32 resLen = 0;
    int objNum;
    int ret;
    int num = 0;

    ret = FrmToOrigImg((VIDEO_FRAME_INFO_S*)srcFrm, &img);
    SAMPLE_CHECK_EXPR_RET(ret != HI_SUCCESS, ret, "fridge detect for YUV Frm to Img FAIL, ret=%#x\n", ret);

    // printf("wait key for detect\n");//ztong
    // (void)getchar();
    objNum = FridgeDetectCal(&img, objs); // Send IMG to the detection net for reasoning
    for (int i = 0; i < objNum; i++) {
        cnnBoxs[i] = objs[i].box;
        RectBox *box = &objs[i].box;
        RectBoxTran(box, HAND_FRM_WIDTH, HAND_FRM_HEIGHT,
            dstFrm->stVFrame.u32Width, dstFrm->stVFrame.u32Height);
        SAMPLE_PRT("yolo2_out: {%d, %d, %d, %d}\n", box->xmin, box->ymin, box->xmax, box->ymax);
        boxs[i] = *box;
    }
    biggestBoxIndex = GetBiggestFridgeIndex(boxs, objNum);
    SAMPLE_PRT("biggestBoxIndex:%d, objNum:%d\n", biggestBoxIndex, objNum);

    /*
     * 当检测到对象时，在DSTFRM中绘制一个矩形
     * When an object is detected, a rectangle is drawn in the DSTFRM
     */
    if (biggestBoxIndex >= 0) {
        objBoxs[0] = boxs[biggestBoxIndex];
        MppFrmDrawRects(dstFrm, objBoxs, 1, RGB888_GREEN, DRAW_RETC_THICK); // Target fridge objnum is equal to 1
        //画框
        for (int j = 0; (j < objNum) && (objNum > 1); j++) {
            if (j != biggestBoxIndex) {
                remainingBoxs[num++] = boxs[j];
                MppFrmDrawRects(dstFrm, remainingBoxs, objNum - 1, RGB888_GREEN, DRAW_RETC_THICK);
            }
        }
    }
    if (objNum>0)
    {
        send_fridge_result(objs,objNum); //ztong
    }
    recbuf[0] = 0;
    return ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
