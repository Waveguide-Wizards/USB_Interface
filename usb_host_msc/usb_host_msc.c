#include <stdbool.h>
#include <stdint.h>
#include <string.h>

//------------------------------------------
// TivaWare Header Files
//------------------------------------------
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/udma.h"
#include "driverlib/rom.h"
#include "driverlib/pin_map.h"
#include "utils/ustdlib.h"
#include "usblib/usblib.h"
#include "usblib/usbmsc.h"
#include "usblib/host/usbhost.h"
#include "usblib/host/usbhmsc.h"
#include "third_party/fatfs/src/ff.h"
#include "third_party/fatfs/src/diskio.h"
#include "driverlib/uart.h"
#include "inc/hw_ints.h"
#include "utils/uartstdio.h"
#include "utils/cmdline.h"
#include "flash.h"




//
// Defines the size of the buffers that hold the path, or temporary
// data from the USB disk.  There are two buffers allocated of this size.
// The buffer size must be large enough to hold the int32_test expected
// full path name, including the file name, and a trailing null character.
//
//*****************************************************************************
#define PATH_BUF_SIZE   80
#define NUM_SSI_DATA            6
//*****************************************************************************
//
// Defines the number of times to call to check if the attached device is
// ready.
//
//*****************************************************************************
#define USBMSC_DRIVE_RETRY      4
#define CMD_BUF_SIZE            64

//*****************************************************************************
//
// This buffer holds the full path to the current working directory.
// Initially it is root ("/").
//
//*****************************************************************************
static char g_pcCwdBuf[PATH_BUF_SIZE] = "/";
static char g_pcTmpBuf[PATH_BUF_SIZE] = "POLYGO~1.GCO";

//*****************************************************************************
//
// The buffer that holds the command line.
//
//*****************************************************************************
static char g_pcCmdBuf[CMD_BUF_SIZE];
//*****************************************************************************
//
// The following are data structures used by FatFs.
//
//*****************************************************************************
static FATFS g_sFatFs;
static DIR g_sDirObject;
static FILINFO g_sFileInfo;
static FIL g_sFileObject;

unsigned int valueToSave;
unsigned int isRead;
//*****************************************************************************
//
// A structure that holds a mapping between an FRESULT numerical code,
// and a string representation.  FRESULT codes are returned from the FatFs
// FAT file system driver.
//
//*****************************************************************************

typedef struct
{
    FRESULT fresult;
    char *pcResultStr;
}
tFresultString;

//*****************************************************************************
//
// A macro to make it easy to add result codes to the table.
//
//*****************************************************************************
#define FRESULT_ENTRY(f)        { (f), (#f) }

//*****************************************************************************
//
// A table that holds a mapping between the numerical FRESULT code and
// it's name as a string.  This is used for looking up error codes and
// providing a human-readable string.
//
//*****************************************************************************
tFresultString g_sFresultStrings[] =
{
    FRESULT_ENTRY(FR_OK),
    FRESULT_ENTRY(FR_DISK_ERR),
    FRESULT_ENTRY(FR_INT_ERR),
    FRESULT_ENTRY(FR_NOT_READY),
    FRESULT_ENTRY(FR_NO_FILE),
    FRESULT_ENTRY(FR_NO_PATH),
    FRESULT_ENTRY(FR_INVALID_NAME),
    FRESULT_ENTRY(FR_DENIED),
    FRESULT_ENTRY(FR_EXIST),
    FRESULT_ENTRY(FR_INVALID_OBJECT),
    FRESULT_ENTRY(FR_WRITE_PROTECTED),
    FRESULT_ENTRY(FR_INVALID_DRIVE),
    FRESULT_ENTRY(FR_NOT_ENABLED),
    FRESULT_ENTRY(FR_NO_FILESYSTEM),
    FRESULT_ENTRY(FR_MKFS_ABORTED),
    FRESULT_ENTRY(FR_TIMEOUT),
    FRESULT_ENTRY(FR_LOCKED),
    FRESULT_ENTRY(FR_NOT_ENOUGH_CORE),
    FRESULT_ENTRY(FR_TOO_MANY_OPEN_FILES),
    FRESULT_ENTRY(FR_INVALID_PARAMETER),
};

