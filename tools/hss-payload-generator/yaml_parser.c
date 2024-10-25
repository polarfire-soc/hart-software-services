/******************************************************************************************
 *
 * MPFS HSS Embedded Software - tools/hss-payload-generator
 *
 * Copyright 2020-2024 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>

#include <yaml.h>
#include "yaml_parser.h"
#include "elf_parser.h"
#include "blob_handler.h"
#include "generate_payload.h"

#include "debug_printf.h"


#define ARRAY_SIZE(x)		(sizeof(x)/sizeof(x[0]))

/*
 * Sample config file
 *
 * 	set-name: 'PolarFire-SoC-HSS::TestImage'
 * 	hart-entry-points: {u54_1: '0x80200000', u54_2: '0x80200000', u54_3: '0xB0000000', u54_4: '0x80200000'}
 * 	payloads:
 *   	test/baremetal: {exec-addr: '0xB0000000', owner-hart: u54_3, priv-mode: prv_m, payload-name: "baremetal"}
 *   	test/u-boot:    {exec-addr: '0x80200000', owner-hart: u54_1, secondary-hart: u54_2, secondary-hart: u54_4, priv-mode: prv_s, ancilliary-data: test/amp-context-0.dtb, payload-name: "Yocto (U-Boot)"}
 */

/////////////////////////////////////////////////////////////////////////////
//
// Local module types
//

#ifndef NR_CPUs
#  define NR_CPUs (4u)
#endif

#define BYTES_PER_LINE (12u)

#define ENTRY_POINT 0x80000000lu

#define PAD_SIZE  8

#define PRV_U		0x00
#define PRV_S		0x01
#define PRV_H		0x02
#define PRV_M		0x03
#define PRV_ILLEGAL	0xFF

enum token
{
	TOKEN_UNKNOWN = 0,
	TOKEN_SET_NAME,
	TOKEN_HART_ENTRY_POINTS,
	TOKEN_PAYLOADS,
	TOKEN_PAYLOAD_EXEC_ADDR,
	TOKEN_PAYLOAD_OWNER_HART,
	TOKEN_PAYLOAD_SECONDARY_HART,
	TOKEN_PAYLOAD_PAYLOAD_NAME,
	TOKEN_PAYLOAD_PRIV_MODE,
	TOKEN_PAYLOAD_SKIP_OPENSBI,
	TOKEN_PAYLOAD_ALLOW_REBOOT,
	TOKEN_PAYLOAD_SKIP_AUTOBOOT,
	TOKEN_PAYLOAD_ANCILLIARY_DATA,
	TOKEN_PRIV_MODE_M,
	TOKEN_PRIV_MODE_H,
	TOKEN_PRIV_MODE_S,
	TOKEN_PRIV_MODE_U,
	TOKEN_HART_U54_1,
	TOKEN_HART_U54_2,
	TOKEN_HART_U54_3,
	TOKEN_HART_U54_4,
	TOKEN_REBOOT_COLD,
	TOKEN_REBOOT_WARM,
};

struct hss_config_token
{
	const enum token token_index;
	char const * const token_string;
};

const struct hss_config_token tokens[] = {
	{ TOKEN_SET_NAME,			"set-name" },
	{ TOKEN_HART_ENTRY_POINTS,		"hart-entry-points" },
	{ TOKEN_PAYLOADS,			"payloads" },
	{ TOKEN_PAYLOAD_EXEC_ADDR,		"exec-addr" },
	{ TOKEN_PAYLOAD_OWNER_HART,		"owner-hart" },
	{ TOKEN_PAYLOAD_PAYLOAD_NAME,		"payload-name" },
	{ TOKEN_PAYLOAD_PRIV_MODE,		"priv-mode" },
	{ TOKEN_PAYLOAD_SECONDARY_HART,		"secondary-hart" },
	{ TOKEN_PAYLOAD_SKIP_OPENSBI,		"skip-opensbi" },
	{ TOKEN_PAYLOAD_ALLOW_REBOOT,		"allow-reboot" },
	{ TOKEN_PAYLOAD_SKIP_AUTOBOOT,		"skip-autoboot" },
	{ TOKEN_PAYLOAD_ANCILLIARY_DATA,	"ancilliary-data" },
	{ TOKEN_PRIV_MODE_M,			"prv_m" },
	{ TOKEN_PRIV_MODE_S,			"prv_s" },
	{ TOKEN_PRIV_MODE_U,			"prv_u" },
	{ TOKEN_HART_U54_1,			"u54_1" },
	{ TOKEN_HART_U54_2,			"u54_2" },
	{ TOKEN_HART_U54_3,			"u54_3" },
	{ TOKEN_HART_U54_4,			"u54_4" },
	{ TOKEN_REBOOT_COLD,			"cold" },
	{ TOKEN_REBOOT_WARM,			"warm" }
};

enum ParserState
{
	STATE_IDLE,
	STATE_STREAM,
	STATE_DOC,
	STATE_MAPPING,
	STATE_SET_NAME,
	STATE_HART_ENTRY_POINTS,
	STATE_HART_ENTRY_POINTS_U54_1,
	STATE_HART_ENTRY_POINTS_U54_2,
	STATE_HART_ENTRY_POINTS_U54_3,
	STATE_HART_ENTRY_POINTS_U54_4,
	STATE_PAYLOAD_MAPPINGS,
	STATE_NEW_PAYLOAD,
	STATE_NEW_PAYLOAD_EXEC_ADDR,
	STATE_NEW_PAYLOAD_OWNER_HART,
	STATE_NEW_PAYLOAD_PAYLOAD_NAME,
	STATE_NEW_PAYLOAD_PRIV_MODE,
	STATE_NEW_PAYLOAD_SECONDARY_HART,
	STATE_NEW_PAYLOAD_SKIP_OPENSBI,
	STATE_NEW_PAYLOAD_ALLOW_REBOOT,
	STATE_NEW_PAYLOAD_SKIP_AUTOBOOT,
	STATE_NEW_PAYLOAD_ANCILLIARY_DATA,
};

