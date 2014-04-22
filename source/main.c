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

void drawBoxAt(u32 *buffer, GXRModeObj *vmode, int x, int y, int w, int h) {
    int maxX = x + w,
        maxY = y + h,
        iX = x,
        iY = y;
    for(iY=y; iY<maxY; iY++) {
        for(iX=x; iX<maxX; iX++) {
            buffer[iY * (vmode->fbWidth / VI_DISPLAY_PIX_SZ) + iX] = COLOR_WHITE;
        }
    }
}

void moveBall() {
    ball[0] += ballSpeed[0];
    ball[1] += ballSpeed[1];

    if(ball[0] < 1 || ball[0] > 634) {
        ball[0] = ((ball[0] < 1) ? 1 : 634);
        ballSpeed[0] *= -1;
    }
    if(ball[1] < 1 || ball[1] > 470) {
        ball[1] = ((ball[1] < 1) ? 1 : 470);
        ballSpeed[1] *= -1;
    }
}

int main(int argc, char **argv) {
	VIDEO_Init();
	WPAD_Init();

	videoMode = VIDEO_GetPreferredMode(NULL);

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
	if(videoMode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();

    WPAD_SetDataFormat(0, WPAD_FMT_BTNS_ACC_IR);
    WPAD_SetVRes(0, videoMode->fbWidth, videoMode->xfbHeight);

    int currentBuffer = 0;
    ir_t irData[2];

	while(1) {
		WPAD_ScanPads();
        WPAD_IR(0, &irData[0]);
        WPAD_IR(1, &irData[1]);

        CON_Init(xfb[currentBuffer], 0, 0, 
                 videoMode->fbWidth, videoMode->xfbHeight, videoMode->fbWidth*VI_DISPLAY_PIX_SZ);

		u32 pressed = WPAD_ButtonsDown(0);
		if( pressed & WPAD_BUTTON_HOME ) exit(0);

        int playerY[2];
        playerY[0] = max(0, min(irData[0].y, 399));
        playerY[1] = max(0, min(irData[1].y, 399));
        drawBoxAt((u32 *)xfb[currentBuffer], videoMode,  10, playerY[0], 5, 80);
        drawBoxAt((u32 *)xfb[currentBuffer], videoMode, 629, playerY[1], 5, 80);
       
        moveBall(); 
        drawBoxAt((u32 *)xfb[currentBuffer], videoMode, ball[0], ball[1], 4, 8);

		VIDEO_SetNextFramebuffer(xfb[currentBuffer]);
        VIDEO_Flush();
        VIDEO_WaitVSync();
        currentBuffer ^= currentBuffer;
	}

	return 0;
}