//*****************************************************************************
//
// A macro that holds the number of result codes.
//
//*****************************************************************************
#define NUM_FRESULT_CODES (sizeof(g_sFresultStrings) / sizeof(tFresultString))

//*****************************************************************************
//
// Error reasons returned by ChangeToDirectory().
//
//*****************************************************************************
#define NAME_TOO_LONG_ERROR 1
#define OPENDIR_ERROR       2

//*****************************************************************************
//
// The number of SysTick ticks per second.
//
//*****************************************************************************
#define TICKS_PER_SECOND 100
#define MS_PER_SYSTICK (1000 / TICKS_PER_SECOND)

//*****************************************************************************
//
// A counter for system clock ticks, used for simple timing.
//
//*****************************************************************************
static uint32_t g_ui32SysTickCount;

//*****************************************************************************
//
// Holds global flags for the system.
//
//*****************************************************************************
static uint32_t g_ui32Flags = 0;


//*****************************************************************************
//
// Storage for the filenames.
//
//*****************************************************************************
#define NUM_LIST_STRINGS 48
const char *g_ppcDirListStrings[NUM_LIST_STRINGS];

//*****************************************************************************
//
// Storage for the names of the files in the current directory.  Filenames
// are stored in format "(D) filename.ext" for directories or "(F) filename.ext"
// for files.
//
//*****************************************************************************
#define MAX_FILENAME_STRING_LEN (4 + 8 + 1 + 3 + 1)
char g_pcFilenames[NUM_LIST_STRINGS][MAX_FILENAME_STRING_LEN];

//*****************************************************************************
//
// Storage for the strings which appear in the status box at the bottom of the
// display.
//
//****************************************************************************
#define NUM_STATUS_STRINGS 6
#define MAX_STATUS_STRING_LEN (36 + 1)
char g_pcStatus[NUM_STATUS_STRINGS][MAX_STATUS_STRING_LEN];


//*****************************************************************************
//
// Flag indicating that some USB device is connected.
//
//*****************************************************************************
#define FLAGS_DEVICE_PRESENT    0x00000001

//*****************************************************************************
//
// Hold the current state for the application.
//
//*****************************************************************************
volatile enum
{
    //
    // No device is present.
    //
    STATE_NO_DEVICE,

    //
    // Mass storage device is being enumerated.
    //
    STATE_DEVICE_ENUM,

    //
    // Mass storage device is ready.
    //
    STATE_DEVICE_READY,

    //
    // An unsupported device has been attached.
    //
    STATE_UNKNOWN_DEVICE,

    //
    // A mass storage device was connected but failed to ever report ready.
    //
    STATE_TIMEOUT_DEVICE,

    //
    // A power fault has occurred.
    //
    STATE_POWER_FAULT
}
g_eState;

//*****************************************************************************
//
// The size of the host controller's memory pool in bytes.
//
//*****************************************************************************
#define HCD_MEMORY_SIZE         128

//*****************************************************************************
//
// The memory pool to provide to the Host controller driver.
//
//*****************************************************************************
uint8_t g_pHCDPool[HCD_MEMORY_SIZE];


//*****************************************************************************
//
// The instance data for the MSC driver.
//
//*****************************************************************************
tUSBHMSCInstance *g_psMSCInstance = 0;

//*****************************************************************************
//
// Declare the USB Events driver interface.
//
//*****************************************************************************
DECLARE_EVENT_DRIVER(g_sUSBEventDriver, 0, 0, USBHCDEvents);

//*****************************************************************************
//
// The global that holds all of the host drivers in use in the application.
// In this case, only the MSC class is loaded.
//
//*****************************************************************************
static tUSBHostClassDriver const * const g_ppHostClassDrivers[] =
{
    &g_sUSBHostMSCClassDriver,
    &g_sUSBEventDriver
};

//*****************************************************************************
//
// This global holds the number of class drivers in the g_ppHostClassDrivers
// list.
//
//*****************************************************************************
static const uint32_t g_ui32NumHostClassDrivers =
    sizeof(g_ppHostClassDrivers) / sizeof(tUSBHostClassDriver *);

