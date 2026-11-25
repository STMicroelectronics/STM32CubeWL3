/**
  ******************************************************************************
  * @file    sfuota_frames.h
  * @author  GPAM Wireless Application Team
  * @brief   SFUOTA over-the-air frame format, shared by both peers of the link.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef SFUOTA_FRAMES_H
#define SFUOTA_FRAMES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Exported macros -----------------------------------------------------------*/
/* Frame layout: [HEADER(1)][PAYLOAD], multi-byte fields big-endian. */

/* Frame headers */
#define SFUOTA_HDR_CONNECTION  (0xA0U)  /*!< Connection handshake */
#define SFUOTA_HDR_SIZE        (0xB0U)  /*!< Image size exchange */
#define SFUOTA_HDR_START       (0xC0U)  /*!< Download start */
#define SFUOTA_HDR_DATAREQ     (0xD0U)  /*!< Chunk request, payload carries the sequence number */
#define SFUOTA_HDR_DATA        (0xE0U)  /*!< Data chunk and its acknowledgement */
#define SFUOTA_HDR_NOTSTART    (0xF0U)  /*!< Transfer refused */

/* Payload size of a data frame */
#define SFUOTA_FRAME_PAYLOAD                       (16U)  /*!< Image bytes carried by one data frame */

#ifdef __cplusplus
}
#endif

#endif /* SFUOTA_FRAMES_H */