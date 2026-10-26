/*=============================================================================
 * pc_handle.c - PC communication protocol handler
 *
 * Implements the UART frame protocol between the device and the PC tool:
 *   frame: 0xD1 0xF0 | cmd1 | cmd2 | id(2B) | data-len(2B) ... | CRC32
 * cmd1 = GENERAL (register read/write) or FIRMWARE_UPDATE.
 * Firmware update flow: CONNECT -> ACK -> START -> receive 1KB packets that
 * are written to the external SPI flash -> END, then reboot into IAP which
 * copies the image from SPI flash to internal flash.
 * pc_comm_handle() is the periodic entry point (PC_HANDLE_TIME ms).
 *===========================================================================*/
#include <stdbool.h>
#include "pc_handle.h"

PC_DATA_t pc_data;
pc_event_e pc_event = PC_END_EVENT;

#if !PC_FLASH_SIMPLE_ENABLE
static uint32_t current_flash_addr = SPI_FLASH_UPDATE_START_ADDR;

static void ask_data_cmd_to_pc(PC_DATA_t *pc, uint8_t state)
{
    uint32_t crc_value = 0;

    if (state)
    {
        pc->tx_buff[2] = 0x02;
        pc->tx_buff[8] = 0x1D;
    }
    else
    {
        pc->tx_buff[2] = 0x03;
        pc->tx_buff[8] = 0x18;
    }

    pc->tx_buff[0] = PC_HEAD_1;
    pc->tx_buff[1] = 0x03;
    pc->tx_buff[3] = LOCAL_DEVICE_ID_2;
    pc->tx_buff[4] = LOCAL_DEVICE_ID_1;
    pc->tx_buff[5] = 0x00;
    pc->tx_buff[6] = 0x0A;
    pc->tx_buff[7] = 0x1D;
    pc->tx_buff[9] = 0x00;
    pc->tx_buff[10] = 0x00;
    pc->tx_buff[11] = 0x00;
    pc->tx_buff[12] = 0x00;
    pc->tx_buff[13] = 0x00;
    pc->tx_buff[14] = 0x00;
    pc->tx_buff[15] = 0x00;
    pc->tx_buff[16] = 0x00;
    memset(pc->check_crc_buff, 0, PC_CRC_BUFF_SIZE);
    convert_data(pc->tx_buff, pc->check_crc_buff, 1, 16);
    crc_value = crc_block_calculate(pc->check_crc_buff, 4);
    crc_data_reset();
    pc->tx_buff[17] = (crc_value >> 24) & 0xff;
    pc->tx_buff[18] = (crc_value >> 16) & 0xff;
    pc->tx_buff[19] = (crc_value >> 8) & 0xff;
    pc->tx_buff[20] = crc_value & 0xff;
    pc->tx_buff[21] = PC_HEAD_2;
    uart_send(uart1,pc->tx_buff, PC_MAX_SEND_SIZE);
}

static void ask_connect_cmd_to_pc(PC_DATA_t *pc, const CZ01_t *this)
{
    uint32_t crc_value = 0;
    pc->tx_buff[PC_HEAD1] = PC_HEAD_1;
    pc->tx_buff[PC_CMD1] = 0x01;
    pc->tx_buff[PC_CMD2] = 0x01;
    pc->tx_buff[PC_ID_H] = LOCAL_DEVICE_ID_2;
    pc->tx_buff[PC_ID_L] = LOCAL_DEVICE_ID_1;
    pc->tx_buff[PC_DATA_LEN_H] = 0x00;
    pc->tx_buff[PC_DATA_LEN_L] = 0x0A;
    pc->tx_buff[PC_DATA1_LEN_H] = this->sw_ver_major;
    pc->tx_buff[PC_DATA1_LEN_L] = this->sw_ver_minor;
    pc->tx_buff[PC_DATA2_LEN_H] = this->sw_ver_patch;
    pc->tx_buff[PC_DATA2_LEN_L] = this->hw_ver_major;
    pc->tx_buff[PC_DATA3_LEN_H] = this->hw_ver_minor;
    pc->tx_buff[PC_DATA3_LEN_L] = this->hw_ver_patch;
    pc->tx_buff[PC_DATA4_LEN_H] = ((this->uid_value >> 24) & 0xff);
    pc->tx_buff[PC_DATA4_LEN_L] = ((this->uid_value >> 16) & 0xff);
    pc->tx_buff[PC_DATA5_LEN_H] = ((this->uid_value >> 8) & 0xff);
    pc->tx_buff[PC_DATA5_LEN_L] = (this->uid_value & 0xff);
    memset(pc_data.check_crc_buff, 0, PC_CRC_SIZE);
    convert_data(pc_data.tx_buff, pc_data.check_crc_buff, PC_CMD1, PC_DATA5_LEN_L);
    crc_value = crc_block_calculate(pc_data.check_crc_buff, PC_CRC_SIZE);
    crc_data_reset();
    pc->tx_buff[PC_CRC32_1] = ((crc_value >> 24) & 0xff);
    pc->tx_buff[PC_CRC32_2] = ((crc_value >> 16) & 0xff);
    pc->tx_buff[PC_CRC32_3] = ((crc_value >> 8) & 0xff);
    pc->tx_buff[PC_CRC32_4] = (crc_value & 0xff);
    pc->tx_buff[PC_HEAD2] = PC_HEAD_2;
    /* send data */
    uart_send(uart1,pc->tx_buff, PC_MAX_SEND_SIZE);
}

