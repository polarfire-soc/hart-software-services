/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * Microchip PolarFire SoC MSS USB Driver Stack
 *
 *
 * Standard USB definitions.
 *
 *
 * SVN $Revision$
 * SVN $Date$
 */
#ifndef __MSS_USB_STD_DEF_H_
#define __MSS_USB_STD_DEF_H_

#ifdef __cplusplus
extern "C" {
#endif

/*-------------------------------------------------------------------------*//**
  Constant values which are internally used by the driver.
  ============================
 */

/* USB request types */
#define USB_STANDARD_REQUEST                            0x00u
#define USB_CLASS_REQUEST                               0x20u
#define USB_VENDOR_REQUEST                              0x40u

/* USB request type masks */
#define USB_STD_REQ_DATA_DIR_MASK                       0x80u
#define USB_STD_REQ_TYPE_MASK                           0x60u
#define USB_STD_REQ_RECIPIENT_MASK                      0x1Fu

#define USB_STD_REQ_DATA_DIR_OUT                        0x00u
#define USB_STD_REQ_DATA_DIR_IN                         0x80u

#define USB_STD_REQ_RECIPIENT_DEVICE                    0x00u
#define USB_STD_REQ_RECIPIENT_INTERFACE                 0x01u
#define USB_STD_REQ_RECIPIENT_ENDPOINT                  0x02u

#define USB_STD_REQ_GET_STATUS                          0x00u
#define USB_STD_REQ_CLEAR_FEATURE                       0x01u
#define USB_STD_REQ_SET_FEATURE                         0x03u
#define USB_STD_REQ_SET_ADDRESS                         0x05u
#define USB_STD_REQ_GET_DESCRIPTOR                      0x06u
#define USB_STD_REQ_SET_DESCRIPTOR                      0x07u
#define USB_STD_REQ_GET_CONFIG                          0x08u
#define USB_STD_REQ_SET_CONFIG                          0x09u
#define USB_STD_REQ_GET_INTERFACE                       0x0Au
#define USB_STD_REQ_SET_INTERFACE                       0x0Bu
#define USB_STD_REQ_SYNCH_FRAME                         0x0Cu

/* USB Feature selector */
#define USB_STD_FEATURE_REMOTE_WAKEUP                   0x0001u
#define USB_STD_FEATURE_TEST_MODE                       0x0002u
#define USB_STD_FEATURE_EP_HALT                         0x0000u

/* USB Test Mode, Test selectors */
#define USB_TEST_MODE_SELECTOR_TEST_J                   0x01u
#define USB_TEST_MODE_SELECTOR_TEST_K                   0x02u
#define USB_TEST_MODE_SELECTOR_TEST_SE0NAK              0x03u
#define USB_TEST_MODE_SELECTOR_TEST_PACKET              0x04u
#define USB_TEST_MODE_SELECTOR_TEST_FORCE_ENA           0x05u

/* Descriptor types */
#define USB_DEVICE_DESCRIPTOR_TYPE                      1u
#define USB_CONFIGURATION_DESCRIPTOR_TYPE               2u
#define USB_STRING_DESCRIPTOR_TYPE                      3u
#define USB_INTERFACE_DESCRIPTOR_TYPE                   4u
#define USB_ENDPOINT_DESCRIPTOR_TYPE                    5u
#define USB_DEVICE_QUALIFIER_DESCRIPTOR_TYPE            6u
#define USB_OTHER_SPEED_CONFIG_DESCRIPTOR_TYPE          7u
#define USB_INTERFACE_POWER_DESCRIPTOR_TYPE             8u
#define USB_INTERFACE_ASSOCIATION_DESCRIPTOR_TYPE       11u

#define USB_WVALUE_HIBITE_SHIFT                         8u
#define USB_WINDEX_HIBITE_SHIFT                         8u

#define USB_EP_DESCR_ATTR_CONTROL                       0x00u
#define USB_EP_DESCR_ATTR_ISO                           0x01u
#define USB_EP_DESCR_ATTR_BULK                          0x02u
#define USB_EP_DESCR_ATTR_INTR                          0x03u

#define USB_BCD_VERSION_2_0                             0x0200u
#define USB_BCD_VERSION_2_1                             0x0210u
#define USB_BCD_VERSION_1_1                             0x0110u
#define USB_BCD_VERSION_1_0                             0x0100u

#define USB_DEFAULT_TARGET_ADDR                         0u
#define USB_SETUP_PKT_LEN                               8u

#define USB_STD_DEVICE_DESCR_LEN                        18u
#define USB_STD_CONFIG_DESCR_LEN                        9u
#define USB_STD_INTERFACE_DESCR_LEN                     9u
#define USB_STD_ENDPOINT_DESCR_LEN                      7u
#define USB_STD_IA_DESCR_LEN                            8u
#define USB_STD_DEV_QUAL_DESCR_LENGTH                   10u

#define USB_DEVICE_BUS_POWER_MASK                       0x40u
#define USB_DEVICE_REM_WAKEUP_MASK                      0x20u
#define USB_MAX_BUS_POWER                               250u   /*num = mA/2*/

#define USB_CLASS_CODE_MSD                              0x08u  /* bInterfaceClass */
#define USB_CLASS_MSD_SUBCLASS_SCSI                     0x06u  /* bInterfaceSubClass */
#define USB_CLAS_MSD_PROTOCOL_BOT                       0x50u  /* bInterfaceProtocol */

#define USB_DEF_CONFIG_NUM                              0u

/*-------------------------------------------------------------------------*//**
  Maximum allowed Packet Sizes for respective transfer types
 */
#define USB_HS_BULK_MAX_PKT_SIZE                        512u
#define USB_HS_INTERRUPT_MAX_PKT_SIZE                   1024u
#define USB_HS_ISO_MAX_PKT_SIZE                         1024u

#define USB_FS_BULK_MAX_PKT_SIZE                        64u
#define USB_FS_INTERRUPT_MAX_PKT_SIZE                   64u
#define USB_FS_ISO_MAX_PKT_SIZE                         1023u

/*-------------------------------------------------------------------------*//**
 MSC class related definitions
 */
/* BoT protocol constants */
#define USBD_MSC_BOT_CSW_LENGTH                         13u
#define USBD_MSC_BOT_CBW_LENGTH                         31u

#define USB_MSC_BOT_REQ_GET_MAX_LUN                     0xFEu
#define USB_MSC_BOT_REQ_BMS_RESET                       0xFFu

#define USB_MSC_BOT_CBW_SIGNATURE                       0x43425355u
#define USB_MSC_BOT_CSW_SIGNATURE                       0x53425355u

/* Supported SCSI commands*/
#define USB_MSC_SCSI_INQUIRY                            0x12u
#define USB_MSC_SCSI_READ_FORMAT_CAPACITIES             0x23u
#define USB_MSC_SCSI_READ_CAPACITY_10                   0x25u
#define USB_MSC_SCSI_READ_10                            0x28u
#define USB_MSC_SCSI_REQUEST_SENSE                      0x03u
#define USB_MSC_SCSI_TEST_UNIT_READY                    0x00u
#define USB_MSC_SCSI_WRITE_10                           0x2Au
#define USB_MSC_SCSI_MODE_SENSE_6                       0x1Au
#define USB_MSC_SCSI_MODE_SELECT_6                      0x15u
#define USB_MSC_SCSI_PREVENT_ALLW_MDM_RMVL              0x1Eu
#define USB_MSC_SCSI_VERIFY_10                          0x2Fu
#define USB_MSC_SCSI_INVALID_COMMAND_CODE               0xFFu

/* Additional sense codes */
#define SC_NO_SENSE                                     0x00u
#define SC_RECOVERED_ERR                                0x01u
#define SC_NOT_READY                                    0x02u
#define SC_MEDIUM_ERROR                                 0x03u
#define SC_HARDWARE_ERR                                 0x04u
#define SC_ILLEGAL_REQUEST                              0x05u
#define SC_UNIT_ATTENTION                               0x06u
#define SC_DATA_PROTECT                                 0x07u
#define SC_BLANK_CHECK                                  0x08u
#define SC_VENDOR_SPECIFIC                              0x09u
#define SC_COPY_ABORTED                                 0x0Au
#define SC_ABORTED_COMMAND                              0x0Bu
/*0x0C is obsolete*/
#define SC_VOLUME_OVERFLOW                              0x0Du
#define SC_MISCOMPARE                                   0x0Eu
/*0x0F is reserved*/

#define ASC_INVALID_CDB                                 0x20u
#define ASC_INVALID_FIELED_IN_COMMAND                   0x24u
#define ASC_PARAMETER_LIST_LENGTH_ERROR                 0x1Au
#define ASC_INVALID_FIELD_IN_PARAMETER_LIST             0x26u
#define ASC_ADDRESS_OUT_OF_RANGE                        0x21u
#define ASC_MEDIUM_NOT_PRESENT                          0x3Au
#define ASC_MEDIUM_HAS_CHANGED                          0x28u
#define ASC_WRITE_PROTECTED                             0x27u
#define ASC_UNRECOVERED_READ_ERROR                      0x11u
#define ASC_WRITE_FAULT                                 0x03u

/*-------------------------------------------------------------------------*//**
  CDC class related definitions
 */
/* CDC Requests for ACM sub-class */
#define USB_CDC_SEND_ENCAPSULATED_COMMAND               0x00u
#define USB_CDC_GET_ENCAPSULATED_RESPONSE               0x01u
#define USB_CDC_SET_COMM_FEATURE                        0x02u
#define USB_CDC_GET_COMM_FEATURE                        0x03u
#define USB_CDC_CLEAR_COMM_FEATURE                      0x04u
#define USB_CDC_SET_LINE_CODING                         0x20u
#define USB_CDC_GET_LINE_CODING                         0x21u
#define USB_CDC_SET_CONTROL_LINE_STATE                  0x22u
#define USB_CDC_SEND_BREAK                              0x23u

/*-------------------------------------------------------------------------*//**
  HID class related definitions
 */
#define USB_HID_DESCR_LENGTH                            9u

#define USB_HID_DESCRIPTOR_TYPE                         0x21u
#define USB_REPORT_DESCRIPTOR_TYPE                      0x22u


#ifdef __cplusplus
}
#endif

#endif  /* __MSS_USB_STD_DEF_H_ */
