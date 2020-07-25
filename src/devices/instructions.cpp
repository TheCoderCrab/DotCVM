#include <DotCVM/devices/instructions.h>
#include <DotCVM/gui/window.h>
#include <DotCVM/utils/err_code.h>

uint32_t sizeOfInstruction(uint8_t* instructionPtr)
{
    return 2 + sizeOfArg(instructionPtr + 1, 0) + sizeOfArg(instructionPtr + 1, 1);
}
uint32_t sizeOfArg(uint8_t* argDescriptorPtr, uint8_t argPosition)
{
    uint8_t argType;
    if(argPosition == 0)
        argType = (*argDescriptorPtr << 4) >> 4;
    else if(argPosition == 1)
        argType = *argDescriptorPtr >> 4;
    else
    {
        requestClose(INSTRUCTIONS_UNRECONIZED_ARG_TYPE, "Unrecognized arg type");
        return -1;
    }
    if(argType == 0x0) // Disabled
        return 0;
    if(argType == 0x1) // literal8
        return 1;
    if(argType == 0x2) // literal16
        return 2;
    if(argType == 0x3) // literal32
        return 4;
    if(0x4 <= argType && argType <= 0x6) // mem
        return 1 + sizeOfArg(argPosition == 0 ? argDescriptorPtr + 1 : argDescriptorPtr + 1 + sizeOfArg(argDescriptorPtr, 0), 0);
    // else register:
    return 1;
}