//*****************************************************************************
//
// The control table used by the uDMA controller.  This table must be aligned
// to a 1024 byte boundary.  In this application uDMA is only used for USB,
// so only the first 6 channels are needed.
//
//*****************************************************************************
#if defined(ewarm)
#pragma data_alignment=1024
tDMAControlTable g_psDMAControlTable[6];
#elif defined(ccs)
#pragma DATA_ALIGN(g_psDMAControlTable, 1024)
tDMAControlTable g_psDMAControlTable[6];
#else
tDMAControlTable g_psDMAControlTable[6] __attribute__ ((aligned(1024)));
#endif

//*****************************************************************************
//
// Define a pair of buffers that are used for holding path information.
// The buffer size must be large enough to hold the longest expected
// full path name, including the file name, and a trailing null character.
// The initial path is set to root "/".
//
//*****************************************************************************
#define PATH_BUF_SIZE   80

//*****************************************************************************
//
// Define the maximum number of files that can appear at any directory level.
// This is used for allocating space for holding the file information.
// Define the maximum depth of subdirectories, also used to allocating space
// for directory structures.
// Define the maximum number of characters allowed to be stored for a file
// name.
//
//*****************************************************************************
#define MAX_FILES_PER_MENU 64
#define MAX_SUBDIR_DEPTH 32


//----------------------------------------
// Prototypes
//----------------------------------------
void hardware_init(void);
static bool FileInit(void);
void SysTickHandler(void);
static const char *StringFromFresult(FRESULT fresult);
static void MSCCallback(tUSBHMSCInstance *ps32Instance, uint32_t ui32Event, void *pvData);
static int printFileStructure (void);

//TODO: increment flash every read
//Flash presets
uint32_t address[] = {0x01, 0x00, 0x00};

//---------------------------------------------------------------------------
// main()
//---------------------------------------------------------------------------
int
Cmd_cat(int argc, char *argv[])
{
    FRESULT fresult;
    uint32_t ui32BytesRead;

    //
    // First, check to make sure that the current path (CWD), plus the file
    // name, plus a separator and trailing null, will all fit in the temporary
    // buffer that will be used to hold the file name.  The file name must be
    // fully specified, with path, to FatFs.
    //
//    if(strlen(g_pcCwdBuf) + strlen(argv[1]) + 1 + 1 > sizeof(g_pcTmpBuf))
//    {
//        UARTprintf("Resulting path name is too long\n");
//        return(0);
//    }
//
//    //
//    // Copy the current path to the temporary buffer so it can be manipulated.
//    //
//    strcpy(g_pcTmpBuf, g_pcCwdBuf);
//
//    //
//    // If not already at the root level, then append a separator.
//    //
//    if(strcmp("/", g_pcCwdBuf))
//    {
//        strcat(g_pcTmpBuf, "/");
//    }
//
//    //
//    // Now finally, append the file name to result in a fully specified file.
//    //
//    strcat(g_pcTmpBuf, argv[1]);

    //
    // Open the file for reading.
    //
   fresult = f_open(&g_sFileObject, g_pcTmpBuf, FA_READ);

    //
    // If there was some problem opening the file, then return an error.
    //
    if(fresult != FR_OK)
    {
        return((int)fresult);
    }

    //
    // Enter a loop to repeatedly read data from the file and display it, until
    // the end of the file is reached.
    //
    do
    {
        //
        // Read a block of data from the file.  Read as much as can fit in the
        // temporary buffer, including a space for the trailing null.
        //
        fresult = f_read(&g_sFileObject, g_pcTmpBuf, sizeof(g_pcTmpBuf) - 1,
                          (UINT *)&ui32BytesRead);

        //
        // If there was an error reading, then print a newline and return the
        // error to the user.
        //
        if(fresult != FR_OK)
        {
            UARTprintf("\n");
            return((int)fresult);
        }

        //
        // Null terminate the last block that was read to make it a null
        // terminated string that can be used with printf.
        //
        //TODO: get rid of for final version
        g_pcTmpBuf[ui32BytesRead] = 0;

        //UARTprintf("USB read: %s", g_pcTmpBuf);

        uint32_t dataRx[PATH_BUF_SIZE + 4];
        uint32_t dataTx[PATH_BUF_SIZE];
        volatile int i = 0;
        dataRx[4]=255;
        FLASHInit();
        //Clock speed used for testing
        //SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
        do{
            for(i=0;i < ui32BytesRead; i++){
                dataTx[i] = g_pcTmpBuf[i];
            }
            FLASHWriteEnable();
            FLASHEraseSector(address);
            while(FLASHIsBusy());
            FLASHWriteEnable();
            FLASHWriteAddress(address,dataTx,ui32BytesRead);
            while(FLASHIsBusy());
            FLASHReadAddress(address,dataRx,ui32BytesRead+4);
        }while(dataRx[4] == 255);

        char print_string[PATH_BUF_SIZE];
        for(i = 4; i< ui32BytesRead+4; i++){
            print_string[i-4] = dataRx[i];
        }

        //TODO: get rid of for final version
        print_string[ui32BytesRead] = 0;
        UARTprintf("Flash read: %s \n",print_string);

        //
        // Print the last chunk of the file that was received.
        //

    }
    while(ui32BytesRead == sizeof(g_pcTmpBuf) - 1);

    //
    // Return success.
    isRead = 1;
    //
    return(0);
}

