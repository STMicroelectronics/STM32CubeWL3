#ifndef STM32WL3_CLI_COMMANDS_H
#define STM32WL3_CLI_COMMANDS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "binary_iqsampling.h"
#include "binary_iqsampling_spi.h"
#include "stm32wl3x_hal.h"
#include "command-interpreter2.h"
#include "serial_utils.h"
#include "timer_utils.h"


#define STM32WL3_COMMAND_TABLE \
{ "help",		helpAction,		"",		"List commands"}, \
{ "reboot",		rebootAction,		"",		"Reboot device"}, \
{ "interactive",	interactiveAction,	"u",		"Set interactive mode"}, \
{ "fwVersion",		fwVersionAction,	"",		"Get fw version"}, \
{ "ReadRegister",	ReadRegisterAction,	"w",		"Read register"}, \
{ "WriteRegister",	WriteRegisterAction,	"ww",		"Write register"}, \
{ "CliGetTimer",	CliGetTimerAction,      "",   	        "Read current timer (us)"}, \
{ "CliResetTimer",	CliResetTimerAction,    "",   	        "Reset timer"}, \
{ "MemAllocInit", 	MemAllocInitAction, 	"",		"Initilalize dynamic memory"}, \
{ "MemAlloc", 		MemAllocAction, 	"wu",		"Allocate memory"}, \
{ "MemDealloc", 	MemDeallocAction, 	"w",		"Deallocate memory"}, \
{ "RxPackets", 		RxPacketsAction,	"ww",		"Reception in packet mode for defined amount of time"}, \
{ "RxPacketsSum", 	RxPacketsSumAction,	"wwb",		"Reception summary in packet mode for defined amount of time"}, \
{ "TxPackets", 		TxPacketsAction,	"wwb",	        "Transmission in packet mode for defined amount of time"}, \
{ "RxData", 		RxDataAction,		"",		"Receive data until almost full is raised"}, \
{ "RssiLive", 		RssiLiveAction,		"w",		"Read and print RSSI_LEVEL_RUN field every interval ms"}, \
{ "IQDump", 		IQDumpAction,		"ww",		"Dump of the Raw I/Q sampling taken at the output of the Channel filter inside the demodulator"}, \
{ "SoftSymDump", 	SoftSymDumpAction,	"ww",		"Dump of the Raw data taken at the output of the post-filter inside the demodulator"}, \
{ "FreqDetectDump", 	FreqDetectDumpAction,	"ww",		"Dump of the Raw data taken at the output of the frequency detector inside the demodulator"}, \
{ "DirectBufferDump", 	DirectBufferDumpAction,	"",		"Dump the Full bit stream stored into the RAM buffers"}, \
{ "WakeupRx", 		WakeupRxAction,		"",		"Activate wakeup radio and dumps received packets"}, \
{ "wfi", 		wfiAction,		"u",		"Set WFI mode: 0 exit from wfi, 1 wfi, 2, wfi with usart disabled"}, \
{ "disableUsart", 	disableUsartAction,	"",		"Disable USART"}

extern uint8_t responsePrintf(const char * formatString, ...);
extern uint8_t responseHeaderPrintf(const char * formatString, ...);

/* Commands Interface */
void helpAction(void);
void rebootAction(void);
void interactiveAction(void);
void fwVersionAction(void);
void ReadRegisterAction(void);
void WriteRegisterAction(void);
void CliGetTimerAction(void);
void CliResetTimerAction(void);
void MemAllocAction(void);
void MemAllocInitAction(void);
void MemDeallocAction(void);
void RxDataAction (void);
void RxPacketsAction (void);
void RxPacketsSumAction (void);
void TxPacketsAction(void);
void wfiAction(void);
void disableUsartAction(void);
void RssiLiveAction(void);
void IQDumpAction(void);
void SoftSymDumpAction(void);
void FreqDetectDumpAction(void);
void DirectBufferDumpAction(void);
void WakeupRxAction(void);

/* Prototypes */
uint8_t checkStop(void);

#ifdef __cplusplus
}
#endif

#endif