const char * const stateNames[] =
{
	[ STATE_IDLE ] =			"STATE_IDLE",
	[ STATE_STREAM ] =			"STATE_STREAM",
	[ STATE_DOC ] =				"STATE_DOC",
	[ STATE_MAPPING ] =			"STATE_MAPPING",
	[ STATE_SET_NAME ] =			"STATE_SET_NAME",
	[ STATE_HART_ENTRY_POINTS ] =		"STATE_HART_ENTRY_POINTS",
	[ STATE_HART_ENTRY_POINTS_U54_1 ] =	"STATE_HART_ENTRY_POINTS_U54_1",
	[ STATE_HART_ENTRY_POINTS_U54_2 ] =	"STATE_HART_ENTRY_POINTS_U54_2",
	[ STATE_HART_ENTRY_POINTS_U54_3 ] =	"STATE_HART_ENTRY_POINTS_U54_3",
	[ STATE_HART_ENTRY_POINTS_U54_4 ] =	"STATE_HART_ENTRY_POINTS_U54_4",
	[ STATE_PAYLOAD_MAPPINGS ] =		"STATE_PAYLOAD_MAPPINGS",
	[ STATE_NEW_PAYLOAD ] =			"STATE_NEW_PAYLOAD",
	[ STATE_NEW_PAYLOAD_EXEC_ADDR ] =	"STATE_NEW_PAYLOAD_EXEC_ADDR",
	[ STATE_NEW_PAYLOAD_OWNER_HART ] =	"STATE_NEW_PAYLOAD_OWNER_HART",
	[ STATE_NEW_PAYLOAD_PAYLOAD_NAME ] =	"STATE_NEW_PAYLOAD_PAYLOAD_NAME",
	[ STATE_NEW_PAYLOAD_PRIV_MODE ] =	"STATE_NEW_PAYLOAD_PRIV_MODE",
	[ STATE_NEW_PAYLOAD_SECONDARY_HART ] =	"STATE_NEW_PAYLOAD_SECONDARY_HART",
	[ STATE_NEW_PAYLOAD_SKIP_OPENSBI ] =	"STATE_NEW_PAYLOAD_SKIP_OPENSBI",
	[ STATE_NEW_PAYLOAD_ALLOW_REBOOT ] =	"STATE_NEW_PAYLOAD_ALLOW_REBOOT",
	[ STATE_NEW_PAYLOAD_SKIP_AUTOBOOT ] =	"STATE_NEW_PAYLOAD_SKIP_AUTOBOOT",
	[ STATE_NEW_PAYLOAD_ANCILLIARY_DATA ] =	"STATE_NEW_PAYLOAD_ANCILLIARY_DATA",
};

struct StateHandler
{
	enum ParserState state;
	void (*handler)(yaml_event_t *pEvent);
};

/////////////////////////////////////////////////////////////////////////////
//
// Local module prototypes
//

static uint8_t map_token_to_priv_mode(enum token token_idx);
static void report_illegal_token(char const * const stateName, yaml_event_t *pEvent)	__attribute__((nonnull));
static void report_illegal_event(char const * const stateName, yaml_event_t *pEvent)	__attribute__((nonnull));
static enum token string_to_scalar(unsigned char const * const token_string)	__attribute__((nonnull));
static void concatenate(char *existing, char const * const new_chars, const size_t n)	__attribute__((nonnull));
static void populate_boot_flags(void);

static void Do_State_Transition(enum ParserState new_state);
static void Handle_STATE_IDLE(yaml_event_t *pEvent)				__attribute__((nonnull));
static void Handle_STATE_STREAM(yaml_event_t *pEvent)				__attribute__((nonnull));
static void Handle_STATE_DOC(yaml_event_t *pEvent)				__attribute__((nonnull));
static void Handle_STATE_MAPPING(yaml_event_t *pEvent)		 		__attribute__((nonnull));
static void Handle_STATE_SET_NAME(yaml_event_t *pEvent)				__attribute__((nonnull));
static void Handle_STATE_HART_ENTRY_POINTS(yaml_event_t *pEvent)		__attribute__((nonnull));
static void Handle_STATE_HART_ENTRY_POINTS_U54_1(yaml_event_t *pEvent)		__attribute__((nonnull));
static void Handle_STATE_HART_ENTRY_POINTS_U54_2(yaml_event_t *pEvent)		__attribute__((nonnull));
static void Handle_STATE_HART_ENTRY_POINTS_U54_3(yaml_event_t *pEvent)		__attribute__((nonnull));
static void Handle_STATE_HART_ENTRY_POINTS_U54_4(yaml_event_t *pEvent)		__attribute__((nonnull));
static void Handle_STATE_PAYLOAD_MAPPINGS(yaml_event_t *pEvent)			__attribute__((nonnull));
static void Handle_STATE_NEW_PAYLOAD(yaml_event_t *pEvent)			__attribute__((nonnull));
static void Handle_STATE_NEW_PAYLOAD_EXEC_ADDR(yaml_event_t *pEvent)		__attribute__((nonnull));
static void Handle_STATE_NEW_PAYLOAD_OWNER_HART(yaml_event_t *pEvent)		__attribute__((nonnull));
static void Handle_STATE_NEW_PAYLOAD_PAYLOAD_NAME(yaml_event_t *pEvent)		__attribute__((nonnull));
static void Handle_STATE_NEW_PAYLOAD_PRIV_MODE(yaml_event_t *pEvent)		__attribute__((nonnull));
static void Handle_STATE_NEW_PAYLOAD_SECONDARY_HART(yaml_event_t *pEvent)	__attribute__((nonnull));
static void Handle_STATE_NEW_PAYLOAD_SKIP_OPENSBI(yaml_event_t *pEvent)		__attribute__((nonnull));
static void Handle_STATE_NEW_PAYLOAD_ALLOW_REBOOT(yaml_event_t *pEvent)		__attribute__((nonnull));
static void Handle_STATE_NEW_PAYLOAD_SKIP_AUTOBOOT(yaml_event_t *pEvent)	__attribute__((nonnull));
static void Handle_STATE_NEW_PAYLOAD_ANCILLIARY_DATA(yaml_event_t *pEvent)	__attribute__((nonnull));

