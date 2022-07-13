/**
 * @file TPF.c
 * @author ZheWana(zhewana.cn)
 * @brief Time Piece Framework
 * @date 2022/7/11
  */
#include "TPF.h"
#include "stdlib.h"

static uint32_t TPtick;
static uint8_t TaskNum = 0;
static tptask_t *head, *tail;

__attribute__((weak))void TP_ErrorHandler(tptask_t *pTask) {
    while (1) {
        //If your code reaches here,please check
        //whether you have initialized the task!
    };
}

uint32_t TP_GetTick(void) {
    return TPtick;
}

void TP_TickUpdate(uint32_t period) {
    TPtick += period;
}

tpStatus TP_TaskInit(tptask_t *pTask, uint32_t timePiece, void (*taskHandler)(void)) {

    if (pTask == NULL) {
        return TP_Error;
    } else {
        //链表维护
        if (head == NULL) {
            pTask->last = NULL;
            head = pTask;
            tail = pTask;
        } else {
            tail->next = pTask;
            pTask->last = tail;
            tail = pTask;
        }

        //结构体赋值
        pTask->ID = TaskNum++;
        pTask->state = tpTask_Ready;
        pTask->pHandler = taskHandler;
        pTask->timePiece = timePiece;
        pTask->timeoutTick = pTask->timePiece;
    }

    return TP_OK;
}

void TP_TaskHandler(void) {
    for (tptask_t *pTask = head; pTask != NULL; pTask = pTask->next) {
        switch (pTask->state) {
            case tpTask_Ready:
                if ((TPtick - pTask->timeoutTick) <= TP_MAXTICK / 2) {
                    pTask->timeoutTick = TPtick + pTask->timePiece;
                    if (pTask->pHandler != NULL) {
                        pTask->pHandler();
                    } else {
                        TP_ErrorHandler(pTask);
                    }
                }
                break;
            case tpTask_Blocked:
                if ((TPtick - pTask->timeoutTick) <= TP_MAXTICK / 2) {
                    pTask->timeoutTick = TPtick + pTask->timePiece;
                    pTask->state = tpTask_Ready;
                }
                break;
            case tpTask_Terminated:
                break;
            default:
                TP_ErrorHandler(pTask);
        }
    }
}

tpStatus TP_TaskDelay(tptask_t *pTask, uint32_t ms) {
    if (pTask->state == tpTask_Terminated) {
        return TP_Error;
    } else {
        pTask->state = tpTask_Blocked;
        pTask->timeoutTick = TPtick + ms;
    }
    return TP_OK;
}

tpStatus TP_GlobalDelay(uint32_t ms) {
    for (tptask_t *pTask = head; pTask != NULL; pTask = pTask->next) {
        if (pTask->state == tpTask_Terminated) {
            return TP_Error;
        } else {
            pTask->state = tpTask_Blocked;
            pTask->timeoutTick = TPtick + ms;
        }
    }
    return TP_OK;
}

tpStatus TP_TaskTerminate(tptask_t *pTask) {
    pTask->state = tpTask_Terminated;
    pTask->timeoutTick = 0;
    return TP_OK;
}

tpStatus TP_TaskRework(tptask_t *pTask) {
    pTask->state = tpTask_Ready;
    pTask->timeoutTick = TPtick + pTask->timePiece;
    return TP_OK;
}