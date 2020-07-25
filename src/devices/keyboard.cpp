#include <DotCVM/devices/devices.h>
#include <DotCVM/devices/interrupts.h>

Keyboard::Keyboard()
{
    m_LastKey = 0;
}
void Keyboard::press(CPU* cpu, dword keyCode)
{
    if(m_LastKey == 0)
    {
        m_LastKey = keyCode;
        cpu->intr(KEYBOARD_PRESS_INTERRUPT, true);
    }
}
void Keyboard::release(CPU* cpu, dword keyCode)
{
    if(m_LastKey == 0)
    {
        m_LastKey = keyCode;
        cpu->intr(KEYBOARD_PRESS_INTERRUPT, true);
    }
}
dword Keyboard::out()
{
    dword keyCode = m_LastKey;
    m_LastKey = 0;
    return keyCode;
}