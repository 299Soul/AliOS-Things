/*
 * Copyright (c) 2021 Bestechnic (Shanghai) Co., Ltd. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _USB_HCD_TEST
#define _USB_HCD_TEST
#ifdef __cplusplus
extern "C" {
#endif

int test_usb_hcd_main(void);

void test_usb_hcd_loop(void);

void test_usb_hcd_get_interrupt_status(void);

void test_usb_hcd_get_port_status(void);

void test_usb_hcd_get_plug_status(void);

int test_usb_hcd_cmd_get_port_status(void);

void test_usb_hcd_cmd_create_point(uint8_t device,
                                                 uint8_t port,
                                                 uint8_t attributes,
                                                 uint8_t interval,
                                                 uint8_t max_packet_size);
void test_usb_hcd_cmd_destroy_point(void);
void test_usb_hcd_cmd_disable_controller(void);
void test_usb_hcd_cmd_enable_port(void);
void test_usb_hcd_cmd_disable_port(void);
void test_usb_hcd_cmd_power_on_port(void);
void test_usb_hcd_cmd_power_off_port(void);
void test_usb_hcd_cmd_resume_port(void);
void test_usb_hcd_cmd_suspend_port(void);
void test_usb_hcd_cmd_reset_port(void);
void test_usb_hcd_cmd_get_frame_number_port(void);
void test_usb_hcd_cmd_set_frame_number_port(uint32_t frame_number);
void test_usb_hcd_cmd_transfer_req(void);
void test_usb_hcd_cmd_set_addr(void);
void test_usb_hcd_cmd_get_descriptor(int descriptor_type);

void test_usb_hcd_cmd_set_config(void);

void test_usb_set_itf_ep(void);

int  test_usb_bulk_tracefer_req(int device, int ep, uint8_t* data, int length, HCD_TRANSFER_CALLBACK callback, void* userdata);

void test_usb_hcd_cmd_process_done_queue(void);
#ifdef __cplusplus
}
#endif
#endif
