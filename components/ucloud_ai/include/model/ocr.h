/*
 * Copyright (C) 2021-2023 Alibaba Group Holding Limited
 */

#ifndef _OCR_H_
#define _OCR_H_

#include "model/internal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef union _RecogIdentityCardStruct {
    struct {
        char *address;
        char *birthDate;
        char *gender;
        char *nationality;
        char *iDNumber;
        float cardX[4];
        float cardY[4];
        float faceX[4];
        float faceY[4];
        RectStruct location;
    } face;
    struct {
        char *startDate;
        char *issue;
        char *endDate;
    } back;
} RecogIdentityCardStruct;

typedef struct _RecogBankCardStruct {
    char *bankName;
    char *cardNumber;
    char *validDate;
} RecogBankCardStruct;

typedef struct _RecogCharacterStruct {
    char *text;
    float probability;
    int angle;
    int left;
    int top;
    int height;
    int width;
} RecogCharacterStruct;

typedef struct _RecogLicensePlateStruct {
    const char *plateNumber;
    float confidence;
    const char *plateType;
    float plateTypeConfidence;
    struct {
    int x;
    int y;
    int w;
    int h;
    } roi;
} RecogLicensePlateStruct;

typedef struct _RecogOcrCodeStruct {
    char *taskId;
    char imageURL;
    struct {
        char *suggestion;
        float rate;
        char *qrCodesData;
        char *label;
    } results;
} RecogOcrCodeStruct;

typedef struct _OcrResultStruct {
    union {
        RecogIdentityCardStruct identity;
        RecogBankCardStruct bank;
        RecogCharacterStruct character;
        RecogLicensePlateStruct licensePlate;
        RecogOcrCodeStruct qrCode;
    };
} OcrResultStruct;

int recognizeIdentityCardFaceSide(char *url, AIModelCBFunc cb);
int recognizeIdentityCardBackSide(char *url, AIModelCBFunc cb);
int recognizeBankCard(char *url, AIModelCBFunc cb);
int recognizeCharacter(char *url, AIModelCBFunc cb);
int recognizeLicensePlate(char *url, AIModelCBFunc cb);

#ifdef __cplusplus
}
#endif
#endif
