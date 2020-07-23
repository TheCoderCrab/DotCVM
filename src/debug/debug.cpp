#define DC_LINKED_LIST_USE_MALLOC // To use linked list with malloc instead of new, thus preventing infinite calls to operator new

#include <DotCVM/gui/window.h>
#include <DotCVM/utils/err_code.h>
#include <DotCVM/utils/linkedlist.h>
#include <signal.h>


#ifdef DEBUG

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
    debug("Memory allocated using malloc");
    allocationDataList.add(AllocationData(ptr, size));
    debug("Added data to list");
    debug("Allocation data:\n\tSize: " << allocationDataList.lastItem().size() << "\n\tAddress: " << allocationDataList.lastItem().ptr());
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
        debug("Block was not found in register... there might be some problem here! Pointer: " << ptr);
    debug("Size: " << size);
    debug("Deallocated: " << ptr);
    free(ptr);
}
#endif

static void signalHandler(int nSignum, siginfo_t* si, void* vcontext)
{
    if(nSignum == SIGSEGV)
    {
        debug("Caught a SIGSEGV");
        debug("Pointer: " << vcontext);
        debug("Signal number: " << nSignum);
        debug("Signal info:");
        debug("\tSignal number      : " << si->si_signo);
        debug("\tSignal code        : " << si->si_code);
        debug("\tSignal error number: " << si->si_signo);
        log("Check errors/faultHandler.txt");
        FILE* errorLogFile = fopen("errors/faultHandler.txt", "w+");
        fprintf(errorLogFile,   "Error occured!\n"
                                "Signal number: %d\n"
                                "Signal info:\n"
                                "\tSignal number      : %d\n"
                                "\tSignal code        : %d\n"
                                "\tSignal error number: %d\n",
                                nSignum, si->si_signo, si->si_code, si->si_signo);
        fclose(errorLogFile);
        requestClose(SIGNAL_SIGSEGV, "Error occured!");
        return;
    }
    if(nSignum == SIGINT)
    {
       requestClose(SIGNAL_INT, "Program interrupted");
    }
}

void setupSignalHandler()
{
    debug("Setting up fault handler");
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_flags = SA_SIGINFO;
    action.sa_sigaction = signalHandler;

    sigaction(SIGSEGV, &action, NULL);
    sigaction(SIGINT, &action, NULL);
}

void checkMemoryLeaks()
{
#ifdef DEBUG
    debug("Checking for memory leaks");
    if(allocationDataList.size() == 0)
    {
        debug("No memory leaks!");
        return;
    }
    for(size_t i = 0; i < allocationDataList.size(); i++)
    {
        debug("Memory leak at: " << allocationDataList[i].ptr() << ", of size: " << allocationDataList[i].size());
    }
#endif
}