/////////////////////////////////////////////////////////////////////////////
//
// the parser is based on a simple state machine, which is fed by events
// and tokens from the LibYAML parsing engine
//
// this allows for some rudimentary detection of errors in the config file
// format

static size_t payload_idx = 0u;
static enum ParserState parser_state = STATE_IDLE;
static struct StateHandler stateHandler[] = {
	{ STATE_IDLE,				Handle_STATE_IDLE },
	{ STATE_STREAM,				Handle_STATE_STREAM },
	{ STATE_DOC,				Handle_STATE_DOC },
	{ STATE_MAPPING,			Handle_STATE_MAPPING },
	{ STATE_SET_NAME,			Handle_STATE_SET_NAME },
	{ STATE_HART_ENTRY_POINTS,		Handle_STATE_HART_ENTRY_POINTS },
	{ STATE_HART_ENTRY_POINTS_U54_1,	Handle_STATE_HART_ENTRY_POINTS_U54_1 },
	{ STATE_HART_ENTRY_POINTS_U54_2,	Handle_STATE_HART_ENTRY_POINTS_U54_2 },
	{ STATE_HART_ENTRY_POINTS_U54_3,	Handle_STATE_HART_ENTRY_POINTS_U54_3 },
	{ STATE_HART_ENTRY_POINTS_U54_4,	Handle_STATE_HART_ENTRY_POINTS_U54_4 },
	{ STATE_PAYLOAD_MAPPINGS,		Handle_STATE_PAYLOAD_MAPPINGS },
	{ STATE_NEW_PAYLOAD,			Handle_STATE_NEW_PAYLOAD },
	{ STATE_NEW_PAYLOAD_EXEC_ADDR,		Handle_STATE_NEW_PAYLOAD_EXEC_ADDR },
	{ STATE_NEW_PAYLOAD_OWNER_HART,		Handle_STATE_NEW_PAYLOAD_OWNER_HART },
	{ STATE_NEW_PAYLOAD_PAYLOAD_NAME,	Handle_STATE_NEW_PAYLOAD_PAYLOAD_NAME },
	{ STATE_NEW_PAYLOAD_PRIV_MODE,		Handle_STATE_NEW_PAYLOAD_PRIV_MODE },
	{ STATE_NEW_PAYLOAD_SECONDARY_HART,	Handle_STATE_NEW_PAYLOAD_SECONDARY_HART },
	{ STATE_NEW_PAYLOAD_SKIP_OPENSBI,	Handle_STATE_NEW_PAYLOAD_SKIP_OPENSBI },
	{ STATE_NEW_PAYLOAD_ALLOW_REBOOT,	Handle_STATE_NEW_PAYLOAD_ALLOW_REBOOT },
	{ STATE_NEW_PAYLOAD_SKIP_AUTOBOOT,	Handle_STATE_NEW_PAYLOAD_SKIP_AUTOBOOT },
	{ STATE_NEW_PAYLOAD_ANCILLIARY_DATA,	Handle_STATE_NEW_PAYLOAD_ANCILLIARY_DATA },
};

static bool override_set_name_flag = false;

extern struct HSS_BootImage bootImage;

/////////////////////////////////////////////////////////////////////////////

static uint8_t map_token_to_priv_mode(enum token token_idx)
{
	uint8_t result;

	switch (token_idx) {
	case TOKEN_PRIV_MODE_U:
		result = PRV_U;
		break;

	case TOKEN_PRIV_MODE_S:
		result = PRV_S;
		break;

	case TOKEN_PRIV_MODE_M:
		result = PRV_M;
		break;

	case TOKEN_PRIV_MODE_H:
		__attribute__((fallthrough)); // deliberate fallthrough
	default:
		fprintf(stderr, "Unknown priv mode token: %d\n", token_idx);
		exit(EXIT_FAILURE);
		break;
	}

	return result;
}

static void report_illegal_token(char const * const stateName, yaml_event_t *pEvent)
{
	assert(stateName);
	assert(pEvent);

	fprintf(stderr, "In %s: illegal token >>%s<< (%d)\n"
		"Please check configuration file\n",
		 (char *)stateName,
		(char *)pEvent->data.scalar.value,
		string_to_scalar(pEvent->data.scalar.value));
}

static void report_illegal_event(char const * const stateName, yaml_event_t *pEvent)
{
	assert(stateName);
	assert(pEvent);

	fprintf(stderr, "In %s: illegal event %d\n"
		"Please check configuration file\n",
		(char *)stateName, pEvent->type);
}


static enum token string_to_scalar(unsigned char const * const token_string)
{
	assert(token_string);

	enum token result = TOKEN_UNKNOWN;

	for (size_t i = 0u; i < ARRAY_SIZE(tokens); i++) {
		if (strncmp((char *)token_string, tokens[i].token_string, strlen(tokens[i].token_string)) == 0) {
			result = tokens[i].token_index;
			break;
		}
	}

	return result;
}

static void concatenate(char *existing, char const * const new_chars, const size_t n)
{
	char tempBuffer[n];

	strncpy(tempBuffer, existing, ARRAY_SIZE(tempBuffer));
	int ret_val = snprintf(existing, n-1, "%s%s", tempBuffer, new_chars);
	if (ret_val < 0) {
		// silently truncate
		debug_printf(3, "\t\tYAML Parser: string truncation occurred\n");
	}
	existing[n-1] = '\0';
}