static void get_event_from_pc(CZ01_t *this, PC_DATA_t *pc)
{
    uint32_t crc_value;

    if (this->flash_updata)
    {
        return;
    }

    if (uart_rx_available(uart1))
    {
        pc->read_size = uart_read(uart1,pc->rx_buff, PC_MAX_RECV_SIZE);

        if (pc->read_size == PC_MAX_RECV_SIZE)
        {
            if (PC_CHECK_HEAD(pc->rx_buff[PC_HEAD1], pc->rx_buff[PC_HEAD2]))
            {
                /* crc check */
                convert_data(pc->rx_buff, pc->check_crc_buff, PC_CMD1, PC_DATA5_LEN_L);
                crc_value = crc_block_calculate(pc->check_crc_buff, PC_CRC_SIZE);
                crc_data_reset();

                if (pc->rx_buff[PC_CRC32_1] == ((crc_value >> 24) & 0xff) &&
                        pc->rx_buff[PC_CRC32_2] == ((crc_value >> 16) & 0xff) &&
                        pc->rx_buff[PC_CRC32_3] == ((crc_value >> 8) & 0xff) &&
                        pc->rx_buff[PC_CRC32_4] == (crc_value & 0xff))
                {
                    /* connect PC */
                    if (pc->rx_buff[PC_CMD1] == 0x01 && pc->rx_buff[PC_CMD2] == 0x01 && pc->rx_buff[PC_ID_L] == 0x00)
                    {
                        pc_event = CONNECT_PC_EVENT;
                    }
                    /* jump to bootloader */
                    else if (pc->rx_buff[PC_CMD1] == PC_FIRMWARE_UPDATE && pc->rx_buff[PC_CMD2] == PC_FIRMWARE_UPDATE_CONNECT
                             && pc->rx_buff[PC_ID_L] == 0x00)
                    {
                        pc_event = IAP_EVENT;
                    }
                    else if (pc->rx_buff[PC_CMD1] == PC_UI_UPDATE && pc->rx_buff[PC_CMD2] == PC_FIRMWARE_UPDATE_CONNECT
                             && pc->rx_buff[PC_ID_L] == 0x00)
                    {
                        pc_event = FLASU_UPDATA_EVENT;
                    }
                    /* other cmd */
                    else
                    {
                        __NOP();
                    }
                }
            }

            pc->read_size = 0;
        }
    }
}

#if 0
static void send_data_to_pc(CZ01_t *this)
{
}

void send_heartbeat_to_pc(CZ01_t *this, PC_DATA_t *pc)
{
}


#endif

