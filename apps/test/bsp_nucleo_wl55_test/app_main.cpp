#include "app_main.h"

uart_dev_t *out;
uart_dev_t *in;

enum RUNNING_STATE {
    STATE_WAIT_CONNECTING = 0x00,
    STATE_CONNECTED,
};
static int rd_state = STATE_WAIT_CONNECTING;

int head_check_value = 0x00;

__attribute__((section(".RAM2_region"))) static uint8_t buf_receivers[256] = {0};
__attribute__((section(".RAM2_region"))) static uint8_t buf_senders[1024] = {0};

uint32_t recv_heart_timestamp = 0;
uint32_t send_heart_timestamp = 0;
uint32_t link_fail_check_timestamp = 0;

static char sync_sender[] = "$REQUEST_ALLOW";
static char sync_reciver[] = "$REQUEST_CONNECT";
static char head_sender[] = "$MASTER";
static uint8_t head_sender_len = 7;
static char head_receiver[] = "$SLAVE";
static uint8_t head_receiver_len = 6;
bool process_sender_wait_connecting()
{
    size_t rsz = board_radio_rxbuf_read(&buf_senders[0], 32);
    if (rsz > 0 && !strncmp((const char *)buf_senders, "$REQUEST_CONNECT",16)) {

        RADIO_DEBUG("[%s] recv request\n",
            role_str[board_get_role()]);

        Radio.Send((uint8_t *)sync_sender, strlen(sync_sender));
        HAL_Delay(500);
        rd_state = STATE_CONNECTED;

        RADIO_DEBUG("[%s] start trans %d \n", role_str[board_get_role()], rd_state);

        return true;
    } else {
        
        RADIO_DEBUG("[%s] wait connecting...%d \n", 
            role_str[board_get_role()], rd_state);
    }
    return false;
}

bool process_receiver_wait_connecting()
{
    size_t rsz = board_radio_rxbuf_read(&buf_receivers[0], 32);
    if (rsz > 0 && !strncmp((const char *)buf_receivers, "$REQUEST_ALLOW",14)) {

        RADIO_DEBUG("[%s] connected\n",
            role_str[board_get_role()]);

        rd_state = STATE_CONNECTED;
        return true;
    } else {
        Radio.Send((uint8_t *)sync_reciver, strlen(sync_reciver));
        board_radio_set_send_done_flag(false);

        RADIO_DEBUG("[%s] wait connecting... \n", 
            role_str[board_get_role()]);
    }
    return false;
}

void process_sender_connected()
{
    int rsz_radio = 0;
    int rsz = 0;
    int max_pack_len = board_radio_max_payload_sz();
    int send_len = 0;
    int req_rsz = max_pack_len;
    uint8_t *p_read_buf = &buf_senders[0];
    uint8_t *p_send_buf = &buf_senders[0];
    uint8_t *p_radio_read_buf = &buf_receivers[0];
    bool ck_flag = false;

    if (board_radio_get_send_done_flag() && 
        board_radio_is_ready_to_send() &&
        head_check_value == 0x00) {

        if (HAL_GetTick() - send_heart_timestamp > 3*1000) {
            memcpy(p_send_buf, head_sender, head_sender_len);
            p_read_buf += head_sender_len;
            req_rsz -= head_sender_len;
            ck_flag = true;
        }

        rsz = SERIAL_RDBUF(in, p_read_buf, req_rsz);

        if (rsz > 0) {
            send_len = rsz;
            if (ck_flag) {
                ck_flag = false;
                send_len += head_sender_len;
                RADIO_DEBUG("[%s] ck value = 1 \n", 
                        role_str[board_get_role()]);
                head_check_value = 0x01;
            }
            board_radio_set_send_done_flag(false);
            Radio.Send(p_send_buf, send_len);

            RADIO_SENDER_DISPLAY(p_send_buf, send_len);

            if (send_len >= max_pack_len) {
                HAL_Delay(50);
            }
        }

        return;
    }

    if (board_radio_get_send_done_flag() && 
        board_radio_is_ready_to_send() &&
        head_check_value == 0x01) {
        Radio.Rx(100);
        head_check_value = 0x02;
        RADIO_DEBUG("[%s] ck value = 2 \n", 
                role_str[board_get_role()]);
        link_fail_check_timestamp = HAL_GetTick();
        RADIO_DEBUG("[%s] start to get rcv header \n", 
            role_str[board_get_role()]);

        return;
    }

    rsz_radio = board_radio_rxbuf_read(p_radio_read_buf, max_pack_len);
    if (rsz_radio > 0 && head_check_value == 0x02 &&
        !strncmp((const char *)buf_receivers, "$SLAVE", 6)) {
        head_check_value = 0x00;
        send_heart_timestamp = HAL_GetTick();

        board_radio_delay_between_rx_tx();

        RADIO_DEBUG("[%s] rcv header get \n", 
            role_str[board_get_role()]);

        return;
    }

    if (head_check_value == 0x02 && (HAL_GetTick() - link_fail_check_timestamp) > 3*1000) {
        rd_state = STATE_WAIT_CONNECTING;

        head_check_value = 0x00;
        board_radio_delay_between_rx_tx();
        send_heart_timestamp = HAL_GetTick();
        link_fail_check_timestamp = HAL_GetTick();

        RADIO_DEBUG("[%s] fail get back, restart \n", 
            role_str[board_get_role()]);
    }

    return;
}

