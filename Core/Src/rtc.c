/**
  ******************************************************************************
  * File Name          : RTC.c
  * Description        : This file provides code for the configuration
  *                      of the RTC instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "rtc.h"

/* USER CODE BEGIN 0 */
extern RTC_DateTypeDef RtcDate;
extern RTC_TimeTypeDef RtcTime;
uint32_t CalculateDayNumber(uint8_t Date, uint8_t Month, uint8_t Year); // DD.MM.YY
void CalculateDateFromDayNumber(uint32_t DayNumber, uint8_t *Date, uint8_t *Month, uint8_t *Year);
/* USER CODE END 0 */

RTC_HandleTypeDef hrtc;

/* RTC init function */
void MX_RTC_Init(void)
{
  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef DateToUpdate = {0};

  /** Initialize RTC Only 
  */
  hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_ALARM;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  RTC_DateTypeDef BackupDate;

  RtcDate.Date = (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR2) >> 8);
  RtcDate.Month = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR2);
  RtcDate.Year = (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR3) >> 8);
  RtcDate.WeekDay = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR3);

  HAL_RTC_GetTime(&hrtc, &RtcTime, RTC_FORMAT_BIN); // There is also an internal date update based on HW RTC time elapsed!!
  HAL_RTC_GetDate(&hrtc, &BackupDate, RTC_FORMAT_BIN); // Days elapsed since MCU power down

  uint32_t BackupDateDays = CalculateDayNumber(BackupDate.Date, BackupDate.Month, BackupDate.Year);
  uint32_t RtcDateDays = CalculateDayNumber(RtcDate.Date, RtcDate.Month, RtcDate.Year);

  RtcDateDays += (BackupDateDays - CalculateDayNumber(1, 1, 0));

  CalculateDateFromDayNumber(RtcDateDays, &RtcDate.Date, &RtcDate.Month, &RtcDate.Year);

  HAL_RTC_SetDate(&hrtc, &RtcDate, RTC_FORMAT_BIN);

  //
  //	You have to do not let Init code to reinitialize Time and Date in RTC.
  //	It's "a bug" in HAL widely described and complain on forums.
  //	That causes every MCU restart the time and date will be same as you configured in CubeMX.
  //	All you have to do is just return before init time/date in this user section.
  //
    return;
  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date 
  */
  sTime.Hours = 23;
  sTime.Minutes = 59;
  sTime.Seconds = 55;

  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  DateToUpdate.WeekDay = RTC_WEEKDAY_SATURDAY;
  DateToUpdate.Month = RTC_MONTH_FEBRUARY;
  DateToUpdate.Date = 3;
  DateToUpdate.Year = 20;

  if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }

}

void HAL_RTC_MspInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspInit 0 */

  /* USER CODE END RTC_MspInit 0 */
    HAL_PWR_EnableBkUpAccess();
    /* Enable BKP CLK enable for backup registers */
    __HAL_RCC_BKP_CLK_ENABLE();
    /* RTC clock enable */
    __HAL_RCC_RTC_ENABLE();
  /* USER CODE BEGIN RTC_MspInit 1 */

  /* USER CODE END RTC_MspInit 1 */
  }
}

void HAL_RTC_MspDeInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspDeInit 0 */

  /* USER CODE END RTC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_RTC_DISABLE();
  /* USER CODE BEGIN RTC_MspDeInit 1 */

  /* USER CODE END RTC_MspDeInit 1 */
  }
} 

/* USER CODE BEGIN 1 */
uint32_t CalculateDayNumber(uint8_t Date, uint8_t Month, uint8_t Year)
{
	// Format:
	// DD.MM.YY
	uint32_t _Year = Year + 20;
	Month = (Month + 9) % 12;
	_Year = _Year - (Month / 10);
	return ((365 * _Year) + (_Year / 4) - (_Year / 100) + (_Year / 400) + (((Month * 306) + 5) / 10) + (Date - 1));
}

void CalculateDateFromDayNumber(uint32_t DayNumber, uint8_t *Date, uint8_t *Month, uint8_t *Year)
{
	uint32_t _Date, _Month, _Year;
	_Year = ((10000 * DayNumber) + 14780) / 3652425;
	int32_t ddd = DayNumber - ((365 * _Year) + (_Year / 4) - (_Year / 100) + (_Year / 400));
	if (ddd < 0)
	{
		_Year -= 1;
		ddd = DayNumber - ((365 * _Year) + (_Year / 4) - (_Year / 100) + (_Year / 400));
	}
	int32_t mi = ((100 * ddd) + 52) / 3060;
	_Month = (mi + 2) % 12 + 1;
	_Year = _Year + (mi + 2)/12;
	_Date = ddd - ((mi * 306) + 5)/10 + 1;
	*Date = _Date;
	*Month = _Month;
	*Year = _Year - 20;
}
/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