static void get_flash_data_from_pc(CZ01_t *this, PC_DATA_t *pc)
{
    /* bytes received so far for the current packet, kept across calls */
    static uint16_t got = 0;

    if (!this->flash_updata)
    {
        got = 0;
        return;
    }

    /* accumulate bytes until a full packet is gathered. while not synced
     * (got==0) only the header 0xD1 can start a packet, so a single lost
     * byte merely drops one garbage byte and we re-align on the next
     * packet instead of drifting forever. */
    while (uart_rx_available(uart1) && got < UPDATE_PACKET_SIZE)
    {
        uint8_t b = uart_read_byte(uart1);

        if (got == 0)
        {
            if (b != PC_HEAD_1)
            {
                continue;   /* not synced: skip everything except 0xD1 */
            }
        }
        pc->rx_buff[got] = b;
        got++;
    }

    if (got == UPDATE_PACKET_SIZE)
    {
        got = 0;

        if (PC_CHECK_HEAD(pc->rx_buff[PC_HEAD1], pc->rx_buff[1035]))
        {
            memset(pc->check_crc_buff, 0, PC_CRC_BUFF_SIZE);
            convert_data(pc->rx_buff, pc->check_crc_buff, 1, 1030);
            uint32_t crc_value = crc_block_calculate(pc->check_crc_buff, 258);
            crc_data_reset();

            if (pc->rx_buff[1031] == ((crc_value >> 24) & 0xff) &&
                pc->rx_buff[1032] == ((crc_value >> 16) & 0xff) &&
                pc->rx_buff[1033] == ((crc_value >> 8)  & 0xff) &&
                pc->rx_buff[1034] == (crc_value & 0xff))
            {
                spiflash_write(&pc_data.rx_buff[UPDATE_DATA_OFFSET],
                               current_flash_addr, UPDATE_DATA_LEN);
                current_flash_addr += 1024;
                ask_data_cmd_to_pc(&pc_data, ASK_OK);

                if (pc->rx_buff[2] == 0x04)
                {
                    wk_delay_ms(20);
                    nvic_system_reset();
                }
            }
            else
            {
                ask_data_cmd_to_pc(&pc_data, ASK_ERROR);
            }
        }
        else
        {
            ask_data_cmd_to_pc(&pc_data, ASK_ERROR);
        }
    }
}

void handing_pc_event(CZ01_t *this)
{
    static bool first_in = true;

    /* get info */
    if (first_in)
    {
        get_cz01_version(this);
        this->uid_value = get_mcu_info(this);
        first_in = false;
    }

    switch (pc_event)
    {
    case CONNECT_PC_EVENT:
        ask_connect_cmd_to_pc(&pc_data, this);
        pc_event = PC_END_EVENT;
        break;

    case IAP_EVENT:
        pc_event = PC_END_EVENT;
        iap_flag = IAP_REV_FLAG_DONE;
        break;

    case FLASU_UPDATA_EVENT:
        this->flash_updata = true;
        memset(pc_data.rx_buff, 0, PC_RX_BUFF_SIZE);
        ask_data_cmd_to_pc(&pc_data, ASK_OK);
        pc_event = PC_END_EVENT;
        break;

    case PC_END_EVENT:
        break;
    }
}

#endif /* !PC_FLASH_SIMPLE_ENABLE */

#if (PC_FLASH_SIMPLE_ENABLE && !PC_LOOPBACK_TEST)
/*=============================================================================
 * Debug: standalone flash burning via SSCOM
 *   start frame : D1 AA AA AA AA FC    -> enter burning
 *   data        : 256 B blocks (host paced ~50 ms), no checks
 *   auto end    : no explicit end frame. When no data has arrived for
 *                 FS_TIMEOUT_MS, the remaining bytes are written and the
 *                 "done" frame is sent back.
 *   reply frame : D1 CC CC CC CC FC
 *   When this macro is on, pc_handle() runs ONLY this flow.
 *===========================================================================*/
#define FS_START_LEN   6
#define FS_TIMEOUT_MS  3000  /* idle time before the transfer is considered done */
#define FS_WRITE_CHUNK 2048  /* data bytes collected before one flash write */

static const uint8_t fs_start_seq[FS_START_LEN] = {0xD1,0xAA,0xAA,0xAA,0xAA,0xFC};

typedef enum { FS_IDLE, FS_DATA } fs_state_e;
static fs_state_e fs_state = FS_IDLE;
static uint16_t   fs_pos   = 0;
static uint8_t    fs_blk[FS_WRITE_CHUNK];

/* Send the "done" reply frame */
static void fs_send_done(void)
{
    static const uint8_t done[6] = {0xD1,0xCC,0xCC,0xCC,0xCC,0xFC};
    uart_send(uart1,done, 6);
}

