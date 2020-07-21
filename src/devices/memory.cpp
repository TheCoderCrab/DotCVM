#include <DotCVM/devices/devices.h>
#include <DotCVM/utils/log.h>

Memory::Memory(uint32_t size)
{
    debug("Creating a new instance of Memory");
    m_Size = size;
    debug("Memory size set");
    m_Data = new data[m_Size];
    debug("Allocated space for machine memory");
    debug("Memory created");
}
Memory::~Memory() { delete [] m_Data; }
byte& Memory::operator[](address adr) { return at<byte>(adr); }
dword Memory::size() { return m_Size; }
dword Memory::out()  { return m_Size; }
