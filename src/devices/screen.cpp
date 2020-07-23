#include <DotCVM/devices/devices.h>
#include <DotCVM/utils/log.h>
#include <DotCVM/utils/err_code.h>
#include <DotCVM/gui/window.h>

ScreenDevice::ScreenDevice(Memory* mem)
{
    debug("Creating screen");
    m_ScrBuffer = &(mem->at<dword>(SCR_BUFFER));
    m_InputMode = IOInputMode::INSTRUCTION;
    for(uint i = 0; i < SCR_HEIGHT * SCR_WIDTH; i++)
        m_ScrBuffer[i] = 0;
    debug("Screen created");
}
dword* ScreenDevice::buffer()
{
    return m_ScrBuffer;
}
void ScreenDevice::update()
{
    refreshScr({mainWin}, m_ScrBuffer);
}
void ScreenDevice::in(dword in)
{
    if(m_InputMode == IOInputMode::INSTRUCTION)
    {
        switch (in)
        {
        case IO_SCR_SET_TEXT_MODE:
            m_Mode = ScreenMode::TEXT_MODE;
            break;
        case IO_SCR_SET_PXL_MODE:
            m_Mode = ScreenMode::PXL_MODE;
            break;
        case IO_SCR_SET_CURSOR_X:
        case IO_SCR_SET_CURSOR_Y:
            break;
        case IO_SCR_FORCE_REFRESH:
            refreshScr({mainWin}, m_ScrBuffer, true);
            break;
        default:
            requestClose(IO_INVALID_OPERATION, "Invalid screen IO instruction");
            break;
        }
        m_io_LastInstruction = in;
        return;
    }
    if(m_InputMode == IOInputMode::ARGUMENT)
    {
        switch (m_io_LastInstruction)
        {
        case IO_SCR_SET_CURSOR_X:
            m_io_cx = in;
            break;
        case IO_SCR_SET_CURSOR_Y:
            m_io_cy = in;
            break;
        default:
            requestClose(IO_INVALID_OPERATION, "Invalid screen IO argument");
            break;
        }
    }
}
