#ifndef HAND_CLASSIFY_H
#define HAND_CLASSIFY_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "hi_comm_video.h"

#if __cplusplus
extern "C" {
#endif

/*
 * 加载手部检测和手势分类模型
 * Load fridge detect and classify model
 */
HI_S32 Yolo2FridgeDetectResnetClassifyLoad(uintptr_t* model);

/*
 * 卸载手部检测和手势分类模型
 * Unload fridge detect and classify model
 */
HI_S32 Yolo2FridgeDetectResnetClassifyUnload(uintptr_t model);

/*
 * 手部检测和手势分类推理
 * Fridge detect and classify calculation
 */
HI_S32 Yolo2FridgeDetectResnetClassifyCal(uintptr_t model, VIDEO_FRAME_INFO_S *srcFrm, VIDEO_FRAME_INFO_S *dstFrm);

#ifdef __cplusplus
}
#endif
#endif