static void Do_State_Transition(enum ParserState new_state)
{
	debug_printf(3, "\t\tYAML Parser: %s -> %s\n",
		(char *)stateNames[parser_state], (char *)stateNames[new_state]);
	parser_state = new_state;
}

static void Handle_STATE_IDLE(yaml_event_t *pEvent)
{
	assert(pEvent);

	switch (pEvent->type) {
	case YAML_STREAM_START_EVENT:
		Do_State_Transition(STATE_STREAM);
		break;

	default:
		report_illegal_event(stateNames[parser_state], pEvent);
		exit(EXIT_FAILURE);
		break;
	}
}

static void Handle_STATE_STREAM(yaml_event_t *pEvent)
{
	assert(pEvent);

	switch (pEvent->type) {
	case YAML_DOCUMENT_START_EVENT:
		Do_State_Transition(STATE_DOC);
		break;

	case YAML_DOCUMENT_END_EVENT:
		Do_State_Transition(STATE_IDLE);
		break;

	default:
		fprintf(stderr, "In %s: illegal event %d\n"
			"Please check configuration file\n",
			(char *)stateNames[parser_state], pEvent->type);
		exit(EXIT_FAILURE);
		break;
	}
}

static void Handle_STATE_DOC(yaml_event_t *pEvent)
{
	assert(pEvent);

	switch (pEvent->type) {
	case YAML_MAPPING_START_EVENT:
		Do_State_Transition(STATE_MAPPING);
		break;

	case YAML_DOCUMENT_END_EVENT:
		Do_State_Transition(STATE_STREAM);
		break;

	default:
		fprintf(stderr, "In %s: illegal event %d\n"
			"Please check configuration file\n",
			(char *)stateNames[parser_state], pEvent->type);
		exit(EXIT_FAILURE);
		break;
	}
}

static void Handle_STATE_MAPPING(yaml_event_t *pEvent)
{
	assert(pEvent);

	enum token token_idx = TOKEN_UNKNOWN;

	switch (pEvent->type) {
	case YAML_MAPPING_END_EVENT:
		Do_State_Transition(STATE_DOC);
		break;

	case YAML_SCALAR_EVENT:
		token_idx = string_to_scalar(pEvent->data.scalar.value);
		switch (token_idx) {
		case TOKEN_SET_NAME:
			debug_printf(0, "Parsing set-name\n");
			Do_State_Transition(STATE_SET_NAME);
			break;

		case TOKEN_HART_ENTRY_POINTS:
			debug_printf(0, "Parsing hart entry points\n");
			Do_State_Transition(STATE_HART_ENTRY_POINTS);
			break;

		case TOKEN_PAYLOADS:
			Do_State_Transition(STATE_PAYLOAD_MAPPINGS);
			break;
		default:
			report_illegal_token(stateNames[parser_state], pEvent);
			exit(EXIT_FAILURE);
			break;
		}
		break;

	default:
		report_illegal_event(stateNames[parser_state], pEvent);
		exit(EXIT_FAILURE);
		break;
	}
}

static void Handle_STATE_SET_NAME(yaml_event_t *pEvent)
{
	assert(pEvent);

	switch (pEvent->type) {
	case YAML_MAPPING_START_EVENT:
		assert(0==1);
		Do_State_Transition(STATE_MAPPING);
		break;

	case YAML_MAPPING_END_EVENT:
		assert(0==1);
		Do_State_Transition(STATE_MAPPING);
		break;

	case YAML_SCALAR_EVENT:
		override_set_name_flag = true;
		strncpy(bootImage.set_name, (char *)pEvent->data.scalar.value, BOOT_IMAGE_MAX_NAME_LEN-2);
		bootImage.set_name[BOOT_IMAGE_MAX_NAME_LEN-1] = '\0';
		Do_State_Transition(STATE_MAPPING);
		break;

	default:
		report_illegal_event(stateNames[parser_state], pEvent);
		exit(EXIT_FAILURE);
		break;
	}
}
static void Handle_STATE_HART_ENTRY_POINTS(yaml_event_t *pEvent)
{
	assert(pEvent);

	enum token token_idx = TOKEN_UNKNOWN;

	switch (pEvent->type) {
	case YAML_MAPPING_START_EVENT:
		break;

	case YAML_MAPPING_END_EVENT:
		Do_State_Transition(STATE_MAPPING);
		break;

	case YAML_SCALAR_EVENT:
		token_idx = string_to_scalar(pEvent->data.scalar.value);
		switch (token_idx) {
		case TOKEN_HART_U54_1:
			Do_State_Transition(STATE_HART_ENTRY_POINTS_U54_1);
			break;

		case TOKEN_HART_U54_2:
			Do_State_Transition(STATE_HART_ENTRY_POINTS_U54_2);
			break;

		case TOKEN_HART_U54_3:
			Do_State_Transition(STATE_HART_ENTRY_POINTS_U54_3);
			break;

		case TOKEN_HART_U54_4:
			Do_State_Transition(STATE_HART_ENTRY_POINTS_U54_4);
			break;

		default:
			report_illegal_token(stateNames[parser_state], pEvent);
			exit(EXIT_FAILURE);
			break;
		}
		break;

	default:
		report_illegal_event(stateNames[parser_state], pEvent);
		exit(EXIT_FAILURE);
		break;
	}
}

static void Handle_STATE_HART_ENTRY_POINTS_U54_1(yaml_event_t *pEvent)
{
	assert(pEvent);

	uintptr_t entry_point;

	switch (pEvent->type) {
	case YAML_MAPPING_START_EVENT:
		break;

	case YAML_MAPPING_END_EVENT:
		Do_State_Transition(STATE_MAPPING);
		break;

	case YAML_SCALAR_EVENT:
		entry_point = (uintptr_t)strtoul((char *)pEvent->data.scalar.value, NULL, 0);
		bootImage.hart[0].entryPoint = entry_point;

		debug_printf(1, "\tEntry Point U54_1 is %p\n", (void *)entry_point);
		Do_State_Transition(STATE_HART_ENTRY_POINTS);
		break;

	default:
		report_illegal_event(stateNames[parser_state], pEvent);
		exit(EXIT_FAILURE);
		break;
	}
}