tCmdLineEntry g_psCmdTable[] =
{

    { "cat",    Cmd_cat,    "Show contents of a text file" },
    { 0, 0, 0 }
};


void main(void)
{

    //Example Flash code for reference, do not delete
    int nStatus;


    char *fileName[] = {"POLYGO~1.GCO"};

    uint32_t ui32DriveTimeout, ui32SysClock;

    //
    // Set the main system clock to run from the PLL at 50MHz
    // Processor clock is calculated with (pll/2)/4 - > (400/2)/4 = 50
    // NOTE: For USB operation, it should be a minimum of 20MHz
    //

    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    //
    // Get the System Clock Rate
    // 50 MHz
    //
    ui32SysClock = SysCtlClockGet();

    // Enable USB0
    //
    // PB0 ---- USB ID ----> GND (Hardware)
    // PB1 ---- USB VBUS
    // PD4 ---- USB D-
    // PD5 ---- USB D+
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_USB0);
    SysCtlUSBPLLEnable();

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    GPIOPinTypeUSBAnalog(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    GPIOPinTypeUSBAnalog(GPIO_PORTD_BASE, GPIO_PIN_4 | GPIO_PIN_5);

    //
    // Initialize the USB stack for host mode only.
    //
    USBStackModeSet(0, eUSBModeForceHost, 0);

    //
    // Register the host class drivers.
    //
    USBHCDRegisterDrivers(0, g_ppHostClassDrivers, g_ui32NumHostClassDrivers);

    //
    // Configure SysTick for a 100Hz interrupt.
    // Systick Period = 5000000 / 100 -> 500000
    //
    SysTickPeriodSet(SysCtlClockGet() / TICKS_PER_SECOND);
    SysTickEnable();
    SysTickIntEnable();

    //
    // Enable the uDMA controller and set up the control table base.
    // The uDMA controller is used by the USB library.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
    uDMAEnable();
    uDMAControlBaseSet(g_psDMAControlTable);

    // Initialize UART0 (brought out to the console via the DEBUG USB port)
    // RX --- PA0
    // TX --- PA1
    // NOTE: Uses the UARTstdio utility
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTClockSourceSet(UART0_BASE, (UART_CLOCK_SYSTEM));
    UARTStdioConfig(0, 115200, SysCtlClockGet());
    IntEnable(INT_UART0);
    UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);

    // Enable all Interrupts.
    // IntMasterEnable();

    UARTprintf("Hardware Initialized\r\n");

    //
    // Initially wait for device connection.
    //
    g_eState = STATE_NO_DEVICE;

    //
    // Open an instance of the mass storage class driver.
    //
    g_psMSCInstance = USBHMSCDriveOpen(0, MSCCallback);

    //
    // Initialize the drive timeout.
    //
    ui32DriveTimeout = USBMSC_DRIVE_RETRY;

    //
    // Initialize the USB controller for host operation.
    //
    USBHCDInit(0, g_pHCDPool, HCD_MEMORY_SIZE);
