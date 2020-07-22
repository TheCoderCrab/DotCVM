#include <DotCVM/devices/devices.h>
#include <DotCVM/devices/interrupts.h>
#include <DotCVM/gui/window.h>
#include <DotCVM/utils/err_code.h>

Mouse::Mouse()
{
    m_PointerX      = 0;
    m_PointerY      = 0;
    m_LastEventType = MouseEventType::NONE;
    m_ButtonNumber  = 0;
}
void Mouse::move(CPU* cpu, dword x, dword y)
{
    m_PointerX = x;
    m_PointerY = y;
    m_LastEventType = MouseEventType::MOTION;
    cpu->intr(MOUSE_MOTION_INTERRUPT);
}
void Mouse::press(CPU* cpu, dword buttonNum)
{
    m_ButtonNumber = buttonNum;
    m_LastEventType = MouseEventType::BUTTON_PRESS;
    cpu->intr(MOUSE_PRESS_INTERRUPT);
}
void Mouse::release(CPU* cpu, dword buttonNum)
{
    m_ButtonNumber = buttonNum;
    m_LastEventType = MouseEventType::BUTTON_RELEASE;
    cpu->intr(MOUSE_RELEASE_INTERRUPT);
}

dword Mouse::out()
{
    if(m_LastEventType == MouseEventType::MOTION)
        return (m_PointerX << 16) | m_PointerY;
    if(m_LastEventType == MouseEventType::BUTTON_PRESS || m_LastEventType == MouseEventType::BUTTON_RELEASE)
        return m_ButtonNumber;
    requestClose(IO_INVALID_OPERATION, "Invalid mouse operation");
    return -1;
}