static void Handle_STATE_HART_ENTRY_POINTS_U54_2(yaml_event_t *pEvent)
{
	assert(pEvent);

	uintptr_t entry_point;

	switch (pEvent->type) {
	case YAML_MAPPING_START_EVENT:
		break;

	case YAML_MAPPING_END_EVENT:
		Do_State_Transition(STATE_MAPPING);
		break;

	case YAML_SCALAR_EVENT:
		entry_point = (uintptr_t)strtoul((char *)pEvent->data.scalar.value, NULL, 0);
		bootImage.hart[1].entryPoint = entry_point;

		debug_printf(1, "\tEntry Point U54_2 is %p\n", (void *)entry_point);
		Do_State_Transition(STATE_HART_ENTRY_POINTS);
		break;

	default:
		report_illegal_event(stateNames[parser_state], pEvent);
		exit(EXIT_FAILURE);
		break;
	}
}

static void Handle_STATE_HART_ENTRY_POINTS_U54_3(yaml_event_t *pEvent)
{
	assert(pEvent);

	uintptr_t entry_point;

	switch (pEvent->type) {
	case YAML_MAPPING_START_EVENT:
		break;

	case YAML_MAPPING_END_EVENT:
		Do_State_Transition(STATE_MAPPING);
		break;

	case YAML_SCALAR_EVENT:
		entry_point = (uintptr_t)strtoul((char *)pEvent->data.scalar.value, NULL, 0);
		bootImage.hart[2].entryPoint = entry_point;

		debug_printf(1, "\tEntry Point U54_3 is %p\n", (void *)entry_point);
		Do_State_Transition(STATE_HART_ENTRY_POINTS);
		break;

	default:
		report_illegal_event(stateNames[parser_state], pEvent);
		exit(EXIT_FAILURE);
		break;
	}
}

static void Handle_STATE_HART_ENTRY_POINTS_U54_4(yaml_event_t *pEvent)
{
	assert(pEvent);

	uintptr_t entry_point;

	switch (pEvent->type) {
	case YAML_MAPPING_START_EVENT:
		break;

	case YAML_MAPPING_END_EVENT:
		Do_State_Transition(STATE_MAPPING);
		break;

	case YAML_SCALAR_EVENT:
		entry_point = (uintptr_t)strtoul((char *)pEvent->data.scalar.value, NULL, 0);
		bootImage.hart[3].entryPoint = entry_point;

		debug_printf(1, "\tEntry Point U54_4 is %p\n", (void *)entry_point);
		Do_State_Transition(STATE_HART_ENTRY_POINTS);
		break;

	default:
		report_illegal_event(stateNames[parser_state], pEvent);
		exit(EXIT_FAILURE);
		break;
	}
}

static char base_name[BOOT_IMAGE_MAX_NAME_LEN];
static char payload_name[BOOT_IMAGE_MAX_NAME_LEN];
static uintptr_t base_exec_addr = 0u;
static size_t base_owner = 0u;
static size_t base_secondary[3] = { 0u, 0u, 0u };
static uint8_t base_priv_mode = PRV_ILLEGAL;
static size_t secondary_idx = 0u;

static struct {
	bool ancilliary_data_present;
	bool skip_opensbi;
	bool skip_autoboot_flag;
	bool allow_coldreboot;
	bool allow_warmreboot;
} entitlement_flags = { false, false, false, false, false };

static char ancilliary_name[BOOT_IMAGE_MAX_NAME_LEN];

static void Handle_STATE_PAYLOAD_MAPPINGS(yaml_event_t *pEvent)
{
	assert(pEvent);

	switch (pEvent->type) {
	case YAML_MAPPING_START_EVENT:
		break;

	case YAML_MAPPING_END_EVENT:
		Do_State_Transition(STATE_MAPPING);
		break;

	case YAML_SCALAR_EVENT:
		debug_printf(0, "Parsing payload >>%s<<\n", pEvent->data.scalar.value);

		strncpy(base_name, (char *)pEvent->data.scalar.value, BOOT_IMAGE_MAX_NAME_LEN-2);
		payload_name[0] = '\0';
		base_name[BOOT_IMAGE_MAX_NAME_LEN-1] = '\0';

		base_exec_addr = 0u;
		base_owner = 0u;
		secondary_idx = 0u;
		base_secondary[0] = 0u;
		base_secondary[1] = 0u;
		base_secondary[2] = 0u;
		base_priv_mode = PRV_M;

		entitlement_flags.ancilliary_data_present = false;
		entitlement_flags.skip_opensbi = false;
		entitlement_flags.skip_autoboot_flag = false;
		entitlement_flags.allow_coldreboot = false;
		entitlement_flags.allow_warmreboot = false;

		Do_State_Transition(STATE_NEW_PAYLOAD);
		break;

	default:
		report_illegal_event(stateNames[parser_state], pEvent);
		exit(EXIT_FAILURE);
		break;
	}
}

