
/* ------------------------------------------------------------ */
/*				Include File Definitions						*/
/* ------------------------------------------------------------ */

#include "xaxivdma.h"
#include "sii9022_init/sii9022_init.h"
#include <stdio.h>
#include "xuartps.h"
#include "math.h"
#include <ctype.h>
#include <stdlib.h>
#include "xil_types.h"
#include "xil_cache.h"
#include "xparameters.h"


#include "xil_types.h"
#include "vga_modes.h"

#include "xvtc.h"
#include "sleep.h"
#include "xil_printf.h"

/* ------------------------------------------------------------ */
/*					Miscellaneous Declarations					*/
/* ------------------------------------------------------------ */


#define BIT_DISPLAY_RED 16
#define BIT_DISPLAY_BLUE 8
#define BIT_DISPLAY_GREEN 0

#define VGA_VDMA_ID XPAR_AXIVDMA_0_DEVICE_ID
#define DISP_VTC_ID XPAR_VTC_0_DEVICE_ID
#define VID_VTC_IRPT_ID XPS_FPGA3_INT_ID
#define VID_GPIO_IRPT_ID XPS_FPGA4_INT_ID
#define SCU_TIMER_ID XPAR_SCUTIMER_DEVICE_ID
#define UART_BASEADDR XPAR_PS7_UART_1_BASEADDR

#define H_STRIDE            1920
#define H_ACTIVE            1920
#define V_ACTIVE            1080

#define VIDEO_LENGTH  (H_STRIDE*V_ACTIVE)
#define VIDEO_BASEADDR0 DDR_BASEADDR + 0x2000000
#define VIDEO_BASEADDR1 DDR_BASEADDR + 0x3000000
#define VIDEO_BASEADDR2 DDR_BASEADDR + 0x4000000

#define DISPLAY_NUM_FRAMES 3
#define DEMO_PATTERN_0 0
#define DEMO_PATTERN_1 1

#define DEMO_MAX_FRAME (1920*1080*4)
#define DEMO_STRIDE (1920 * 4)

extern const unsigned char lcd[1536000];
extern const unsigned char zturnlcd[1152000];
extern const unsigned char zturnhdmi[6220800];
extern const unsigned char gImage_beijing[8294400];


/*
 * Framebuffers for video data
 */
u8 frameBuf[DISPLAY_NUM_FRAMES][DEMO_MAX_FRAME];
u8 *pFrames[DISPLAY_NUM_FRAMES]; //array of pointers to the frame buffers


/* ------------------------------------------------------------ */
/*				Procedure Definitions							*/
/* ------------------------------------------------------------ */


XAxiVdma vdma;

typedef enum {
	DISPLAY_STOPPED = 0,
	DISPLAY_RUNNING = 1
} DisplayState;

typedef struct {
		u32 dynClkAddr; /*Physical Base address of the dynclk core*/
		XAxiVdma *vdma; /*VDMA driver struct*/
		XAxiVdma_DmaSetup vdmaConfig; /*VDMA channel configuration*/
		XVtc vtc; /*VTC driver struct*/
		VideoMode vMode; /*Current Video mode*/
		u8 *framePtr[DISPLAY_NUM_FRAMES]; /* Array of pointers to the framebuffers */
		u32 stride; /* The line stride of the framebuffers, in bytes */
		double pxlFreq; /* Frequency of clock currently being generated */
		u32 curFrame; /* Current frame being displayed */
		DisplayState state; /* Indicates if the Display is currently running */
} DisplayCtrl;

DisplayCtrl dispCtrl;

