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

#include <stdint.h>
#include <string.h>

/* HAL */
#include "boards.h"
#include "simple_hal.h"
#include "app_timer.h"

/* Core */
#include "nrf_mesh_configure.h"
#include "nrf_mesh.h"
#include "mesh_stack.h"
#include "device_state_manager.h"
#include "access_config.h"

/* Provisioning and configuration */
#include "mesh_provisionee.h"
#include "mesh_app_utils.h"
#include "mesh_softdevice_init.h"

/* Models */
#include "generic_onoff_server.h"

/* Logging and RTT */
#include "log.h"
#include "rtt_input.h"

/* Example specific includes */
#include "app_config.h"
#include "example_common.h"
#include "nrf_mesh_config_examples.h"
#include "light_switch_example_common.h"
#include "simple_beacon_server.h"

/* DFU module */
#include "nrf_mesh_dfu.h"
#include "nrf_mesh_events.h"

#define ONOFF_SERVER_0_LED          (BSP_LED_0)

static bool m_device_provisioned;
static bool m_beacon_report_enabled = 0;
static nrf_mesh_evt_handler_t m_evt_handler;
static simple_beacon_server_t m_beacon_server;

static bool simple_beacon_server_set_cb(const simple_beacon_server_t * p_self, bool beacon)
{
    m_beacon_report_enabled = beacon;
    hal_led_pin_set(LED_1, m_beacon_report_enabled);
    return m_beacon_report_enabled;
}

static bool simple_beacon_server_get_cb(const simple_beacon_server_t * p_self)
{
    return m_beacon_report_enabled;
}


/*************************************************************************************************/

static void app_model_init(void)
{
    /* Instantiate beacon server on element index 0 */
    m_beacon_server.set_cb = simple_beacon_server_set_cb;
    m_beacon_server.get_cb = simple_beacon_server_get_cb;
    ERROR_CHECK(simple_beacon_server_init(&m_beacon_server, 0));
    access_model_subscription_list_alloc(m_beacon_server.model_handle);
}

/*************************************************************************************************/
#if defined(NRF51)
    #define FLASH_PAGE_SIZE                 ( 0x400)
    #define FLASH_PAGE_MASK             (0xFFFFFC00)
#elif defined(NRF52_SERIES)
    #define FLASH_PAGE_SIZE                 (0x1000)
    #define FLASH_PAGE_MASK             (0xFFFFF000)
#endif

#if defined(_lint)
    const volatile uint32_t * rom_base   = NULL;
    const volatile uint32_t * rom_length = NULL;
    uint32_t rom_end;
    uint32_t bank_addr;
#elif defined ( __CC_ARM )
    extern uint32_t Image$$ER_IROM1$$Base;
    extern uint32_t Image$$ER_IROM1$$Length;
    const volatile uint32_t * rom_base   = &Image$$ER_IROM1$$Base;
    const volatile uint32_t * rom_length = &Image$$ER_IROM1$$Length;
    uint32_t rom_end;
    uint32_t bank_addr;
#elif defined   ( __GNUC__ )
    extern uint32_t _start;
    extern uint32_t __exidx_end;
    const volatile uint32_t rom_base   = (uint32_t) &_start;
    const volatile uint32_t rom_end    = (uint32_t) &__exidx_end;
    uint32_t rom_length;
    uint32_t bank_addr;
#endif

static bool fw_updated_event_is_for_me(const nrf_mesh_evt_dfu_t * p_evt)
{
    switch (p_evt->fw_outdated.transfer.dfu_type)
    {
        case NRF_MESH_DFU_TYPE_APPLICATION:
            return (p_evt->fw_outdated.current.application.app_id == p_evt->fw_outdated.transfer.id.application.app_id &&
                    p_evt->fw_outdated.current.application.company_id == p_evt->fw_outdated.transfer.id.application.company_id &&
                    p_evt->fw_outdated.current.application.app_version < p_evt->fw_outdated.transfer.id.application.app_version);

        case NRF_MESH_DFU_TYPE_BOOTLOADER:
            return (p_evt->fw_outdated.current.bootloader.bl_id == p_evt->fw_outdated.transfer.id.bootloader.bl_id &&
                    p_evt->fw_outdated.current.bootloader.bl_version < p_evt->fw_outdated.transfer.id.bootloader.bl_version);

        case NRF_MESH_DFU_TYPE_SOFTDEVICE:
            return false;

        default:
            return false;
    }
}