static void Handle_STATE_NEW_PAYLOAD(yaml_event_t *pEvent)
{
	assert(pEvent);

	enum token token_idx = TOKEN_UNKNOWN;

	switch (pEvent->type) {
	case YAML_MAPPING_START_EVENT:
		base_owner = 0u;
		break;

	case YAML_MAPPING_END_EVENT:
		if (!override_set_name_flag) {
			if (strlen(payload_name)) {
				concatenate(bootImage.set_name, payload_name, ARRAY_SIZE(bootImage.set_name));
			} else {
				concatenate(bootImage.set_name, base_name, ARRAY_SIZE(bootImage.set_name));
			}
		}

		assert(base_owner);
		if (strlen(bootImage.hart[base_owner-1].name)) {
			concatenate(bootImage.hart[base_owner-1].name, "+",
					ARRAY_SIZE(bootImage.hart[base_owner-1].name));
		}

		if (strlen(payload_name)) {
			concatenate(bootImage.hart[base_owner-1].name, payload_name,
					ARRAY_SIZE(bootImage.hart[base_owner-1].name));
		} else {
			concatenate(bootImage.hart[base_owner-1].name, base_name,
					ARRAY_SIZE(bootImage.hart[base_owner-1].name));
		}

		bool retVal = elf_parser(base_name, base_owner, base_exec_addr);
		if (!retVal) {
			// assume it is a binary file, so just embed the entire thing...
			if (entitlement_flags.ancilliary_data_present) {
				// legacy: smuggle this into owner highest bit...
				blob_handler(base_name, base_exec_addr, base_owner, true, ancilliary_name);
			} else {
				blob_handler(base_name, base_exec_addr, base_owner, false, NULL);
			}

		}

		populate_boot_flags();
		payload_idx++;
		Do_State_Transition(STATE_PAYLOAD_MAPPINGS);
		break;

	case YAML_SCALAR_EVENT:
		// debug_printf(0, "Parsing scalar >>%s<<\n", pEvent->data.scalar.value);

		token_idx = string_to_scalar(pEvent->data.scalar.value);
		switch (token_idx) {
		case TOKEN_PAYLOAD_EXEC_ADDR:
			Do_State_Transition(STATE_NEW_PAYLOAD_EXEC_ADDR);
			break;

		case TOKEN_PAYLOAD_OWNER_HART:
			Do_State_Transition(STATE_NEW_PAYLOAD_OWNER_HART);
			break;

		case TOKEN_PAYLOAD_SECONDARY_HART:
			Do_State_Transition(STATE_NEW_PAYLOAD_SECONDARY_HART);
			break;

		case TOKEN_PAYLOAD_PAYLOAD_NAME:
			Do_State_Transition(STATE_NEW_PAYLOAD_PAYLOAD_NAME);
			break;

		case TOKEN_PAYLOAD_PRIV_MODE:
			Do_State_Transition(STATE_NEW_PAYLOAD_PRIV_MODE);
			break;

		case TOKEN_PAYLOAD_SKIP_OPENSBI:
			debug_printf(1, "\tskipping OpenSBI for this context\n");
			Do_State_Transition(STATE_NEW_PAYLOAD_SKIP_OPENSBI);
			break;

		case TOKEN_PAYLOAD_SKIP_AUTOBOOT:
			debug_printf(1, "\tskipping autoboot for this payload\n");
			Do_State_Transition(STATE_NEW_PAYLOAD_SKIP_AUTOBOOT);
			break;

		case TOKEN_PAYLOAD_ALLOW_REBOOT:
			debug_printf(1, "\tallow reboot for this context\n");
			Do_State_Transition(STATE_NEW_PAYLOAD_ALLOW_REBOOT);
			break;

		case TOKEN_PAYLOAD_ANCILLIARY_DATA:
			debug_printf(1, "\tancilliary data found\n");
			entitlement_flags.ancilliary_data_present = true;
			Do_State_Transition(STATE_NEW_PAYLOAD_ANCILLIARY_DATA);
			break;

		default:
			report_illegal_token(stateNames[parser_state], pEvent);
			exit(EXIT_FAILURE);
			break;
		}
		break;

	default:
		report_illegal_event(stateNames[parser_state], pEvent);
		exit(EXIT_FAILURE);
		break;
	}
}

static void Handle_STATE_NEW_PAYLOAD_EXEC_ADDR(yaml_event_t *pEvent)
{
	assert(pEvent);

	switch (pEvent->type) {
	case YAML_MAPPING_START_EVENT:
		break;

	case YAML_MAPPING_END_EVENT:
		Do_State_Transition(STATE_MAPPING);
		break;

	case YAML_SCALAR_EVENT:
		base_exec_addr = (uintptr_t)strtoul((char *)pEvent->data.scalar.value, NULL, 0);

		debug_printf(1, "\texec_addr is %p\n", (void *)base_exec_addr);
		Do_State_Transition(STATE_NEW_PAYLOAD);
		break;

	default:
		report_illegal_event(stateNames[parser_state], pEvent);
		exit(EXIT_FAILURE);
		break;
	}
}

static void Handle_STATE_NEW_PAYLOAD_OWNER_HART(yaml_event_t *pEvent)
{
	assert(pEvent);

	enum token token_idx;

	switch (pEvent->type) {
	case YAML_MAPPING_START_EVENT:
		break;

	case YAML_MAPPING_END_EVENT:
		Do_State_Transition(STATE_MAPPING);
		break;

	case YAML_SCALAR_EVENT:
		token_idx = string_to_scalar(pEvent->data.scalar.value);
		switch (token_idx) {
		case TOKEN_HART_U54_1:
			__attribute__((fallthrough)); // deliberate fallthrough
		case TOKEN_HART_U54_2:
			__attribute__((fallthrough)); // deliberate fallthrough
		case TOKEN_HART_U54_3:
			__attribute__((fallthrough)); // deliberate fallthrough
		case TOKEN_HART_U54_4:
			base_owner = (token_idx - TOKEN_HART_U54_1 + 1u);
			debug_printf(1, "\towner is %" PRIu64 "\n", base_owner);
			Do_State_Transition(STATE_NEW_PAYLOAD);
			break;

		default:
			report_illegal_token(stateNames[parser_state], pEvent);
			exit(EXIT_FAILURE);
			break;
		}
		break;

	default:
		report_illegal_event(stateNames[parser_state], pEvent);
		exit(EXIT_FAILURE);
		break;
	}
}

