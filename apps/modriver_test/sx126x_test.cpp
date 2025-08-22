#include "app_main.h"
#include "sx126x_driver.h"

uint8_t tx_buffer[300] = {0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA};

void m_sx126x_init()
{
    int ret = 0;

    G_LoRaConfig.HeaderType = LORA_PACKET_EXPLICIT;
    G_LoRaConfig.CrcMode = LORA_CRC_ON;
    G_LoRaConfig.InvertIQ = LORA_IQ_NORMAL;

    G_LoRaConfig.LoRa_Freq = 470800000;
    G_LoRaConfig.BandWidth = LORA_BW_500;
    G_LoRaConfig.SpreadingFactor = LORA_SF6;
    G_LoRaConfig.CodingRate = LORA_CR_4_5;
    G_LoRaConfig.PowerCfig = 22;
    G_LoRaConfig.PreambleLength = 16;
    G_LoRaConfig.PayloadLength = 200;

    sx126x_hal_init();
    ret = sx126x_lora_init();
    if (ret != NORMAL) {
        while (1) {
            printf("sx126x lora init failed %d\r\n", ret);
        }
    }
    printf("sx126x lora init success, start rx\r\n");

    sx126x_start_rx(0xffffff);
}

void m_sx126x_run()
{
    sx126x_tx_packet(tx_buffer);
}

void sx126x_tx_done_callback()
{
    printf("on tx done \r\n");
}

void sx126x_rx_done_callback(uint8_t *payload, uint16_t size, 
    int16_t lastpack_rssi, int16_t aver_rssi, int16_t snr)
{
    printf("rx done: lst_rssi:%d, aver_rssi:%d, snr:%d\r\n",
        lastpack_rssi, aver_rssi, snr);

    printf("rx payload len: %d\r\n", size);
    for (int i = 0; i < size; i++) {
        printf("%x ", payload[i]);
    }
    printf("\r\n");
    // sx126x_start_rx(0x2fffff);
}