int DisplayStart(DisplayCtrl *dispPtr)
{
	int Status;
	int i;
	XVtc_Timing vtcTiming;
	XVtc_SourceSelect SourceSelect;

	xdbg_printf(XDBG_DEBUG_GENERAL, "display start entered\n\r");

	if (dispPtr->state == DISPLAY_RUNNING)
	{
		return XST_SUCCESS;
	}

	/*
	 * Configure the vtc core with the display mode timing parameters
	 */
	vtcTiming.HActiveVideo = dispPtr->vMode.width;	/**< Horizontal Active Video Size */
	vtcTiming.HFrontPorch = dispPtr->vMode.hps - dispPtr->vMode.width;	/**< Horizontal Front Porch Size */
	vtcTiming.HSyncWidth = dispPtr->vMode.hpe - dispPtr->vMode.hps;		/**< Horizontal Sync Width */
	vtcTiming.HBackPorch = dispPtr->vMode.hmax - dispPtr->vMode.hpe + 1;		/**< Horizontal Back Porch Size */
	vtcTiming.HSyncPolarity = dispPtr->vMode.hpol;	/**< Horizontal Sync Polarity */
	vtcTiming.VActiveVideo = dispPtr->vMode.height;	/**< Vertical Active Video Size */
	vtcTiming.V0FrontPorch = dispPtr->vMode.vps - dispPtr->vMode.height;	/**< Vertical Front Porch Size */
	vtcTiming.V0SyncWidth = dispPtr->vMode.vpe - dispPtr->vMode.vps;	/**< Vertical Sync Width */
	vtcTiming.V0BackPorch = dispPtr->vMode.vmax - dispPtr->vMode.vpe + 1;;	/**< Horizontal Back Porch Size */
	vtcTiming.V1FrontPorch = dispPtr->vMode.vps - dispPtr->vMode.height;	/**< Vertical Front Porch Size */
	vtcTiming.V1SyncWidth = dispPtr->vMode.vpe - dispPtr->vMode.vps;	/**< Vertical Sync Width */
	vtcTiming.V1BackPorch = dispPtr->vMode.vmax - dispPtr->vMode.vpe + 1;;	/**< Horizontal Back Porch Size */
	vtcTiming.VSyncPolarity = dispPtr->vMode.vpol;	/**< Vertical Sync Polarity */
	vtcTiming.Interlaced = 0;		/**< Interlaced / Progressive video */

	/* Setup the VTC Source Select config structure. */
	/* 1=Generator registers are source */
	/* 0=Detector registers are source */
	memset((void *)&SourceSelect, 0, sizeof(SourceSelect));
	SourceSelect.VBlankPolSrc = 1;
	SourceSelect.VSyncPolSrc = 1;
	SourceSelect.HBlankPolSrc = 1;
	SourceSelect.HSyncPolSrc = 1;
	SourceSelect.ActiveVideoPolSrc = 1;
	SourceSelect.ActiveChromaPolSrc= 1;
	SourceSelect.VChromaSrc = 1;
	SourceSelect.VActiveSrc = 1;
	SourceSelect.VBackPorchSrc = 1;
	SourceSelect.VSyncSrc = 1;
	SourceSelect.VFrontPorchSrc = 1;
	SourceSelect.VTotalSrc = 1;
	SourceSelect.HActiveSrc = 1;
	SourceSelect.HBackPorchSrc = 1;
	SourceSelect.HSyncSrc = 1;
	SourceSelect.HFrontPorchSrc = 1;
	SourceSelect.HTotalSrc = 1;

	XVtc_SelfTest(&(dispPtr->vtc));

	XVtc_RegUpdateEnable(&(dispPtr->vtc));
	XVtc_SetGeneratorTiming(&(dispPtr->vtc), &vtcTiming);
	XVtc_SetSource(&(dispPtr->vtc), &SourceSelect);
    /*
	 * Enable VTC core, releasing backpressure on VDMA
	 */
	XVtc_EnableGenerator(&dispPtr->vtc);

	/*
	 * Configure the VDMA to access a frame with the same dimensions as the
	 * current mode
	 */
	dispPtr->vdmaConfig.VertSizeInput = dispPtr->vMode.height;
	dispPtr->vdmaConfig.HoriSizeInput = (dispPtr->vMode.width) * 4;
	dispPtr->vdmaConfig.FixedFrameStoreAddr = dispPtr->curFrame;
	/*
	 *Also reset the stride and address values, in case the user manually changed them
	 */
	dispPtr->vdmaConfig.Stride = dispPtr->stride;
	for (i = 0; i < DISPLAY_NUM_FRAMES; i++)
	{
		dispPtr->vdmaConfig.FrameStoreStartAddr[i] = (u32)  dispPtr->framePtr[i];
	}

	/*
	 * Perform the VDMA driver calls required to start a transfer. Note that no data is actually
	 * transferred until the disp_ctrl core signals the VDMA core by pulsing fsync.
	 */

	Status = XAxiVdma_DmaConfig(dispPtr->vdma, XAXIVDMA_READ, &(dispPtr->vdmaConfig));
	if (Status != XST_SUCCESS)
	{
		xdbg_printf(XDBG_DEBUG_GENERAL, "Read channel config failed %d\r\n", Status);
		return XST_FAILURE;
	}
	Status = XAxiVdma_DmaSetBufferAddr(dispPtr->vdma, XAXIVDMA_READ, dispPtr->vdmaConfig.FrameStoreStartAddr);
	if (Status != XST_SUCCESS)
	{
		xdbg_printf(XDBG_DEBUG_GENERAL, "Read channel set buffer address failed %d\r\n", Status);
		return XST_FAILURE;
	}
	Status = XAxiVdma_DmaStart(dispPtr->vdma, XAXIVDMA_READ);
	if (Status != XST_SUCCESS)
	{
		xdbg_printf(XDBG_DEBUG_GENERAL, "Start read transfer failed %d\r\n", Status);
		return XST_FAILURE;
	}
	Status = XAxiVdma_StartParking(dispPtr->vdma, dispPtr->curFrame, XAXIVDMA_READ);
	if (Status != XST_SUCCESS)
	{
		xdbg_printf(XDBG_DEBUG_GENERAL, "Unable to park the channel %d\r\n", Status);
		return XST_FAILURE;
	}

	dispPtr->state = DISPLAY_RUNNING;

	return XST_SUCCESS;
}

