#include "frtos_test.h"

#include "driver/mlog.h"

#include "FreeRTOS.h"
#include "task.h"
#include "message_buffer.h"

#define MESSAGE_BUFFER_SIZE     100
#define MESSAGE_SIZE            32

MessageBufferHandle_t xMessageBuffer = NULL;

void vMessageSenderTask(void *pvParameters)
{
    uint8_t ucMessage[MESSAGE_SIZE];
    size_t xBytesSent;
    TickType_t xDelay = pdMS_TO_TICKS(2000);

    for (int i = 0; i < MESSAGE_SIZE; i++) {
        ucMessage[i] = i;
    }

    for (;;) {
        xBytesSent = xMessageBufferSend(xMessageBuffer,
                                        ucMessage,
                                        MESSAGE_SIZE,
                                        portMAX_DELAY);
        if (xBytesSent == MESSAGE_SIZE) {
            KMRAW("MessageSender: Sent message of %d bytes\n", MESSAGE_SIZE);
        } else {
            KMRAW("MessageSender: Send failed\n");
        }
        vTaskDelay(xDelay);
    }
}

void vMessageReceiverTask(void *pvParameters)
{
    uint8_t ucReceivedMessage[MESSAGE_SIZE];
    size_t xBytesReceived;

    for (;;) {
        xBytesReceived = xMessageBufferReceive(xMessageBuffer,
                                               ucReceivedMessage,
                                               MESSAGE_SIZE,
                                               portMAX_DELAY);
        if (xBytesReceived > 0) {
            KMRAW("MessageReceiver: Received message of %d bytes, first byte = %d\n",
                        xBytesReceived, ucReceivedMessage[0]);
        }
    }
}

int freertos_messagebuffer_test(int argc, char **argv)
{
    xMessageBuffer = xMessageBufferCreate(MESSAGE_BUFFER_SIZE);

    xTaskCreate(vMessageSenderTask,   "MsgSender",   512, NULL, 1, NULL);
    xTaskCreate(vMessageReceiverTask, "MsgReceiver", 512, NULL, 2, NULL);

    return 0;
}
