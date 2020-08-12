#ifndef DC_USER_DEVICE_H
#define DC_USER_DEVICE_H

#include <dotcvm/dotcvm.hpp>
#include <modules/interrupt_bus.hpp>

#define DC_STD_KEB      0xDC57D2EB
#define DC_STD_MSP      0xDC57D359
#define DC_STD_MSB      0xDC57D35B
#define DC_STD_SCX      0xDC57D5C4 /* To set pixel x */
#define DC_STD_SCY      0xDC57D5C5 /* To set pixel y */
#define DC_STD_SCC      0xDC57D5CC /* To set pixel color */
#define DC_STD_SCB      0xDC57D5CB /* To set pixel on buffer */
#define DC_STD_SCD      0xDC57D5CD /* To draw the buffer */

extern void interrupt(interrupt_data d);
extern dotcvm_data s_dc;

#endif /* DC_USER_DEVICE_H */