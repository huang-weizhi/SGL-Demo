/*=============================================================================
 * pc_handle.h - PC communication protocol definitions
 *   Frame layout offsets (PC_HEAD1..PC_HEAD2), commands, events and buffers.
 *===========================================================================*/
#ifndef __PC_HANDLE_H
#define __PC_HANDLE_H

#include <stdbool.h>
#include "CZ01.h"

#define PC_USART 0
#define PC_HANDLE_TIME 5
#define PC_DEVECE_ID 0x00

/* ---- debug-only: standalone flash burning via SSCOM --------------------------
 *  0 = normal protocol (as before)
 *  1 = pc_handle() runs ONLY this flow; the normal protocol is disabled.
 *        start frame : D1 AA AA AA AA FC  -> enter burning
 *        data        : 256 Byte blocks (host paced ~50 ms, no checks)
 *        end frame   : D1 BB BB BB BB FC  -> write remainder, reply D1 CC CC CC CC FC
 *                      after 1 s without new data it is considered finished
 *   For personal debugging only; the shipped product keeps it off. */
#ifndef PC_FLASH_SIMPLE_ENABLE
#define PC_FLASH_SIMPLE_ENABLE 1
#endif

/* ---- debug: UART loopback test (echo received bytes back, no flash write) --
 *  1 = pc_handle() buffers everything after the start frame (D1 AA AA AA AA FC)
 *      and, after 3 s of silence, echoes ALL bytes back -> verifies PC->MCU
 *  0 = use the normal / simple-flash flow instead */
#ifndef PC_LOOPBACK_TEST
#define PC_LOOPBACK_TEST 0
#endif

#define PC_RX_BUFF_SIZE 1036
#define PC_TX_BUFF_SIZE 256
#define PC_CRC_BUFF_SIZE 258

#define PC_CRC_SIZE 4
#define PC_MAX_SEND_SIZE 22
#define PC_MAX_RECV_SIZE 22

#define PC_CHECK_HEAD(A, B) ((A == 0xD1) && (B == 0xF0) ? TRUE : FALSE)

#define SPI_FLASH_UPDATE_START_ADDR 0x00000000
#define UPDATE_PACKET_SIZE 1036
#define UPDATE_DATA_OFFSET 7
#define UPDATE_DATA_LEN 1024

#define PC_HEAD1 0x00
#define PC_CMD1 0x01
#define PC_CMD2 0x02
#define PC_ID_H 0x03
#define PC_ID_L 0x04
#define PC_DATA_LEN_H 0x05
#define PC_DATA_LEN_L 0x06
#define PC_DATA1_LEN_H 0x07
#define PC_DATA1_LEN_L 0x08
#define PC_DATA2_LEN_H 0x09
#define PC_DATA2_LEN_L 0x0A
#define PC_DATA3_LEN_H 0x0B
#define PC_DATA3_LEN_L 0x0C
#define PC_DATA4_LEN_H 0x0D
#define PC_DATA4_LEN_L 0x0E
#define PC_DATA5_LEN_H 0x0F
#define PC_DATA5_LEN_L 0x10
#define PC_CRC32_1 0x11
#define PC_CRC32_2 0x12
#define PC_CRC32_3 0x13
#define PC_CRC32_4 0x14
#define PC_HEAD2 0x15

#define ASK_OK 1
#define ASK_ERROR 0

typedef enum
{
    PC_HEAD_1 = ((uint8_t)0xD1),
    PC_HEAD_2 = ((uint8_t)0xF0),
} pc_head_e;

typedef enum
{
    PC_GENERAL = 0x01,
    PC_FIRMWARE_UPDATE = 0x02,
    PC_UI_UPDATE = 0x03,
} pc_cmd1_e;

typedef enum
{
    PC_GENERAL_ACK = 0x02,
    PC_GENERAL_READ = 0x05,
    PC_GENERAL_WRITE = 0x06,

    PC_FIRMWARE_UPDATE_CONNECT = 0x01,
    PC_FIRMWARE_UPDATE_ACK = 0x02,
    PC_FIRMWARE_UPDATE_START = 0x03,
    PC_FIRMWARE_UPDATE_END = 0x04,

    PC_FIRMWARE_RETURN_OK = 0x1D,
    PC_FIRMWARE_RETURN_ERROR = 0x18,
} pc_cmd2_e;

typedef enum
{
    CONNECT_PC_EVENT,
    IAP_EVENT,
    FLASU_UPDATA_EVENT,
    PC_END_EVENT,
} pc_event_e;

typedef struct PCObject
{
    bool connect_flag;
    uint8_t rx_buff[PC_RX_BUFF_SIZE];
    uint16_t read_size;
    uint8_t tx_buff[PC_TX_BUFF_SIZE];
    uint32_t check_crc_buff[PC_CRC_BUFF_SIZE];
} PC_DATA_t;

extern PC_DATA_t pc_data;
extern pc_event_e pc_event;

void pc_handle(void);

#endif /* __PC_HANDLE_H */