//
//    //
//    // Initialize the fat file system.
//    //
   FileInit();
//
//    // Initialize Flash
//    FLASHInit();
//
//    UARTprintf("FAT File System Module Initialized\r\n");
//
//    //Example Flash code for reference, do not delete
//    //Write to flash
//    uint32_t dataRx[8] = {0,0,0,0,0,0,0,0};
//    uint32_t dataTx[4] = {'t','e','s','t'};
//    FLASHInit();
//    //Clock speed used for testing
//    //SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
//    FLASHWriteEnable();
//    FLASHEraseSector(address);
//    while(FLASHIsBusy());
//    FLASHWriteEnable();
//    FLASHWriteAddress(address,dataTx,4);
//    while(FLASHIsBusy());
//    FLASHReadAddress(address,dataRx,8);
//
//    char print_string[4];
//    uint32_t i = 0;
//    for(i = 4; i< 8; i++){
//        print_string[i-4] = dataRx[i];
//    }
//    UARTprintf("Flash read: %s \n",print_string);


    //
    // Enter an (almost) infinite loop for reading and processing commands from
    // the user.
    //
    while(1)
    {
        //
        // Call the USB stack to keep it running.
        //
        USBHCDMain();

        switch(g_eState)
        {
            case STATE_DEVICE_ENUM:
            {
                //
                // Take it easy on the Mass storage device if it is slow to
                // start up after connecting.
                //
                if(USBHMSCDriveReady(g_psMSCInstance) != 0)
                {
                    //
                    // Wait about 500ms before attempting to check if the
                    // device is ready again.
                    //
                    // 1 machine cycle takes (1/50*10^6) seconds
                    // SysCtlDelay uses 3 machine cycles, so it would be 3*(1/50*10^6) seconds
                    // Total Delay -> (Time Taken for 3 machine cycles) * Count Value
                    //
                    // Therefore, [(3/50*10^6) * (50*10^6/(3*2))] = 1/2 second
                    //
                    //
                    SysCtlDelay(ui32SysClock / (3 * 2));

                    //
                    // Decrement the retry count.
                    //
                    ui32DriveTimeout--;

                    //
                    // If the timeout is hit then go to the
                    // STATE_TIMEOUT_DEVICE state.
                    //
                    if(ui32DriveTimeout == 0)
                    {
                        g_eState = STATE_TIMEOUT_DEVICE;
                    }
                    break;
                }

                UARTprintf("USB Mass Storage Device Ready\r\n");

                g_pcCwdBuf[0] = '/';
                g_pcCwdBuf[1] = 0;

              //  nStatus = CmdLineProcess(g_pcCmdBuf);

                Cmd_cat(2, fileName);
                while(1);
                //
                // Getting here means the device is ready.
                // Reset the CWD to the root directory.
                //

                //
                // Fill the list box with the files and directories found.
                //
//                if(!printFileStructure())
//                {
//                    //
//                    // If there were no errors reported, we are ready for
//                    // MSC operation.
//                    //
//                    g_eState = STATE_DEVICE_READY;
//                }

                //
                // Set the Device Present flag.
                //
                g_ui32Flags = FLAGS_DEVICE_PRESENT;


                if (isRead == 1){
                                 break;
                    }

                                // Handle the case of bad command.
                                //
                                if(nStatus == CMDLINE_BAD_CMD)
                                {
                                    UARTprintf("Bad command!\n");
                                }

                                //
                                // Handle the case of too many arguments.
                                //
                                else if(nStatus == CMDLINE_TOO_MANY_ARGS)
                                {
                                    UARTprintf("Too many arguments for command processor!\n");
                                }

                                //
                                // Otherwise the command was executed.  Print the error code if one was
                                // returned.
                                //
                                else if(nStatus != 0)
                                {
                                    UARTprintf("Command returned error code %s\n",valueToSave);

                                }

                //break;
            }

            //
            // If there is no device then just wait for one.
            //
            case STATE_NO_DEVICE:
            {
                if(g_ui32Flags == FLAGS_DEVICE_PRESENT)
                {
                    //
                    // Clear the Device Present flag.
                    //
                    g_ui32Flags &= ~FLAGS_DEVICE_PRESENT;
                }
                break;
            }

            //
            // An unknown device was connected.
            //
            case STATE_UNKNOWN_DEVICE:
            {
                //
                // If this is a new device then change the status.
                //
                if((g_ui32Flags & FLAGS_DEVICE_PRESENT) == 0)
                {
                    // Indicate unknown device is present.
                    UARTprintf("Unknown Device was connected \r\n");
                }

                //
                // Set the Device Present flag.
                //
                g_ui32Flags = FLAGS_DEVICE_PRESENT;
                break;
            }

            //
            // The connected mass storage device is not reporting ready.
            //
            case STATE_TIMEOUT_DEVICE:
            {
                //
                // If this is the first time in this state then print a
                // message.
                //
                if((g_ui32Flags & FLAGS_DEVICE_PRESENT) == 0)
                {
                    // Indicate timeout when trying to connect
                    UARTprintf("Unknown device \r\n");

                }

                //
                // Set the Device Present flag.
                //
                g_ui32Flags = FLAGS_DEVICE_PRESENT;
                break;
            }

            //
            // Something has caused a power fault.
            //
            case STATE_POWER_FAULT:
            {
                break;
            }
            default:
            {
                break;
            }
        }



    }
}


