#ifdef DEBUG

#include <stddef.h>
#include <stdlib.h>
#include <log.h>
#include <dc_linkedlist.h>

#define DC_LINKED_LIST_USE_MALLOC // To use linked list with malloc instead of new, thus preventing infinite calls to operator new

struct AllocationData
{
private:
    size_t m_Size;
    void* m_Ptr;
public:
    AllocationData(void* ptr, size_t size)
    {
        m_Size = size;
        m_Ptr = ptr;
    }
    AllocationData()
    {
        m_Size = 0;
        m_Ptr = nullptr;
    }
    void* ptr() { return m_Ptr; }
    size_t size() { return m_Size; }
};

static LinkedList<AllocationData> allocationDataList;

void* operator new(size_t size)
{
    debug("Allocating " << size << " bytes of memory");
    void* ptr = malloc(size);
    allocationDataList.add(AllocationData(ptr, size));
    return ptr;
}

void operator delete(void* ptr)
{
    debug("Deallocating: " << ptr);
    size_t size = -1;
    bool found = false;
    for(unsigned int i = 0; i < allocationDataList.size(); i++)
        if(allocationDataList[i].ptr() == ptr)
        {
            if(found)
                debug("Found duplicate allocation data!");
            size = allocationDataList[i].size();
            allocationDataList.remove(i);
            found = true;
        }
    if(!found)
        debug("Block was not found in register... there might be some problem here!\nPointer: " << ptr);
    debug("Size: " << size);
    debug("Deallocated: " << ptr);
    free(ptr);
}
#endif
