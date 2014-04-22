#include <stdio.h>
#include <stdlib.h>
#include <gccore.h>
#include <wiiuse/wpad.h>

#define min(x,y) (x < y ? x : y)
#define max(x,y) (x > y ? x : y)

static void *xfb[2];
static GXRModeObj *videoMode = NULL;

int ball[2] = {318, 238},
    ballSpeed[2] = {1,1};

void drawBoxAt(u32 *buffer, GXRModeObj *vmode, int x, int y, int w, int h, int color) {
    int maxX = x + w,
        maxY = y + h,
        iX = x,
        iY = y;
    for(iY=y; iY<maxY; iY++) {
        for(iX=x; iX<maxX; iX++) {
            buffer[(iY * vmode->viWidth + iX) / VI_DISPLAY_PIX_SZ] = color;
        }
    }
}

void moveBall() {
    int minX =  0, 
        minY = 40,
        maxX = videoMode->viWidth  - 5,
        maxY = videoMode->viHeight - 5;

    ball[0] += ballSpeed[0];
    ball[1] += ballSpeed[1];

    if(ball[0] <= minX || ball[0] >= maxX) {
        if(ball[0] <= minX) ball[0] = minX;
        else ball[0] = maxX;

        ballSpeed[0] *= -1;
    }
    if(ball[1] <= minY || ball[1] >= maxY) {
        if(ball[1] <= minY) ball[1] = minY;
        else ball[1] = maxY;

        ballSpeed[1] *= -1;
    }
}

int main(int argc, char **argv) {
	VIDEO_Init();
	WPAD_Init();

	videoMode = VIDEO_GetPreferredMode(NULL);
    //videoMode->viTVMode = VI_TVMODE_MPAL_DS;

	// Allocate memory for the display in the uncached region
	xfb[0] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(videoMode));
	xfb[1] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(videoMode));
	
	//console_init(xfb,20,20,rmode->fbWidth,rmode->xfbHeight,rmode->fbWidth*VI_DISPLAY_PIX_SZ);
	
	VIDEO_Configure(videoMode);
	VIDEO_SetNextFramebuffer(xfb[0]);
	
	// Make the display visible
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	VIDEO_WaitVSync();
	//if(videoMode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();

    WPAD_SetDataFormat(0, WPAD_FMT_BTNS_ACC_IR);
    WPAD_SetVRes(0, videoMode->fbWidth, videoMode->xfbHeight);

    int currentBuffer = 0;
    ir_t irData[2];
    int playerY[2];
    u32 pressed;

	while(1) {
		WPAD_ScanPads();
        WPAD_IR(0, &irData[0]);
        WPAD_IR(1, &irData[1]);

		pressed = WPAD_ButtonsDown(0);
		if( pressed & WPAD_BUTTON_HOME ) exit(0);

        // Calculate stuff
        playerY[0] = max(40, min(irData[0].y, videoMode->viHeight - 80));
        playerY[1] = max(40, min(irData[1].y, videoMode->viHeight - 80));
        moveBall(); 

        // Drawing time!
        
        VIDEO_ClearFrameBuffer(videoMode, xfb[currentBuffer], COLOR_BLACK);

        drawBoxAt((u32 *)xfb[currentBuffer], videoMode,  10, playerY[0], 5, 80, COLOR_WHITE);
        drawBoxAt((u32 *)xfb[currentBuffer], videoMode, videoMode->viWidth - 16, playerY[1], 5, 80, COLOR_WHITE);
      
        drawBoxAt((u32 *)xfb[currentBuffer], videoMode, ball[0], ball[1], 4, 8, COLOR_WHITE);

        VIDEO_WaitVSync();
		VIDEO_SetNextFramebuffer(xfb[currentBuffer]);
        currentBuffer ^= currentBuffer;
	}

	return 0;
}