static void mesh_evt_handler(const nrf_mesh_evt_t* p_evt)
{
    switch (p_evt->type)
    {
        case NRF_MESH_EVT_DFU_FIRMWARE_OUTDATED:
        case NRF_MESH_EVT_DFU_FIRMWARE_OUTDATED_NO_AUTH:
            if (fw_updated_event_is_for_me(&p_evt->params.dfu))
            {
                ERROR_CHECK(nrf_mesh_dfu_request(p_evt->params.dfu.fw_outdated.transfer.dfu_type,
                                                 &p_evt->params.dfu.fw_outdated.transfer.id,
                                                 (uint32_t*) bank_addr));
                hal_led_mask_set(LEDS_MASK, false); /* Turn off all LEDs */
            }
            else
            {
                ERROR_CHECK(nrf_mesh_dfu_relay(p_evt->params.dfu.fw_outdated.transfer.dfu_type,
                                               &p_evt->params.dfu.fw_outdated.transfer.id));
            }
            break;

        case NRF_MESH_EVT_DFU_START:
            hal_led_mask_set(BSP_LED_0_MASK | BSP_LED_2_MASK, true);
            break;

        case NRF_MESH_EVT_DFU_END:
            hal_led_mask_set(LEDS_MASK, false); /* Turn off all LEDs */
            hal_led_mask_set(BSP_LED_0_MASK | BSP_LED_1_MASK, true); /* Yellow */
            break;

        case NRF_MESH_EVT_DFU_BANK_AVAILABLE:
            hal_led_mask_set(LEDS_MASK, false); /* Turn off all LEDs */
            ERROR_CHECK(nrf_mesh_dfu_bank_flash(p_evt->params.dfu.bank.transfer.dfu_type));
            break;

        default:
            break;

    }
}

static void node_reset(void)
{
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "----- Node reset  -----\n");
    hal_led_blink_ms(LEDS_MASK, LED_BLINK_INTERVAL_MS, LED_BLINK_CNT_RESET);
    /* This function may return if there are ongoing flash operations. */
    mesh_stack_device_reset();
}

static void config_server_evt_cb(const config_server_evt_t * p_evt)
{
    if (p_evt->type == CONFIG_SERVER_EVT_NODE_RESET)
    {
        node_reset();
    }
}

static void button_event_handler(uint32_t button_number)
{
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Button %u pressed\n", button_number);
    switch (button_number)
    {
        /* Pressing SW1 on the Development Kit will result in LED state to toggle and trigger
        the STATUS message to inform client about the state change. This is a demonstration of
        state change publication due to local event. */
        case 0:
        {
            uint8_t test_report[16] = "Hello world !!!";
            simple_beacon_server_report_publish(&m_beacon_server, test_report);
            break;
        }

        /* Initiate node reset */
        case 3:
        {
            /* Clear all the states to reset the node. */
            mesh_stack_config_clear();
            node_reset();
            break;
        }

        default:
            break;
    }
}


static void app_rtt_input_handler(int key)
{
    if (key >= '0' && key <= '4')
    {
        uint32_t button_number = key - '0';
        button_event_handler(button_number);
    }
}

static void provisioning_complete_cb(void)
{
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Successfully provisioned\n");

    dsm_local_unicast_address_t node_address;
    dsm_local_unicast_addresses_get(&node_address);
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Node Address: 0x%04x \n", node_address.address_start);

    hal_led_mask_set(LEDS_MASK, LED_MASK_STATE_OFF);
    hal_led_blink_ms(LEDS_MASK, LED_BLINK_INTERVAL_MS, LED_BLINK_CNT_PROV);
}

