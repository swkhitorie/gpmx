#include "list_queue.h"

#include "FreeRTOS.h"
#include "queue.h"

int list_queue(int argc, char **argv)
{
    int size;
    QueueRegistryItem_t *plist;
    QueueRegistryItem_t *item;
    QueueHandle_t pxQueue;

    size = uxQueueRegistyListGet(plist);

    for (int i = 0; i < configQUEUE_REGISTRY_SIZE; i++) {

        item = &plist[i];

        if (item->xHandle == NULL) {
            continue;
        }

        pxQueue = item->xHandle;

        int current_msg_wait = pxQueue->uxMessagesWaiting;
        int max_msg_wait = pxQueue->uxMaxMessagesWaiting;
        int queue_capacity = pxQueue->uxLength;
        int queue_item_size = pxQueue->uxItemSize;

        // int len = snprintf(cPrintBuffer, sizeof(cPrintBuffer),
        //     "[main][queue] %-25s,ptr:%p, "
        //     "curMsgs:%03u, maxUsedMsgs:%03u, capacity:%03u, item_size:%03u",
        //     pxRegistryItem->pcQueueName,
        //     (void*)pxQueue,
        //     current_msg_wait,
        //     max_msg_wait,
        //     queue_capacity,
        //     queue_item_size,
        // );
    }

    return 0;
}