/* ------------------------------------------------------------ */

/***	DisplayInitialize(DisplayCtrl *dispPtr, XAxiVdma *vdma, u16 vtcId, u32 dynClkAddr, u8 *framePtr[DISPLAY_NUM_FRAMES], u32 stride)
**
**	Parameters:
**		dispPtr - Pointer to the struct that will be initialized
**		vdma - Pointer to initialized VDMA struct
**		vtcId - Device ID of the VTC core as found in xparameters.h
**		dynClkAddr - BASE ADDRESS of the axi_dynclk core
**		framePtr - array of pointers to the framebuffers. The framebuffers must be instantiated above this driver, and there must be 3
**		stride - line stride of the framebuffers. This is the number of bytes between the start of one line and the start of another.
**
**	Return Value: int
**		XST_SUCCESS if successful, XST_FAILURE otherwise
**
**	Errors:
**
**	Description:
**		Initializes the driver struct for use.
**
*/
int DisplayInitialize(DisplayCtrl *dispPtr, XAxiVdma *vdma, u16 vtcId, u8 *framePtr[DISPLAY_NUM_FRAMES], u32 stride)
{
	int Status;
	int i;
	XVtc_Config *vtcConfig;
//	ClkConfig clkReg;
//	ClkMode clkMode;


	/*
	 * Initialize all the fields in the DisplayCtrl struct
	 */
	dispPtr->curFrame = 0;
	for (i = 0; i < DISPLAY_NUM_FRAMES; i++)
	{
		dispPtr->framePtr[i] = framePtr[i];
	}
	dispPtr->state = DISPLAY_STOPPED;
	dispPtr->stride = stride;
	dispPtr->vMode = VMODE_1920x1080;

	/* Initialize the VTC driver so that it's ready to use look up
	 * configuration in the config table, then initialize it.
	 */
	vtcConfig = XVtc_LookupConfig(vtcId);
	/* Checking Config variable */
	if (NULL == vtcConfig) {
		return (XST_FAILURE);
	}
	Status = XVtc_CfgInitialize(&(dispPtr->vtc), vtcConfig, vtcConfig->BaseAddress);
	/* Checking status */
	if (Status != (XST_SUCCESS)) {
		return (XST_FAILURE);
	}

	dispPtr->vdma = vdma;

	/*
	 * Initialize the VDMA Read configuration struct
	 */
	dispPtr->vdmaConfig.FrameDelay = 0;
	dispPtr->vdmaConfig.EnableCircularBuf = 1;
	dispPtr->vdmaConfig.EnableSync = 0;
	dispPtr->vdmaConfig.PointNum = 0;
	dispPtr->vdmaConfig.EnableFrameCounter = 0;

	return XST_SUCCESS;
}

