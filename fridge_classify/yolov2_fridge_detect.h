/*
 * Copyright (c) 2022 HiSilicon (Shanghai) Technologies CO., LIMITED.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef YOLOV2_FRIDGE_DETECT_H
#define YOLOV2_FRIDGE_DETECT_H

#include "hi_type.h"

#if __cplusplus
extern "C" {
#endif

HI_S32 FridgeDetectInit();
HI_S32 FridgeDetectExit();
HI_S32 FridgeDetectCal(IVE_IMAGE_S *srcYuv, DetectObjInfo resArr[]);

#ifdef __cplusplus
}
#endif
#endif
