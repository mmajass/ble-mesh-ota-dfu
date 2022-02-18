/* Copyright (c) 2010 - 2018, Nordic Semiconductor ASA
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef SIMPLE_BEACON_COMMON_H__
#define SIMPLE_BEACON_COMMON_H__

#include <stdint.h>
#include "access.h"
#include "utils.h"

/**
 * @defgroup SIMPLE_BEACON_MODEL Simple Beacon model
 * This model implements the message based interface required to
 * set the 1 bit value on the server.
 *
 * Model Identification
 * @par
 * Company ID: @ref SIMPLE_BEACON_COMPANY_ID
 * @par
 * Simple Beacon Client Model ID: @ref SIMPLE_BEACON_CLIENT_MODEL_ID
 * @par
 * Simple Beacon Server Model ID: @ref SIMPLE_BEACON_SERVER_MODEL_ID
 *
 * List of supported messages:
 * @par
 * @copydoc SIMPLE_BEACON_OPCODE_SET
 * @par
 * @copydoc SIMPLE_BEACON_OPCODE_GET
 * @par
 * @copydoc SIMPLE_BEACON_OPCODE_SET_UNRELIABLE
 * @par
 * @copydoc SIMPLE_BEACON_OPCODE_STATUS
 *
 * @ingroup MESH_API_GROUP_VENDOR_MODELS
 * @{
 * @defgroup SIMPLE_BEACON_COMMON Common Simple Beacon definitions
 * Types and definitions shared between the two Simple Beacon models.
 * @{
 */

/*lint -align_max(push) -align_max(1) */

/** Vendor specific company ID for Simple Beacon model */
#define SIMPLE_BEACON_COMPANY_ID    (ACCESS_COMPANY_ID_NORDIC)

/** Simple Beacon opcodes. */
typedef enum
{
    SIMPLE_BEACON_OPCODE_SET = 0xC1,            /**< Simple Beacon Acknowledged Set. */
    SIMPLE_BEACON_OPCODE_GET = 0xC2,            /**< Simple Beacon Get. */
    SIMPLE_BEACON_OPCODE_SET_UNRELIABLE = 0xC3, /**< Simple Beacon Set Unreliable. */
    SIMPLE_BEACON_OPCODE_STATUS = 0xC4,          /**< Simple Beacon Status. */
    SIMPLE_BEACON_OPCODE_REPORT_STATUS = 0xC5
} simple_beacon_opcode_t;

/** Message format for the Simple Beacon Set message. */
typedef struct __attribute((packed))
{
    uint8_t report_enable; /**< State to set. */
} simple_beacon_msg_set_t;

/** Message format for th Simple Beacon Set Unreliable message. */
typedef struct __attribute((packed))
{
    uint8_t report_enable; /**< State to set. */
} simple_beacon_msg_set_unreliable_t;

/** Message format for th Simple Beacon Get message. */
typedef struct __attribute((packed))
{
    uint8_t report_enable; /**< State to get. */
} simple_beacon_msg_get_t;

typedef struct __attribute((packed))
{
    uint8_t report_enable; /**< State to get. */
} simple_beacon_msg_status_t;

/** Message format for the Simple Beacon Status message. */
typedef struct __attribute((packed))
{
    uint8_t custome_data[16];
} simple_beacon_msg_report_t;

/*lint -align_max(pop) */

/** @} end of SIMPLE_BEACON_COMMON */
/** @} end of SIMPLE_BEACON_MODEL */
#endif /* SIMPLE_BEACON_COMMON_H__ */
