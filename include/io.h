#ifndef IO_H
#define IO_H

enum IOInputMode { INSTRUCTION, ARGUMENT };

#define IO_DRIVE_ADR             0xD514E010
#define IO_MEM_ADR               0x8C5EEFAB
#define IO_SCR_ADR               0x3E33E33E

#define IO_DRIVE_READ_BLOCKS     0x00 /* Perform a read operation */
#define IO_DRIVE_WRITE_BLOCKS    0x01 /* Perform a write operation */
#define IO_DRIVE_SET_BLOCK_ADR   0x02 /* Address to load to or write to in drive */
#define IO_DRIVE_SET_BLOCK_COUNT 0x03 /* Number of blocks to read or write */
#define IO_DRIVE_SET_MEM_ADR     0x04 /* Address to load to or write to in memory */
#define IO_DRIVE_STATUS          0x05 /* Get status of last operation */
#define IO_DRIVE_GET_SIZE        0x06 /* Get disk size in blocks */

#define IO_SCR_SET_TEXT_MODE     0x00 /* Set screen to text mode */
#define IO_SCR_SET_PXL_MODE      0x01 /* Set screen to pixel mode*/
#define IO_SCR_SET_CURSOR_X      0x02 /* Set cursor x position (only in text mode)*/
#define IO_SCR_SET_CURSOR_Y      0x03 /* Set cursor y position (only in text mode)*/
#define IO_SCR_FORCE_REFRESH     0x04 /* Forces a screen refresh */

#endif