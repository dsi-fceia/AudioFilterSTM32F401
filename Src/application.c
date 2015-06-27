/**
  ******************************************************************************
  * @file    audioFilter.c 
  * @author  Gustavo Muro
  * @version V0.0.1
  * @date    30/05/2015
  * @brief   Filtrado de audio.
  ******************************************************************************
  * @attention
  *
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted provided that the following conditions are met:
  *
  * 1. Redistributions of source code must retain the above copyright notice,
  *    this list of conditions and the following disclaimer.
  *
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  *
  * 3. Neither the name of the copyright holder nor the names of its
  *    contributors may be used to endorse or promote products derived from this
  *    software without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  * POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "application.h"
#include "ff.h"
#include "main.h"
#include "waveplayer.h"
#include "waverecorder.h"

/* Private typedef -----------------------------------------------------------*/
typedef enum
{
  APPSTATE_IDLE = 0,
  APPSTATE_MOUNT_FS,
  APPSTATE_UMOUNT_FS,
	APPSTATE_FILTER_AND_WRITE,
  APPSTATE_PLAY,
}appState_enum;

/* Private define ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static FATFS USBDISKFatFs;           /* File system object for USB disk logical drive */
static char USBDISKPath[4];          /* USB Host logical drive path */
static appState_enum appState = APPSTATE_IDLE;

/* Variable used by FatFs*/
static FIL FileRead;
static WAVE_FormatTypeDef waveformat;
static UINT bytesread = 0;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
int32_t getDataCB(int16_t *pBuff, int32_t length)
{
  UINT bytesread = 0;
  
  f_read(&FileRead, 
    pBuff, 
    length*2,
    (void *)&bytesread); 

  return bytesread;
}
/* Exported functions ------------------------------------------------------- */

extern void application_init(void)
{
  /*##-1- Link the USB Host disk I/O driver ##################################*/
  if(FATFS_LinkDriver(&USBH_Driver, USBDISKPath) != 0)
  {
    Error_Handler();
  }
}

extern void application_task(void)
{
  switch (appState)
  {
    case APPSTATE_IDLE:
      break;
    
    case APPSTATE_MOUNT_FS:
      if (f_mount(&USBDISKFatFs, (TCHAR const*)USBDISKPath, 0 ) != FR_OK ) 
      {
        /* FatFs initialization fails */
        Error_Handler();
      }
      else
      {
        appState = APPSTATE_FILTER_AND_WRITE;
      }
      break;
    
    case APPSTATE_UMOUNT_FS:
      f_mount(NULL, (TCHAR const*)"", 0);
      appState = APPSTATE_IDLE;
      break;
    
		case APPSTATE_FILTER_AND_WRITE:
			if (f_open(&FileRead, WAVE_NAME_COMPLETO, FA_READ) != FR_OK)
      {
        Error_Handler();
      }
      else
      {
        /* Read sizeof(WaveFormat) from the selected file */
        f_read (&FileRead, &waveformat, sizeof(waveformat), &bytesread);

        WaveRecord(waveformat, getDataCB);

        f_close(&FileRead);
      }
			appState = APPSTATE_PLAY;
			break;

    case APPSTATE_PLAY:
      if (f_open(&FileRead, WAVE_NAME_COMPLETO, FA_READ) != FR_OK)
      {
        Error_Handler();
      }
      else
      {    
        /* Read sizeof(WaveFormat) from the selected file */
        f_read (&FileRead, &waveformat, sizeof(waveformat), &bytesread);
        
        WavePlayerStart(waveformat, getDataCB, 70);
        
        f_close(&FileRead);
      }
      break;
    
    default:
      appState = APPSTATE_IDLE;
      break;
  }
}

extern void application_conect(void)
{
  appState = APPSTATE_MOUNT_FS;
}
extern void application_disconect(void)
{
  appState = APPSTATE_UMOUNT_FS;
}

/* End of file ---------------------------------------------------------------*/

