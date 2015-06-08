/**
  ******************************************************************************
  * @file    Audio_playback_and_record/Src/waveplayer.c 
  * @author  MCD Application Team
  * @version V1.2.2
  * @date    25-May-2015
  * @brief   I2S Audio player program. 
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "audioFilter.h"

/** @addtogroup STM32F4-Discovery_Audio_Player_Recorder
* @{
*/ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

#define AUDIO_BUFFER_SIZE             4096

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* LED State (Toggle or OFF)*/
__IO uint32_t LEDsState;

/* Audio wave data length to be played */
static uint32_t WaveDataLength = 0;

/* Audio wave remaining data length to be played */
static __IO uint32_t AudioRemSize = 0;

/* Ping-Pong buffer used for audio play */
uint8_t Audio_Buffer[AUDIO_BUFFER_SIZE];

/* Position in the audio play buffer */
__IO BUFFER_StateTypeDef BufferOffset = BUFFER_OFFSET_NONE;

/* Initial Volume level (from 0 (Mute) to 100 (Max)) */
static uint8_t Volume = 70;

/* Variable used by FatFs*/
FIL FileRead;
DIR Directory;

static uint8_t filterState = 1;

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

static void convertToStereo(int16_t *src, int16_t *dest, int32_t lengthSrc)
{
	int32_t i;
	
	for (i = lengthSrc-1 ; i >= 0 ; i--)
	{
		dest[i*2] = src[i];
		dest[i*2+1] = src[i];
	}
}

/**
  * @brief  Stops playing Wave.
  * @param  None
  * @retval None
  */
static void WavePlayerStop(void)
{ 
  BSP_AUDIO_OUT_Stop(CODEC_PDWN_HW);
}


/* Exported functions ------------------------------------------------------- */
/**
  * @brief  Plays Wave from a mass storage.
  * @param  AudioFreq: Audio Sampling Frequency
  * @retval None
*/
void WavePlayBack(uint32_t AudioFreq)
{ 
	/* tipo de dato UINT para mantener compatibilidad con FatFs */
  UINT bytesread = 0;
  
  /* Initialize Wave player (Codec, DMA, I2C) */
  if(WavePlayerInit(AudioFreq) != 0)
  {
    Error_Handler();
  }
  
  /* Start playing Wave */
  BSP_AUDIO_OUT_Play((uint16_t*)&Audio_Buffer[0], AUDIO_BUFFER_SIZE);
  LEDsState = LED6_TOGGLE;
    
	audioFilter_init();
	
	if (filterState)
	{
		audioFilter_filterOn();
		filterState = 0;
	}
	else
	{
		audioFilter_filterOff();
		filterState = 1;
	}
	
	/* Toggling LED6 to signal Play */
	LEDsState = LED6_TOGGLE;
	/* Resume playing Wave */
	WavePlayerPauseResume(RESUME_STATUS);
	
  /* Check if the device is connected.*/
  while (AudioRemSize != 0)
  { 
		bytesread = 0;
		
		if (BufferOffset == BUFFER_OFFSET_HALF)
		{
			f_read(&FileRead, 
						 &Audio_Buffer[0], 
						 AUDIO_BUFFER_SIZE/4, 
						 (void *)&bytesread); 
			
			audioFilter_filter(
				(q15_t*)&Audio_Buffer[0], 
				(q15_t*)&Audio_Buffer[0], 
				AUDIO_BUFFER_SIZE/8);
							
			convertToStereo(
				(int16_t *)&Audio_Buffer[0],
				(int16_t *)&Audio_Buffer[0],
				AUDIO_BUFFER_SIZE/8);
		
			BufferOffset = BUFFER_OFFSET_NONE;
		}
		
		if(BufferOffset == BUFFER_OFFSET_FULL)
		{
			f_read(&FileRead, 
						 &Audio_Buffer[AUDIO_BUFFER_SIZE/2], 
						 AUDIO_BUFFER_SIZE/4,
						 (void *)&bytesread); 

			audioFilter_filter(
				(q15_t*)&Audio_Buffer[AUDIO_BUFFER_SIZE/2], 
				(q15_t*)&Audio_Buffer[AUDIO_BUFFER_SIZE/2], 
				AUDIO_BUFFER_SIZE/8);
			
			convertToStereo(
				(int16_t *)&Audio_Buffer[AUDIO_BUFFER_SIZE/2],
				(int16_t *)&Audio_Buffer[AUDIO_BUFFER_SIZE/2],
				AUDIO_BUFFER_SIZE/8);
			
			BufferOffset = BUFFER_OFFSET_NONE;
		} 
		if(AudioRemSize > (AUDIO_BUFFER_SIZE / 2))
		{
			AudioRemSize -= bytesread;
		}
		else
		{
			AudioRemSize = 0;
		}
  }
  LEDsState = LEDS_OFF;
  
	/* Stop playing Wave */
  WavePlayerStop();
  f_close(&FileRead);
}

