// TODO: separate window creation from this file
// TODO: make window creation for other plateforms

#include <DotCVM/utils/log.h>
#include <DotCVM/devices/devices.h>
#include <DotCVM/debug/debug.h>
#include <DotCVM/gui/window.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>

WindowData mainWin;

static bool s_CloseRequested = false;
static int s_ExitCode = 0;
static std::string* s_CloseMessage = new std::string("");

static uint* s_CurrentScreenBuffer = new uint[720 * 480];

void requestClose(int exitCode, const char* msg)
{
   s_CloseRequested = true;
   s_ExitCode       = exitCode;
   delete s_CloseMessage;
   s_CloseMessage   = new std::string(msg);
}

#ifdef DEBUG
static const char* s_WinTitle = "DotCVM_DEBUG";
#else
static const char* s_WinTitle = "DotCVM";
#endif

void init()
{
   for(uint x = 0; x < SCR_WIDTH; x++)
      for(uint y = 0; y < SCR_HEIGHT; y++)
         s_CurrentScreenBuffer[x + y * SCR_WIDTH] = 0;
}

void update()
{
   devices::update();
}

static bool hasChanged(uint* scrBuffer)
{
   for(uint x = 0; x < SCR_WIDTH; x++)
      for(uint y = 0; y < SCR_HEIGHT; y++)
            if(scrBuffer[x + y * SCR_WIDTH] != s_CurrentScreenBuffer[x + y * SCR_WIDTH])
            {
               debug("Pixel at: " << x << " " << y << " changed from " << s_CurrentScreenBuffer[x + y * SCR_WIDTH] << " to " << scrBuffer[x + y * SCR_WIDTH]);
               return true;
            }
   return false;
}

void refreshScr(WindowData window, uint* screenBuffer, bool force)
{
   if(hasChanged(screenBuffer) || force)
   {
      debug("Refreshing screen");
      for(uint x = 0; x < SCR_WIDTH; x++)
         for(uint y = 0; y < SCR_HEIGHT; y++)
         {
            setPixel(window, x, y, screenBuffer[x + y * SCR_WIDTH]);
            s_CurrentScreenBuffer[x + y * SCR_WIDTH] = screenBuffer[x + y * SCR_WIDTH];
         }
   }
}

void exit()
{
   log("Closing DotC Virtual Machine");

   // TODO: do this only if asked to
   log("Saving machine state");
   FILE* statFile = fopen("dotcvm_stat.dat", "w+");
   fwrite(&(devices::cpu->mem()[0]), 1, devices::cpu->mem().size(), statFile);
   fclose(statFile);
   log("Saved machine stat");
   devices::exit();
   log(*s_CloseMessage);
   delete s_CloseMessage;
   delete s_CurrentScreenBuffer;
   checkMemoryLeaks();
   log("Bye");
}

int main() {
   log("Starting DotC Virtual Machine");
   debug("Setting up signal handler");
   setupSignalHandler();
   debug("Creating window");
   mainWin = setupMainWindow(s_WinTitle, 720, 480);
   init();
   devices::init(MEM_SIZE, DISK_SIZE);
   while (!s_CloseRequested) 
   {  
      update();
      updateMainWindow();
   }
   closeWindow(mainWin);
   exit();
   closeDisplay();
   return s_ExitCode;
}