//*****************************************************************************
//
// Initializes the file system module.
//
// \param None.
//
// This function initializes the third party FAT implementation.
//
// \return Returns \e true on success or \e false on failure.
//
//*****************************************************************************
static bool FileInit(void) {
    //
    // Mount the file system, using logical disk 0.
    //
    if(f_mount(0, &g_sFatFs) != FR_OK)
    {
        return(false);
    }
    return(true);
}

//*****************************************************************************
//
// This is the handler for this SysTick interrupt.  It simply increments a
// counter that is used for timing.
//
//*****************************************************************************
void SysTickHandler(void) {
    //
    // Update our tick counter.
    //
    g_ui32SysTickCount++;
}

//*****************************************************************************
//
// This function returns a string representation of an error code
// that was returned from a function call to FatFs.  It can be used
// for printing human readable error messages.
//
//*****************************************************************************
static const char * StringFromFresult(FRESULT fresult) {
    uint32_t ui32Idx;

    //
    // Enter a loop to search the error code table for a matching
    // error code.
    //
    for(ui32Idx = 0; ui32Idx < NUM_FRESULT_CODES; ui32Idx++)
    {
        //
        // If a match is found, then return the string name of the
        // error code.
        //
        if(g_sFresultStrings[ui32Idx].fresult == fresult)
        {
            return(g_sFresultStrings[ui32Idx].pcResultStr);
        }
    }

    //
    // At this point no matching code was found, so return a
    // string indicating unknown error.
    //
    return("UNKNOWN ERR");
}

//*****************************************************************************
//
// This is the callback from the MSC driver.
//
// \param ui32Instance is the driver instance which is needed when communicating
// with the driver.
// \param ui32Event is one of the events defined by the driver.
// \param pvData is a pointer to data passed into the initial call to register
// the callback.
//
// This function handles callback events from the MSC driver.  The only events
// currently handled are the MSC_EVENT_OPEN and MSC_EVENT_CLOSE.  This allows
// the main routine to know when an MSC device has been detected and
// enumerated and when an MSC device has been removed from the system.
//
// \return None
//
//*****************************************************************************
static void MSCCallback(tUSBHMSCInstance *ps32Instance, uint32_t ui32Event, void *pvData) {
    //
    // Determine the event.
    //
    switch(ui32Event)
    {
        //
        // Called when the device driver has successfully enumerated an MSC
        // device.
        //
        case MSC_EVENT_OPEN:
        {
            //
            // Proceed to the enumeration state.
            //
            g_eState = STATE_DEVICE_ENUM;

            break;
        }

        //
        // Called when the device driver has been unloaded due to error or
        // the device is no longer present.
        //
        case MSC_EVENT_CLOSE:
        {
            //
            // Go back to the "no device" state and wait for a new connection.
            //
            g_eState = STATE_NO_DEVICE;

            //
            // Re-initialize the file system.
            //
            FileInit();

            break;
        }

        default:
        {
            break;
        }
    }
}