static void Handle_STATE_NEW_PAYLOAD_SECONDARY_HART(yaml_event_t *pEvent)
{
	assert(pEvent);

	enum token token_idx;

//printf("%s(): event %d / token %s\n", __func__, pEvent->type, pEvent->data.scalar.value);
	switch (pEvent->type) {
	case YAML_MAPPING_START_EVENT:
		break;

	case YAML_MAPPING_END_EVENT:
		Do_State_Transition(STATE_MAPPING);
		break;

	case YAML_SCALAR_EVENT:
		token_idx = string_to_scalar(pEvent->data.scalar.value);
		switch (token_idx) {
		case TOKEN_HART_U54_1:
			__attribute__((fallthrough)); // deliberate fallthrough
		case TOKEN_HART_U54_2:
			__attribute__((fallthrough)); // deliberate fallthrough
		case TOKEN_HART_U54_3:
			__attribute__((fallthrough)); // deliberate fallthrough
		case TOKEN_HART_U54_4:
			base_secondary[secondary_idx] = (token_idx - TOKEN_HART_U54_1 + 1u);

			debug_printf(1, "\tsecondary is %" PRIu64 "\n", base_secondary[secondary_idx]);
			secondary_idx++;

			Do_State_Transition(STATE_NEW_PAYLOAD);
			break;

		default:
			report_illegal_token(stateNames[parser_state], pEvent);
			exit(EXIT_FAILURE);
			break;
		}
		break;
	default:
		report_illegal_event(stateNames[parser_state], pEvent);
		exit(EXIT_FAILURE);
		break;
	}
}

static void populate_boot_flags(void)
{
	uint8_t flags = 0u;

	if (entitlement_flags.ancilliary_data_present) {
		flags |= BOOT_FLAG_ANCILLIARY_DATA;
	}

	if (entitlement_flags.skip_opensbi) {
		flags |= BOOT_FLAG_SKIP_OPENSBI;
	}

	if (entitlement_flags.allow_coldreboot) {
		// cold reboot implies warm reboot also allowed
		flags |= BOOT_FLAG_ALLOW_COLD_REBOOT;
		flags |= BOOT_FLAG_ALLOW_WARM_REBOOT;
	} else if (entitlement_flags.allow_warmreboot) {
		flags |= BOOT_FLAG_ALLOW_WARM_REBOOT;
	}

	if (entitlement_flags.skip_autoboot_flag) {
		flags |= BOOT_FLAG_SKIP_AUTOBOOT;
	}

	bootImage.hart[base_owner-1].flags = flags;

	for (size_t i = 0u; i < ARRAY_SIZE(base_secondary); i++) {
		if (base_secondary[i] != 0u) {
			bootImage.hart[base_secondary[i]-1].flags = flags;
		}
	}
}

static void Handle_STATE_NEW_PAYLOAD_PAYLOAD_NAME(yaml_event_t *pEvent)
{
	assert(pEvent);

	switch (pEvent->type) {
	case YAML_MAPPING_START_EVENT:
		assert(0==1);
		Do_State_Transition(STATE_MAPPING);
		break;

	case YAML_MAPPING_END_EVENT:
		assert(0==1);
		Do_State_Transition(STATE_MAPPING);
		break;

	case YAML_SCALAR_EVENT:
		strncpy(payload_name, (char *)pEvent->data.scalar.value, BOOT_IMAGE_MAX_NAME_LEN-2);
		payload_name[BOOT_IMAGE_MAX_NAME_LEN-1] = '\0';
		override_set_name_flag = true;
		Do_State_Transition(STATE_NEW_PAYLOAD);
		break;

	default:
		report_illegal_event(stateNames[parser_state], pEvent);
		exit(EXIT_FAILURE);
		break;
	}
}

static void Handle_STATE_NEW_PAYLOAD_PRIV_MODE(yaml_event_t *pEvent)
{
	assert(pEvent);

	enum token token_idx;

	switch (pEvent->type) {
	case YAML_MAPPING_START_EVENT:
		break;

	case YAML_MAPPING_END_EVENT:
		Do_State_Transition(STATE_MAPPING);
		break;

	case YAML_SCALAR_EVENT:
		token_idx = string_to_scalar(pEvent->data.scalar.value);
		switch (token_idx) {
		case TOKEN_PRIV_MODE_M:
			__attribute__((fallthrough)); // deliberate fallthrough
		case TOKEN_PRIV_MODE_S:
			__attribute__((fallthrough)); // deliberate fallthrough
		case TOKEN_PRIV_MODE_U:
			base_priv_mode = map_token_to_priv_mode(token_idx);

			debug_printf(1, "\tpriv_mode is %u\n", base_priv_mode);
			debug_printf(2, "\tSetting priv mode for %d to %d\n", base_owner, base_priv_mode);

			bootImage.hart[base_owner-1].privMode = base_priv_mode;

			for (size_t i = 0u; i < ARRAY_SIZE(base_secondary); i++) {
				if (base_secondary[i] != 0u) {
					if (bootImage.hart[base_secondary[i]-1].privMode == PRV_ILLEGAL) {
						debug_printf(2, "\tSetting priv mode for %d to %d\n", base_secondary[i], base_priv_mode);
						bootImage.hart[base_secondary[i]-1].privMode = base_priv_mode;
					} else {
						fprintf(stderr, "Attempted to set U54_%" PRIu64 " to priv_mode %u, but it is already set to %u\n"
							"Please check configuration file\n",
							base_secondary[i], base_priv_mode, bootImage.hart[base_secondary[i]-1].privMode);
						exit(EXIT_FAILURE);
					}
				}
			}
			Do_State_Transition(STATE_NEW_PAYLOAD);
			break;

		default:
			report_illegal_token(stateNames[parser_state], pEvent);
			exit(EXIT_FAILURE);
			break;
		}
		break;

	default:
		report_illegal_event(stateNames[parser_state], pEvent);
		exit(EXIT_FAILURE);
		break;
	}
}

