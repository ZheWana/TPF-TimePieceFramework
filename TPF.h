/**
 * @file TPF.h
 * @author ZheWana(zhewana.cn)
 * @brief Time Piece Framework
 * @date 2022/7/11
  */
#ifndef TPF_H
#define TPF_H

#define TP_MAXTICK (0xFFFFFFFF)

#include "stdint.h"

typedef enum tpStatus {
    TP_OK,
    TP_Error
} tpStatus;

typedef enum taskStatus {
    tpTask_Ready,
    tpTask_Blocked,
    tpTask_Terminated
} taskStatus;

typedef struct tptask_t {
    struct tptask_t *last;
    struct tptask_t *next;

    uint8_t ID;
    uint32_t timeoutTick;

    taskStatus state;

    uint32_t timePiece;

    void (*pHandler)(void);
} tptask_t;

uint32_t TP_GetTick(void);

void TP_TickUpdate(uint32_t period);

void TP_TaskHandler(void);

tpStatus TP_TaskInit(tptask_t *pTask, uint32_t timePiece, void (*taskHandler)(void));

tpStatus TP_TaskDelay(tptask_t *pTask, uint32_t ms);

tpStatus TP_GlobalDelay(uint32_t ms);

tpStatus TP_TaskTerminate(tptask_t *pTask);

tpStatus TP_TaskRework(tptask_t *pTask);

#endif //TPF_H