//*****************************************************************************
//
// This is the generic callback from host stack.
//
// pvData is actually a pointer to a tEventInfo structure.
//
// This function will be called to inform the application when a USB event has
// occurred that is outside those related to the mass storage device.  At this
// point this is used to detect unsupported devices being inserted and removed.
// It is also used to inform the application when a power fault has occurred.
// This function is required when the g_USBGenericEventDriver is included in
// the host controller driver array that is passed in to the
// USBHCDRegisterDrivers() function.
//
//*****************************************************************************
void USBHCDEvents(void *pvData) {
    tEventInfo *pEventInfo;

    //
    // Cast this pointer to its actual type.
    //
    pEventInfo = (tEventInfo *)pvData;

    //
    // Process each kind of event
    //
    switch(pEventInfo->ui32Event)
    {
        //
        // An unknown device has been connected.
        //
        case USB_EVENT_UNKNOWN_CONNECTED:
        {
            //
            // An unknown device was detected.
            //
            g_eState = STATE_UNKNOWN_DEVICE;
            break;
        }

        //
        // The unknown device has been been unplugged.
        //
        case USB_EVENT_DISCONNECTED:
        {
            //
            // Unknown device has been removed.
            //
            g_eState = STATE_NO_DEVICE;
            break;
        }

        //
        // A bus power fault was detected.
        //
        case USB_EVENT_POWER_FAULT:
        {
            //
            // No power means no device is present.
            //
            g_eState = STATE_POWER_FAULT;
            break;
        }

        default:
        {
            break;
        }
    }
}


//*****************************************************************************
// Prints the file structure on UART.
//*****************************************************************************
static int printFileStructure (void) {

    uint32_t ui32ItemCount;
    FRESULT fresult;

    //
    // Open the current directory for access.
    //
    fresult = f_opendir(&g_sDirObject, g_pcCwdBuf);

    //
    // Check for error and return if there is a problem.
    //
    if(fresult != FR_OK)
    {
        //
        // Ensure that the error is reported.
        //
        UARTprintf("Error from USB disk:\r\n");
        UARTprintf((char *)StringFromFresult(fresult));
        UARTprintf("\r\n");
        return(fresult);
    }

    ui32ItemCount = 0;

    //
    // Enter loop to enumerate through all directory entries.
    //
    for(;;)
    {
        //
        // Read an entry from the directory.
        //
        fresult = f_readdir(&g_sDirObject, &g_sFileInfo);

        //
        // Check for error and return if there is a problem.
        //
        if(fresult != FR_OK)
        {
            UARTprintf("Error from USB disk:\r\n");
            UARTprintf((char *)StringFromFresult(fresult));
            UARTprintf("\r\n");
            return(fresult);
        }

        //
        // If the file name is blank, then this is the end of the
        // listing.
        //
        if(!g_sFileInfo.fname[0])
        {
            break;
        }

        //
        // Add the information on the console
        //
        if(ui32ItemCount < NUM_LIST_STRINGS)
        {
            usnprintf(g_pcFilenames[ui32ItemCount], MAX_FILENAME_STRING_LEN,
                      "(%c) %s", (g_sFileInfo.fattrib & AM_DIR) ? 'D' : 'F',
                      g_sFileInfo.fname);
            UARTprintf(g_pcFilenames[ui32ItemCount]);
            UARTprintf("\r\n");
        }

        //
        // Move to the next entry in the item array we use to populate the
        // list box.
        //
        ui32ItemCount++;
    }

    //
    // Made it to here, return with no errors.
    //
    return(0);
}