/**
  * @brief  Pauses or Resumes a played Wave.
  * @param  state: Player state: Pause, Resume or Idle
  * @retval None
  */
void WavePlayerPauseResume(uint32_t wState)
{ 
  if(wState == PAUSE_STATUS)
  {
    BSP_AUDIO_OUT_Pause();   
  }
  else
  {
    BSP_AUDIO_OUT_Resume();   
  }
}
 
/**
  * @brief  Initializes the Wave player.
  * @param  AudioFreq: Audio sampling frequency
  * @retval None
  */
int WavePlayerInit(uint32_t AudioFreq)
{ 
  /* Initialize the Audio codec and all related peripherals (I2S, I2C, IOExpander, IOs...) */  
  return(BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_AUTO, Volume, AudioFreq));  
}

/*--------------------------------
Callbacks implementation:
The callbacks prototypes are defined in the stm32f4_discovery_audio_codec.h file
and their implementation should be done in the user code if they are needed.
Below some examples of callback implementations.
--------------------------------------------------------*/

/**
  * @brief  Manages the DMA Half Transfer complete interrupt.
  * @param  None
  * @retval None
  */
void BSP_AUDIO_OUT_HalfTransfer_CallBack(void)
{ 
  BufferOffset = BUFFER_OFFSET_HALF;
}

/**
* @brief  Calculates the remaining file size and new position of the pointer.
* @param  None
* @retval None
*/
void BSP_AUDIO_OUT_TransferComplete_CallBack(void)
{
  BufferOffset = BUFFER_OFFSET_FULL;
  BSP_AUDIO_OUT_ChangeBuffer((uint16_t*)&Audio_Buffer[0], AUDIO_BUFFER_SIZE /2);
}

/**
* @brief  Manages the DMA FIFO error interrupt.
* @param  None
* @retval None
*/
void BSP_AUDIO_OUT_Error_CallBack(void)
{
  /* Stop the program with an infinite loop */
  while (1)
  {}
  
  /* Could also generate a system reset to recover from the error */
  /* .... */
}

/**
  * @brief  Starts Wave player.
  * @param  None
  * @retval None
  */
void WavePlayerStart(void)
{
  UINT bytesread = 0;
  char path[] = "0:/";
  char* wavefilename = NULL;
  WAVE_FormatTypeDef waveformat;
  
  /* Get the read out protection status */
  if(f_opendir(&Directory, path) == FR_OK)
  {
    wavefilename = WAVE_NAME; 
    
		/* Open the Wave file to be played */
    if(f_open(&FileRead, wavefilename , FA_READ) != FR_OK)
    {
      Error_Handler();
    }
    else
    {    
      /* Read sizeof(WaveFormat) from the selected file */
      f_read (&FileRead, &waveformat, sizeof(waveformat), &bytesread);
      
      /* Set WaveDataLenght to the Speech Wave length */
      WaveDataLength = waveformat.FileSize;
    
			AudioRemSize = WaveDataLength - bytesread;
			
      /* Play the Wave */
      WavePlayBack(waveformat.SampleRate);
    }    
  }
}

/**
* @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
