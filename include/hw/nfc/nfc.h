/* Copyright (C) 2013 Mozilla Foundation
**
** This software is licensed under the terms of the GNU General Public
** License version 2, as published by the Free Software Foundation, and
** may be copied, distributed, and modified under those terms.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
*/

#ifndef nfc_h
#define nfc_h

#include <stddef.h>
#include <stdint.h>
#include "hw/nfc/rf.h"
#include "qemu-common.h"

struct nfc_re;
union nci_packet;

enum {
    NUMBER_OF_SUPPORTED_NCI_RF_INTERFACES = 8
};

enum nfc_fsm_state {
    NFC_FSM_STATE_IDLE = 0,
    NFC_FSM_STATE_RESET,
    NFC_FSM_STATE_INITIALIZED,
    NUMBER_OF_NFC_FSM_STATES
};

/* the buffer-type for the delivery callback */
enum nfc_buf_type {
    NO_BUF = 0,
    NTFN_BUF,
    DATA_BUF
};

struct nfc_device {
    enum nfc_fsm_state state;
    enum nfc_rfst rf_state;

    /* Support Frame and NFC-DEP interface */
    struct nfc_rf rf[NUMBER_OF_SUPPORTED_NCI_RF_INTERFACES];
    uint8_t id;

    struct nfc_re* active_re;
    struct nfc_rf* active_rf;

    /* stores all config options */
    uint8_t config_id_value[128];
};

/* supplied to process_{nci,hci}_message */
struct nfc_delivery_cb {
    enum nfc_buf_type type;
    void* data;
    ssize_t (*func)(void* /*user_data*/, union nci_packet*);
};

void
nfc_device_init(struct nfc_device* nfc);

void
nfc_device_set(struct nfc_device* nfc, size_t off, size_t len,
               const void* value);

void
nfc_device_get(const struct nfc_device* nfc, size_t off, size_t len,
               void* value);

uint8_t
nfc_device_incr_id(struct nfc_device* nfc);

struct nfc_rf*
nfc_find_rf_by_protocol_and_mode(struct nfc_device* nfc,
                                 enum nci_rf_protocol proto, enum nci_rf_tech_mode mode);

void
nfc_delivery_cb_setup(struct nfc_delivery_cb* cb, enum nfc_buf_type type,
                      void* data, ssize_t (*func)(void*, union nci_packet*));

#endif
