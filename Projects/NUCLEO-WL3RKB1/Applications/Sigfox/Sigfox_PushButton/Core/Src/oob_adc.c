#include "main.h"
#include "oob_adc.h"

static ADC_HandleTypeDef hadc;

#define ADC_DATAWIDTH ADC_DS_DATA_WIDTH_16_BIT
#define ADC_DATARATIO ADC_DS_RATIO_128
#define ADC_SAMPLERATE LL_ADC_SAMPLE_RATE_16

void OOB_ADC_Init()
{
  __HAL_RCC_ADCDIG_CLK_ENABLE();
  __HAL_RCC_ADCANA_CLK_ENABLE();

  /* Enable ADC for battery voltage measurement */
  hadc.Instance = ADC1;
  hadc.Init.DownSamplerConfig.DataRatio = ADC_DATARATIO;
  hadc.Init.DownSamplerConfig.DataWidth = ADC_DATAWIDTH;
  hadc.Init.SequenceLength = 1;
  hadc.Init.SampleRate = LL_ADC_SAMPLE_RATE_80;
  hadc.Init.InvertOutputMode = ADC_DATA_INVERT_SING;
  hadc.Init.SamplingMode = ADC_SAMPLING_AT_END;
  HAL_ADC_Init(&hadc);
}

/* Called by device-specific Sigfox implementation, no need to call this yourself */
float APP_GetBatteryVoltageMillivolts(void)
{
  /* Switch to Battery Level ADC Channel */
  static ADC_ChannelConfTypeDef channel;
  channel.Channel = ADC_CHANNEL_VBAT;
  channel.Rank = ADC_RANK_1;
  channel.VoltRange = ADC_VIN_RANGE_3V6;
  if (HAL_ADC_ConfigChannel(&hadc, &channel) != HAL_OK)
    while (1) {}

  /* Obtain measurement and return result */
  HAL_ADC_Start(&hadc);
  HAL_ADC_PollForConversion(&hadc, 5);
  float millivolts = __LL_ADC_CALC_DATA_TO_VOLTAGE(LL_ADC_VIN_RANGE_3V6, HAL_ADC_GetValue(&hadc), ADC_DATAWIDTH);                     
  HAL_ADC_Stop(&hadc);

  return millivolts;
}

/* Called by device-specific Sigfox implementation, no need to call this yourself */
int16_t APP_GetTemperatureTenthCelsius(void)
{
  /* Switch to Temperature Level ADC Channel */
  static ADC_ChannelConfTypeDef channel;
  channel.Channel = ADC_CHANNEL_TEMPSENSOR;
  channel.Rank = ADC_RANK_1;
  channel.VoltRange = ADC_VIN_RANGE_1V2;
  if (HAL_ADC_ConfigChannel(&hadc, &channel) != HAL_OK)
    while (1) {};

  /* Obtain measurement and return result */
  HAL_ADC_Start(&hadc);
  HAL_ADC_PollForConversion(&hadc, 5);
  int16_t temperatureValue = __LL_ADC_CALC_TEMPERATURE(HAL_ADC_GetValue(&hadc), ADC_DATAWIDTH);                             
  HAL_ADC_Stop(&hadc);
      
  return temperatureValue;
}
