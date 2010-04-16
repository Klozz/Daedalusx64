#include <pspsdk.h>
#include <pspkernel.h>
#include <pspctrl.h>
 
PSP_MODULE_INFO("buttons", 0x1000, 1, 1);
//PSP_HEAP_SIZE_KB(32);
//PSP_NO_CREATE_MAIN_THREAD();
 
int getbuttons()
{
	u32 k1 = pspSdkGetK1();
	 
	pspSdkSetK1(0);
	 
	SceCtrlData pad;
	 
	sceCtrlReadBufferPositive(&pad, 1);
	 
	pspSdkSetK1(k1);

	return pad.Buttons;
}
 
int module_start(SceSize args, void *argp)
{
	return 0;
}
 
int module_stop()
{
	return 0;
}