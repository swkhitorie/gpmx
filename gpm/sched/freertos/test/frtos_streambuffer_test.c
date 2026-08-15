#include "frtos_test.h"

#include "driver/mlog.h"

#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"

#define STREAM_BUFFER_SIZE     100
#define SEND_DATA_SIZE         32

StreamBufferHandle_t xStreamBuffer = NULL;

void vStreamSenderTask(void *pvParameters)
{
    uint8_t ucDataToSend[SEND_DATA_SIZE];
    size_t xBytesSent;
    TickType_t xDelay = pdMS_TO_TICKS(1000);

    for (int i = 0; i < SEND_DATA_SIZE; i++) {
        ucDataToSend[i] = i;
    }

    for (;;) {
        xBytesSent = xStreamBufferSend(xStreamBuffer,
                                       ucDataToSend,
                                       SEND_DATA_SIZE,
                                       portMAX_DELAY);
        if (xBytesSent == SEND_DATA_SIZE) {
            KMRAW("StreamSender: Sent %d bytes\n", SEND_DATA_SIZE);
        } else {
            KMRAW("StreamSender: Send failed\n");
        }
        vTaskDelay(xDelay);
    }
}

void vStreamReceiverTask(void *pvParameters)
{
    uint8_t ucReceivedData[SEND_DATA_SIZE*2];
    size_t xBytesReceived;

    for (;;) {
        xBytesReceived = xStreamBufferReceive(xStreamBuffer,
                                              ucReceivedData,
                                              SEND_DATA_SIZE*2,
                                              portMAX_DELAY);
        if (xBytesReceived > 0) {
            KMRAW("StreamReceiver: Received %d bytes, first byte = %d\n",
                        xBytesReceived, ucReceivedData[0]);
        }
    }
}

int freertos_streambuffer_test(int argc, char **argv)
{
    xStreamBuffer = xStreamBufferCreate(STREAM_BUFFER_SIZE, 1);

    xTaskCreate(vStreamSenderTask,   "Sender",   512, NULL, 1, NULL);
    xTaskCreate(vStreamReceiverTask, "Receiver", 512, NULL, 2, NULL);

    return 0;
}