int process_receiver_connected()
{
    int rsz_radio = 0;
    int rsz = 0;
    int max_pack_len = board_radio_max_payload_sz();
    uint8_t *p_radio_read_buf = &buf_receivers[0];

    if (head_check_value == 0x00) {
        rsz = board_radio_rxbuf_read(p_radio_read_buf, max_pack_len); 

        if (rsz > 0) {
            if (rsz > head_sender_len && !strncmp((const char *)buf_receivers, "$MASTER", 7)) {
                SERIAL_SEND(out, &p_radio_read_buf[head_sender_len], rsz - head_sender_len);

                RADIO_DEBUG("[%s] rcv start to send back \n", 
                    role_str[board_get_role()]);
                
                head_check_value = 0x01;

                board_radio_set_send_done_flag(false);
                Radio.Send((uint8_t *)&head_receiver[0], head_receiver_len);

            } else {
                SERIAL_SEND(out, p_radio_read_buf, rsz);
            }

            RADIO_DEBUG("[%s] rssi: %d, snr: %d \n", 
                role_str[board_get_role()], board_radio_get_rssi(),
                board_radio_get_lora_snr());
        }
    }

    if (board_radio_get_send_done_flag() && 
        board_radio_is_ready_to_send() &&
        head_check_value == 0x01) {
        Radio.Rx(100);
        head_check_value = 0x00;
        RADIO_DEBUG("[%s] rcv send end \n", 
            role_str[board_get_role()]);
    }

    return rsz;
}

int main(int argc, char *argv[])
{
    board_init();
    board_bsp_init();

    out = board_get_stream_transout();
    in = board_get_stream_transin();

    uint32_t m = HAL_GetTick();
    uint32_t cnt1 = HAL_GetTick();
    uint32_t cnt2 = HAL_GetTick();

    // uint32_t mcuID[3];
    // mcuID[0] = *(volatile uint32_t*)(0x1FFF7590);
    // mcuID[1] = *(volatile uint32_t*)(0x1FFF7594);
    // mcuID[2] = *(volatile uint32_t*)(0x1FFF7598);
    // printf("UID: %x, %x, %x \n", mcuID[0], mcuID[1], mcuID[2]);

    for (;;) {

        switch (rd_state) {


        case STATE_WAIT_CONNECTING:
            if (board_get_role() == RADIO_SENDER) {
                if (HAL_GetTick() - cnt2 >= 500) {
                    cnt2 = HAL_GetTick();
                    process_sender_wait_connecting();
                }
            } else if (board_get_role() == RADIO_RECEIVER) {
                if (HAL_GetTick() - cnt1 >= 500) {
                    cnt1 = HAL_GetTick();
                    process_receiver_wait_connecting();
                }
                if (board_radio_get_send_done_flag() &&
                    board_radio_is_ready_to_send()) {
                    Radio.Rx(100);
                }
            }
            break;


        case STATE_CONNECTED:
            if (board_get_role() == RADIO_SENDER) {
                process_sender_connected();

            } else if (board_get_role() == RADIO_RECEIVER) {
                process_receiver_connected();

            }

            if (HAL_GetTick() - m >= 1) {
                m = HAL_GetTick();
                board_debug();
            }

            break;
        default:
            break;
        }



    }
}