/* Periodic state machine, called every PC_HANDLE_TIME from pc_handle() */
static void fs_flash_wait(void)
{
    static uint16_t start_len  = 0;
    static uint32_t last_data_tick = 0;
    static uint32_t fs_addr = SPI_FLASH_UPDATE_START_ADDR;

    switch (fs_state)
    {
    case FS_IDLE:
        /* wait for the start frame */
        while (uart_rx_available(uart1))
        {
            uint8_t b = uart_read_byte(uart1);

            if (b == fs_start_seq[start_len]) { start_len++; }
            else if (b == 0xD1)                { start_len = 1; }
            else                               { start_len = 0; }

            if (start_len == FS_START_LEN)
            {
                start_len = 0;
                fs_addr = SPI_FLASH_UPDATE_START_ADDR;
                fs_pos  = 0;
                last_data_tick = wk_timebase_get();
                fs_state = FS_DATA;
                break;
            }
        }
        break;

    case FS_DATA:
        /* receive data until FS_TIMEOUT_MS of silence */
        while (uart_rx_available(uart1))
        {
            fs_blk[fs_pos++] = uart_read_byte(uart1);
            last_data_tick  = wk_timebase_get();

            if (fs_pos >= FS_WRITE_CHUNK)      /* flush a full block */
            {
                spiflash_write(fs_blk, fs_addr, fs_pos);
                fs_addr += fs_pos;
                fs_pos  = 0;
            }
        }

        /* no data for FS_TIMEOUT_MS: the transfer is finished */
        if ((wk_timebase_get() - last_data_tick) >= FS_TIMEOUT_MS)
        {
            if (fs_pos > 0)
            {
                spiflash_write(fs_blk, fs_addr, fs_pos);
            }
            fs_send_done();
            fs_state = FS_IDLE;
            fs_pos  = 0;
        }
        break;
    }
}
#endif /* PC_FLASH_SIMPLE_ENABLE */

#if PC_LOOPBACK_TEST
/*=============================================================================
 * Debug: UART loopback test (no flash write)
 *  - start frame : D1 AA AA AA AA FC
 *  - data        : everything after the start frame is buffered
 *  - 3 s of silence ends the transfer and ALL received bytes are echoed back
 *  Purpose: verify the PC -> MCU receive path before touching the flash.
 *===========================================================================*/
#define LB_START_LEN  6
#define LB_BUF_SIZE   3072    /* >= largest expected packet (test file ~2960 B) */
#define LB_TIMEOUT_MS 3000

typedef enum { LB_IDLE, LB_RECV } lb_state_e;
static lb_state_e lb_state = LB_IDLE;
static uint16_t   lb_len   = 0;
static uint8_t    lb_buf[LB_BUF_SIZE];

static void loopback_test(void)
{
    static uint16_t start_len = 0;
    static uint32_t last_tick = 0;
    static const uint8_t seq[LB_START_LEN] = {0xD1,0xAA,0xAA,0xAA,0xAA,0xFC};

    switch (lb_state)
    {
    case LB_IDLE:
        /* wait for the start frame */
        while (uart_rx_available())
        {
            uint8_t b = uart_read_byte();

            if (b == seq[start_len]) { start_len++; }
            else if (b == 0xD1)      { start_len = 1; }
            else                     { start_len = 0; }

            if (start_len == LB_START_LEN)
            {
                start_len = 0;
                lb_len = 0;
                last_tick = wk_timebase_get();
                lb_state = LB_RECV;
                break;
            }
        }
        break;

    case LB_RECV:
        /* buffer everything that arrives */
        while (uart_rx_available() && lb_len < LB_BUF_SIZE)
        {
            lb_buf[lb_len++] = uart_read_byte();
            last_tick        = wk_timebase_get();
        }

        /* 3 s of silence: transfer finished, echo everything back */
        if ((wk_timebase_get() - last_tick) >= LB_TIMEOUT_MS)
        {
            uart_send_blocking(lb_buf, lb_len);
            lb_len  = 0;
            lb_state = LB_IDLE;
        }
        break;
    }
}
#endif /* PC_LOOPBACK_TEST */

void pc_handle(void)
{
#if PC_LOOPBACK_TEST
    loopback_test();
#elif PC_FLASH_SIMPLE_ENABLE
    fs_flash_wait();
#else
    get_event_from_pc(&scz01, &pc_data);
    get_flash_data_from_pc(&scz01, &pc_data);
    handing_pc_event(&scz01);
#endif
}