void DemoPrintTest(u8 *frame, u32 width, u32 height, u32 stride, int pattern)
{
//	return ;
	u32 i,j;

	switch (pattern)
	{
	case DEMO_PATTERN_0:
		memcpy(frame, gImage_beijing, 1920*1080*4);
		/*for (i = 0; i < 1080; i++)
		{
			for (j = 0; j < 1920; j++)
			{
				frame[1920*4*i+4*j]=gImage_beijing[1920*4*i+4*j+0];
				frame[1920*4*i+4*j+1]=gImage_beijing[1920*4*i+4*j+1];
				frame[1920*4*i+4*j+2]=gImage_beijing[1920*4*i+4*j+2];
				frame[1920*4*i+4*j+3]=gImage_beijing[1920*4*i+4*j+3];
			}
		}*/



		Xil_DCacheFlushRange((unsigned int) frame, DEMO_MAX_FRAME);
		break;
	case DEMO_PATTERN_1:

		for (i = 0; i < 1080; i++)
		{
			for (j = 0; j < 1920; j++)
			{
				frame[1920*4*i+4*j]=0x00;
				frame[1920*4*i+4*j+1]=zturnhdmi[1920*3*(1079-i)+3*j+0];
				frame[1920*4*i+4*j+2]=zturnhdmi[1920*3*(1079-i)+3*j+1];
				frame[1920*4*i+4*j+3]=zturnhdmi[1920*3*(1079-i)+3*j+2];
			}
		}

		/*
		 * Flush the framebuffer memory range to ensure changes are written to the
		 * actual memory, and therefore accessible by the VDMA.
		 */
		Xil_DCacheFlushRange((unsigned int) frame, DEMO_MAX_FRAME);
		break;
	default :
		xil_printf("Error: invalid pattern passed to DemoPrintTest");
	}
}


void Display()
{
	int Status;
	XAxiVdma_Config *vdmaConfig;
	int i;
	int j;
	/*
	 * Initialize an array of pointers to the 3 frame buffers
	 */

	for (i = 0; i < DISPLAY_NUM_FRAMES; i++)
	{
		pFrames[i] = frameBuf[i];
	}

	for (i = 0; i < 1080; i++)
	{
		for (j = 0; j < 1920; j++)
		{
			frameBuf[0][1920*4*i+4*j]=gImage_beijing[1920*4*i+4*j+0];
			frameBuf[0][1920*4*i+4*j+1]=gImage_beijing[1920*4*i+4*j+1];
			frameBuf[0][1920*4*i+4*j+2]=gImage_beijing[1920*4*i+4*j+2];
			frameBuf[0][1920*4*i+4*j+3]=gImage_beijing[1920*4*i+4*j+3];
		}
	}

	/*
	 * Initialize VDMA driver
	 */
	vdmaConfig = XAxiVdma_LookupConfig(VGA_VDMA_ID);
	if (!vdmaConfig)
	{
		xil_printf("No video DMA found for ID %d\r\n", VGA_VDMA_ID);
		return;
	}

	Status = XAxiVdma_CfgInitialize(&vdma, vdmaConfig, vdmaConfig->BaseAddress);
	if (Status != XST_SUCCESS)
	{
		xil_printf("VDMA Configuration Initialization failed %d\r\n", Status);
		return;
	}

	/*
	 * Initialize the Display controller and start it
	 */
	Status = DisplayInitialize(&dispCtrl, &vdma, DISP_VTC_ID, pFrames, DEMO_STRIDE);
	if (Status != XST_SUCCESS)
	{
		xil_printf("Display Ctrl initialization failed during demo initialization%d\r\n", Status);
		return;
	}

	Status = DisplayStart(&dispCtrl);
	if (Status != XST_SUCCESS)
	{
		xil_printf("Couldn't start display during demo initialization%d\r\n", Status);
		return;
	}

	while(1)
	{
		DemoPrintTest(dispCtrl.framePtr[dispCtrl.curFrame], dispCtrl.vMode.width, dispCtrl.vMode.height, dispCtrl.stride, DEMO_PATTERN_1);
		xil_printf("press any key to change pattern\n\r");
		XUartPs_RecvByte(STDOUT_BASEADDRESS);
		DemoPrintTest(dispCtrl.framePtr[dispCtrl.curFrame], dispCtrl.vMode.width, dispCtrl.vMode.height, dispCtrl.stride, DEMO_PATTERN_0);
		xil_printf("press any key to change pattern\n\r");
		XUartPs_RecvByte(STDOUT_BASEADDRESS);
	}


	return;
}

int main(void)
{
	xil_printf("/* ------------------------------------------------------------ */\n\r");
	xil_printf("/*                       MYIR display test                      */\n\r");
	xil_printf("/* ------------------------------------------------------------ */\n\r");

	sii9022_init();

	Display();

	return 0;
}