static void models_init_cb(void)
{
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Initializing and adding models\n");
    app_model_init();
}

static void mesh_init(void)
{
    uint8_t dev_uuid[NRF_MESH_UUID_SIZE];
    uint8_t node_uuid_prefix[NODE_UUID_PREFIX_LEN] = SERVER_NODE_UUID_PREFIX;

    ERROR_CHECK(mesh_app_uuid_gen(dev_uuid, node_uuid_prefix, NODE_UUID_PREFIX_LEN));
    mesh_stack_init_params_t init_params =
    {
        .core.irq_priority       = NRF_MESH_IRQ_PRIORITY_LOWEST,
        .core.lfclksrc           = DEV_BOARD_LF_CLK_CFG,
        .core.p_uuid             = dev_uuid,
        .models.models_init_cb   = models_init_cb,
        .models.config_server_cb = config_server_evt_cb
    };
    ERROR_CHECK(mesh_stack_init(&init_params, &m_device_provisioned));
}

static void initialize(void)
{
    __LOG_INIT(LOG_SRC_APP | LOG_SRC_ACCESS, LOG_LEVEL_INFO, LOG_CALLBACK_DEFAULT);
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "----- Digibale Demo with DFU v1-----\n");


#if defined ( __CC_ARM )
    rom_end    = (uint32_t) rom_base + (uint32_t) rom_length;
#elif defined   ( __GNUC__ )
    rom_length = (uint32_t) rom_end - rom_base;
#endif
    /* Take the next available page address */
    bank_addr  = (uint32_t) (rom_end & FLASH_PAGE_MASK) + FLASH_PAGE_SIZE;
    __LOG(LOG_SRC_APP, LOG_LEVEL_DBG2, "rom_base   %X\n", rom_base);
    __LOG(LOG_SRC_APP, LOG_LEVEL_DBG2, "rom_end    %X\n", rom_end);
    __LOG(LOG_SRC_APP, LOG_LEVEL_DBG2, "rom_length %X\n", rom_length);
    __LOG(LOG_SRC_APP, LOG_LEVEL_DBG2, "bank_addr   %X\n", bank_addr);

    ERROR_CHECK(app_timer_init());
    hal_leds_init();

#if BUTTON_BOARD
    ERROR_CHECK(hal_buttons_init(button_event_handler));
#endif

    nrf_clock_lf_cfg_t lfc_cfg = DEV_BOARD_LF_CLK_CFG;
    ERROR_CHECK(mesh_softdevice_init(lfc_cfg));
    mesh_init();
    m_evt_handler.evt_cb = mesh_evt_handler;
    nrf_mesh_evt_handler_add(&m_evt_handler);

}

static void start(void)
{
    rtt_input_enable(app_rtt_input_handler, RTT_INPUT_POLL_PERIOD_MS);
    ERROR_CHECK(mesh_stack_start());

    if (!m_device_provisioned)
    {
        static const uint8_t static_auth_data[NRF_MESH_KEY_SIZE] = STATIC_AUTH_DATA;
        mesh_provisionee_start_params_t prov_start_params =
        {
            .p_static_data    = static_auth_data,
            .prov_complete_cb = provisioning_complete_cb,
            .p_device_uri = NULL
        };
        ERROR_CHECK(mesh_provisionee_prov_start(&prov_start_params));
    }

    const uint8_t *p_uuid = nrf_mesh_configure_device_uuid_get();
    __LOG_XB(LOG_SRC_APP, LOG_LEVEL_INFO, "Device UUID ", p_uuid, NRF_MESH_UUID_SIZE);

    hal_led_mask_set(LEDS_MASK, LED_MASK_STATE_OFF);
    hal_led_blink_ms(LEDS_MASK, LED_BLINK_INTERVAL_MS, LED_BLINK_CNT_START);
}

int main(void)
{
    initialize();
    execution_start(start);

    for (;;)
    {
        (void)sd_app_evt_wait();
    }
}