static void Handle_STATE_NEW_PAYLOAD_SKIP_OPENSBI(yaml_event_t *pEvent)
{
	assert(pEvent);
	uint8_t value;

	switch (pEvent->type) {
	case YAML_MAPPING_START_EVENT:
		break;

	case YAML_MAPPING_END_EVENT:
		assert(0 == 1);
		Do_State_Transition(STATE_MAPPING);
		break;

	case YAML_SCALAR_EVENT:
		if (!strncasecmp((char *)pEvent->data.scalar.value, "true", 4)) {
			value = 1u;
		} else {
			value = (uint8_t)strtoul((char *)pEvent->data.scalar.value, NULL, 0);
		}

		if (value) {
			entitlement_flags.skip_opensbi = true;
		}
		Do_State_Transition(STATE_NEW_PAYLOAD);
		break;

	default:
		report_illegal_event(stateNames[parser_state], pEvent);
		exit(EXIT_FAILURE);
		break;
	}
}

static void Handle_STATE_NEW_PAYLOAD_ALLOW_REBOOT(yaml_event_t *pEvent)
{
	assert(pEvent);

	enum token token_idx = TOKEN_UNKNOWN;

	switch (pEvent->type) {
	case YAML_MAPPING_START_EVENT:
		break;

	case YAML_MAPPING_END_EVENT:
		Do_State_Transition(STATE_MAPPING);
		break;

	case YAML_SCALAR_EVENT:
		token_idx = string_to_scalar(pEvent->data.scalar.value);
		switch (token_idx) {
		case TOKEN_REBOOT_WARM:
			entitlement_flags.allow_warmreboot = true;
			break;

		case TOKEN_REBOOT_COLD:
			entitlement_flags.allow_coldreboot = true;
			break;

		default:
			report_illegal_token(stateNames[parser_state], pEvent);
			exit(EXIT_FAILURE);
			break;
		}
		Do_State_Transition(STATE_NEW_PAYLOAD);
		break;

	default:
		report_illegal_event(stateNames[parser_state], pEvent);
		exit(EXIT_FAILURE);
		break;
	}
}

static void Handle_STATE_NEW_PAYLOAD_SKIP_AUTOBOOT(yaml_event_t *pEvent)
{
	assert(pEvent);
	uint8_t value;

	switch (pEvent->type) {
	case YAML_MAPPING_START_EVENT:
		break;

	case YAML_MAPPING_END_EVENT:
		Do_State_Transition(STATE_MAPPING);
		break;

	case YAML_SCALAR_EVENT:
		if (!strncasecmp((char *)pEvent->data.scalar.value, "true", 4)) {
			value = 1;
		} else {
			value = (uint8_t)strtoul((char *)pEvent->data.scalar.value, NULL, 0);
		}

		if (value) {
			entitlement_flags.skip_autoboot_flag = true;
		}
		Do_State_Transition(STATE_NEW_PAYLOAD);
		break;

	default:
		report_illegal_event(stateNames[parser_state], pEvent);
		exit(EXIT_FAILURE);
		break;
	}
}

static void Handle_STATE_NEW_PAYLOAD_ANCILLIARY_DATA(yaml_event_t *pEvent)
{
	assert(pEvent);

	//printf("%s(): event %d / token %s\n", __func__, pEvent->type, pEvent->data.scalar.value);
	switch (pEvent->type) {
	case YAML_MAPPING_START_EVENT:
		break;

	case YAML_MAPPING_END_EVENT:
		Do_State_Transition(STATE_MAPPING);
		break;

	case YAML_SCALAR_EVENT:
		debug_printf(0, "Parsing ancilliary payload >>%s<<\n", pEvent->data.scalar.value);
		strncpy(ancilliary_name, (char *)pEvent->data.scalar.value, BOOT_IMAGE_MAX_NAME_LEN-2);
		ancilliary_name[BOOT_IMAGE_MAX_NAME_LEN-1] = '\0';

		Do_State_Transition(STATE_NEW_PAYLOAD);
		break;

	default:
		report_illegal_event(stateNames[parser_state], pEvent);
		exit(EXIT_FAILURE);
		break;
	}
}



/////////////////////////////////////////////////////////////////////////////

void yaml_parser(char const * const input_filename)
{
	assert(input_filename);

	yaml_parser_t parser;

	if (!yaml_parser_initialize(&parser)) {
		perror("yaml_parser_initialize()");
		exit(EXIT_FAILURE);
	}

	FILE *configFileIn = fopen((char *)input_filename, "r");
	if (!configFileIn) {
		fprintf(stderr, "%s(): File: %s -", __func__, input_filename);
		perror("fopen()");
		exit(EXIT_FAILURE);
	}

	strncpy(bootImage.set_name, "PolarFireSOC-HSS::", BOOT_IMAGE_MAX_NAME_LEN-2);
	bootImage.hart[0].privMode = PRV_ILLEGAL;
	bootImage.hart[1].privMode = PRV_ILLEGAL;
	bootImage.hart[2].privMode = PRV_ILLEGAL;
	bootImage.hart[3].privMode = PRV_ILLEGAL;

	bootImage.hart[0].flags = 0u;
	bootImage.hart[1].flags = 0u;
	bootImage.hart[2].flags = 0u;
	bootImage.hart[3].flags = 0u;

	yaml_parser_set_input_file(&parser, configFileIn);
	yaml_event_t event;

	do {
		if (!yaml_parser_parse(&parser, &event)) {
			perror("yaml_parser_parse()");
			exit(EXIT_FAILURE);
		}

		switch (event.type) {
		default:
			stateHandler[parser_state].handler(&event);
			yaml_event_delete(&event);
			break;

		case YAML_NO_EVENT:
			yaml_event_delete(&event);
			break;

		case YAML_STREAM_END_EVENT:
			break;

		}
	} while (event.type != YAML_STREAM_END_EVENT);
	yaml_event_delete(&event);

	printf("Set-name is >>%s<<\n", bootImage.set_name);

	yaml_parser_delete(&parser);
	fclose(configFileIn);
}
