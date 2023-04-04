/*******************************************************************************
 * Copyright 2019-2022 Microchip FPGA Embedded Systems Solutions.
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
 *
 * PolarFire SoC Microprocessor Subsystem(MSS) system services bare metal driver
 * implementation.
 */

/*=========================================================================*//**
  @mainpage PolarFire SoC MSS System services Bare Metal Driver

  ==============================================================================
  Introduction
  ==============================================================================
  The PolarFire SoC system services are the services offered by the system
  controller. These services can be requested by PolarFire SoC MSS over System
  Controller Bus (SCB). The MSS appears as SCB master over the SCB bus. MSS can
  communicate with system controller over SCB by write/read to the MSS SCB
  register space. The PolarFire SoC system service driver software provides a
  set of functions for controlling the PolarFire SoC system services as part of
  a bare-metal system where no operating system is available. It can be adapted
  to be used as a part of an operating system, but the implementation of the
  adaptation layer between this driver and the operating system's driver model
  is outside the scope of this driver.

  ==============================================================================
  Hardware Flow Dependencies
  ==============================================================================
  The configuration of all the features of the PolarFire SoC MSS system services
  is covered by this driver. Besides, this driver does not require any other
  configuration. It relies on SCB register access interface to communicate with
  system controller.
  The base address and register addresses are defined in this driver as
  constants. The interrupt number assigned are defined as constants in the MPFS
  HAL. Ensure that the latest MPFS HAL is included in the project settings of
  the SoftConsole toolchain and that it is generated into your project.

  ==============================================================================
  Theory of Operation
  ==============================================================================
  The PolarFire SoC system services are the services offered by the system
  controller. These services can be requested by PolarFire SoC MSS over System
  Controller Bus (SCB). The MSS appears as SCB master over the SCB bus. MSS can
  communicate with the system controller over SCB by accessing the MSS SCB
  register space. Requesting a system service over the SCB bus involves a
  command/response sequence to transfer a system service command from the MSS to
  the system controller and to transfer status back from the system controller
  to the MSS. The MSS SCB register space also provides access to the mailbox.
  The mailbox is used for passing data related to the system service between the
  MSS and system controller in both directions.
  On completion of service, the system controller also writes a status code
  indicating the successful completion of the system service or an error code
  into the status register.

  These system services are grouped into the following categories:
    -  Device and design information services
    -  Design services
    -  Data security services
    -  Fabric services
    -  MSS services

  -----------------------------------------------------------------------------
    Device and Design Information Services
  -----------------------------------------------------------------------------
  The PolarFire SoC system service driver can be used to read information about
  the device on which it is being executed and the current fabric design by
  making a call to the following functions.
    -  MSS_SYS_get_serial_number()
    -  MSS_SYS_get_user_code()
    -  MSS_SYS_get_design_info()
    -  MSS_SYS_get_device_certificate()
    -  MSS_SYS_read_digest()
    -  MSS_SYS_query_security()
    -  MSS_SYS_read_debug_info()
    -  MSS_SYS_read_envm_parameter()

  -----------------------------------------------------------------------------
  Design Services
  -----------------------------------------------------------------------------
  The PolarFire SoC system service driver can be used to perform bitstream
  authentication using the following functions.
    -  MSS_SYS_authenticate_bitstream()
    -  MSS_SYS_authenticate_iap_image()

  -----------------------------------------------------------------------------
  Data Security Services
  -----------------------------------------------------------------------------
  The PolarFire SoC System Service driver can be used to execute data security
  services using the following functions:
    -  MSS_SYS_digital_signature_service ()
    -  MSS_SYS_secure_nvm_write()
    -  MSS_SYS_secure_nvm_read()
    -  MSS_SYS_puf_emulation_service ()
    -  MSS_SYS_nonce_service ()

  -----------------------------------------------------------------------------
  Executing Fabric Services
  -----------------------------------------------------------------------------
  The PolarFire SoC System Service driver can be used to execute fabric services
  using the following functions:
    -  MSS_SYS_digest_check()
    -  MSS_SYS_execute_iap()

  -----------------------------------------------------------------------------
  MSS Services
  -----------------------------------------------------------------------------
  The PolarFire SoC System Service driver can be used to execute MSS services
  using following functions:
    -  MSS_SYS_spi_copy()
    -  MSS_SYS_debug_read_probe()
    -  MSS_SYS_debug_write_probe()
    -  MSS_SYS_debug_live_probe()
    -  MSS_SYS_debug_select_mem()
    -  MSS_SYS_debug_read_mem()
    -  MSS_SYS_debug_write_mem()
    -  MSS_SYS_debug_read_apb()
    -  MSS_SYS_debug_write_apb()
    -  MSS_SYS_debug_fabric_snapshot()
    -  MSS_SYS_otp_generate()
    -  MSS_SYS_otp_match()
    -  MSS_SYS_unlock_debug_passcode()
    -  MSS_SYS_one_way_passcode()
    -  MSS_SYS_debug_terminate()

  -----------------------------------------------------------------------------
  Modes of operation
  -----------------------------------------------------------------------------
  The PolarFire SoC MSS system service driver can be configured to execute
  service in interrupt mode or polling mode. Users need to select the mode of
  operation by configuring the driver with appropriate service mode macros as a
  parameter to MSS_SYS_select_service_mode() function.
  In interrupt mode, the function will be non-blocking and the calling service
  function exits after requesting the system service with a success return value.
  The actual response from the system controller will only be available after 
  the interrupt occurs. Use the MSS_SYS_read_response() function to read the 
  service response and the status response code.
  In Polling mode, the function call will be blocking until the service completes
  and the calling service function exits only after the completion of the service. 
  The return value in this case will indicate the service response code received 
  from the system controller.
  -----------------------------------------------------------------------------
  Status response
  -----------------------------------------------------------------------------
  All the service execution functions return the 16-bit status returned by
  system controller on executing the given service. A zero value indicates the
  successful execution of that service. A non-zero value indicates an error code
  representing the type of error that was encountered while executing the service.
  Irrespective of the mode, if the controller is busy executing the previous
  service the function will exit with the MSS_SYS_BUSY return value. The error
  codes are different for each service. See individual function descriptions to
  know the meaning of the error code for each service.

  -----------------------------------------------------------------------------
  Reference document
  -----------------------------------------------------------------------------
  The function descriptions in this file will mainly focus on details required
  by the user to use the APIs provided by this driver to execute the services.
  To know the complete details of the system services, please refer to the
  PolarFire® FPGA and PolarFire SoC FPGA System Services document. Link below:
  https://onlinedocs.microchip.com/pr/GUID-1409CF11-8EF9-4C24-A94E-70979A688632-en-US-3/index.html
*/

#ifndef MSS_SYS_SERVICES_H_
#define MSS_SYS_SERVICES_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------Public constants----------------------------*/

/*-------------------------------------------------------------------------*//**
  System services Generic constants
  ============================

  These constants are used to communicate the outcome of a system services
  request. These status codes are used across all types of services. The
  following table lists the system service driver generic constants.

  ## MSS_SYS_SUCCESS
    System service executed successfully.

  ## MSS_SYS_BUSY
    system controller is busy executing system service which was initiated using
    its AMBA interface.

  ## MSS_SYS_PARAM_ERR
    System service cannot be executed as one or more parameters are not as
    expected by this driver.

*/
#define MSS_SYS_SUCCESS                                       0u
#define MSS_SYS_BUSY                                          0xEFu
#define MSS_SYS_PARAM_ERR                                     0xFFu

/*-------------------------------------------------------------------------*//**
  System service execution mode macros
  ============================

  The following defines are used in MSS_SYS_service_mode() function to select
  whether to execute services in interrupt mode or polling mode.

  ## MSS_SYS_SERVICE_INTERRUPT_MODE
     Parameter used in MSS_SYS_service_mode() function to execute the services in
     interrupt mode

  ## MSS_SYS_SERVICE_POLLING_MODE
     Parameter used in MSS_SYS_service_mode() function to execute the services in
     polling mode
 */
#define MSS_SYS_SERVICE_INTERRUPT_MODE                          1u
#define MSS_SYS_SERVICE_POLLING_MODE                            0u

/*-------------------------------------------------------------------------*//**
  System service error codes
  ============================

  The following constants list the success/error code for each system service.
*/

/*-------------------------------------------------------------------------*//**
  ## Device Certificate Service error codes

  ### MSS_SYS_DCF_DEVICE_MISMATCH
    Public key or FSN do not match device

  ### MSS_SYS_DCF_INVALID_SIGNATURE
    Certificate signature is invalid

  ### MSS_SYS_DCF_SYSTEM_ERROR
    PUF or storage failure
*/
#define MSS_SYS_DCF_DEVICE_MISMATCH                            1u
#define MSS_SYS_DCF_INVALID_SIGNATURE                          2u
#define MSS_SYS_DCF_SYSTEM_ERROR                               3u

/*------------------------------------------------------------------------*//**
  ## Read ENVM parameters service error codes

  ### MSS_SYS_ENVM_DIGEST_ERROR
  Page digest mismatches. Parameter values still returned
*/
#define MSS_SYS_ENVM_DIGEST_ERROR                              1u

/*-------------------------------------------------------------------------*//**
  ## Bitstream Authentication and IAP Bitstream Authentication Error Codes

  ### BSTREAM_AUTH_CHAINING_MISMATCH_ERR
    Validator or hash chaining mismatch. Incorrectly constructed bitstream or
    wrong key used.

  ### BSTREAM_AUTH_UNEXPECTED_DATA_ERR
    Unexpected data received.
    Additional data received after end of EOB component

  ### BSTREAM_AUTH_INVALID_ENCRY_KEY_ERR
    Invalid/corrupt encryption key.
    The requested key mode is disabled or the key could not be read/reconstructed

  ### BSTREAM_AUTH_INVALID_HEADER_ERR
    Invalid component header

  ### BSTREAM_AUTH_BACK_LEVEL_NOT_SATISFIED_ERR
    Back level not satisfied

  ### BSTREAM_AUTH_ILLEGAL_BITSTREAM_MODE_ERR
    Illegal bitstream mode.
    Requested bitstream mode is disabled by user security

  ### BSTREAM_AUTH_DNS_BINDING_MISMATCH_ERR
    DSN binding mismatch

  ### BSTREAM_AUTH_ILLEGAL_COMPONENT_SEQUENCE_ERR
    Illegal component sequence

  ### BSTREAM_AUTH_INSUFF_DEVICE_CAPAB_ERR
    Insufficient device capabilities

  ### BSTREAM_AUTH_INCORRECT_DEVICEID_ERR
    Incorrect DEVICEID

  ### BSTREAM_AUTH_PROTOCOL_VERSION_ERR
    Unsupported bitstream protocol version (regeneration required)

  ### BSTREAM_AUTH_VERIFY_ERR
    Verify not permitted on this bitstream

  ### BSTREAM_AUTH_INVALID_DEV_CERT_ERR
    Invalid Device Certificate.
    Device SCAC is invalid or not present

  ### BSTREAM_AUTH_INVALID_DIB_ERR
    Invalid DIB

  ### BSTREAM_AUTH_SPI_NOT_MASTER_ERR
    Device not in SPI Master Mode.
    Error may occur only when bitstream is executed through IAP mode

  ### BSTREAM_AUTH_AUTOIAP_NO_VALID_IMAGE_ERR
    No valid images found.
    Error may occur when bitstream is executed through Auto Update mode.
    Occurs when No valid image pointers are found.

  ### BSTREAM_AUTH_INDEXIAP_NO_VALID_IMAGE_ERR
    No valid images found.
    Error may occur when bitstream is executed through IAP mode via Index Mode.
    Occurs when No valid image pointers are found.

  ### BSTREAM_AUTH_NEWER_DESIGN_VERSION_ERR
    Programmed design version is newer than AutoUpdate image found.
    Error may occur when bitstream is executed through Auto Update mode

  ### BSTREAM_AUTH_INVALID_IMAGE_ERR
    Selected image was invalid and no recovery was performed due to valid design
    in device.
    Error may occur only when bitstream is executed through Auto Update or IAP
    mode (This error is here for completeness but only can be observed by
    running the READ_DEBUG_INFO instruction and looking at IAP Error code field)

  ### BSTREAM_AUTH_IMAGE_PROGRAM_FAILED_ERR
    Selected and Recovery image failed to program.
    Error may occur only when bitstream is executed through Auto Update or
    IAP mode
    (This error is here for completeness but only can be observed by running the
    READ_DEBUG_INFO instruction and looking at IAP Error code field)

  ### BSTREAM_AUTH_ABORT_ERR
    Abort.
    Non-bitstream instruction executed during bitstream loading.

  ### BSTREAM_AUTH_NVMVERIFY_ERR
    Fabric/UFS verification failed (min or weak limit)

  ### BSTREAM_AUTH_PROTECTED_ERR
    Device security prevented modification of non-volatile memory

  ### BSTREAM_AUTH_NOTENA
    Programming mode not enabled

  ### BSTREAM_AUTH_PNVMVERIFY
    pNVM verify operation failed

  ### BSTREAM_AUTH_SYSTEM
    System hardware error (PUF or DRBG)

  ### BSTREAM_AUTH_BADCOMPONENT
    An internal error was detected in a component payload

  ### BSTREAM_AUTH_HVPROGERR
    HV programming subsystem failure (pump failure)

  ### BSTREAM_AUTH_HVSTATE
    HV programming subsystem in unexpected state (internal error)
*/
#define MSS_SYS_BSTREAM_AUTH_CHAINING_MISMATCH_ERR                1u
#define MSS_SYS_BSTREAM_AUTH_UNEXPECTED_DATA_ERR                  2u
#define MSS_SYS_BSTREAM_AUTH_INVALID_ENCRY_KEY_ERR                3u
#define MSS_SYS_BSTREAM_AUTH_INVALID_HEADER_ERR                   4u
#define MSS_SYS_BSTREAM_AUTH_BACK_LEVEL_NOT_SATISFIED_ERR         5u
#define MSS_SYS_BSTREAM_AUTH_ILLEGAL_BITSTREAM_MODE_ERR           6u
#define MSS_SYS_BSTREAM_AUTH_DNS_BINDING_MISMATCH_ERR             7u
#define MSS_SYS_BSTREAM_AUTH_ILLEGAL_COMPONENT_SEQUENCE_ERR       8u
#define MSS_SYS_BSTREAM_AUTH_INSUFF_DEVICE_CAPAB_ERR              9u
#define MSS_SYS_BSTREAM_AUTH_INCORRECT_DEVICEID_ERR               10u
#define MSS_SYS_BSTREAM_AUTH_PROTOCOL_VERSION_ERR                 11u
#define MSS_SYS_BSTREAM_AUTH_VERIFY_ERR                           12u
#define MSS_SYS_BSTREAM_AUTH_INVALID_DEV_CERT_ERR                 13u
#define MSS_SYS_BSTREAM_AUTH_INVALID_DIB_ERR                      14u
#define MSS_SYS_BSTREAM_AUTH_SPI_NOT_MASTER_ERR                   21u
#define MSS_SYS_BSTREAM_AUTH_AUTOIAP_NO_VALID_IMAGE_ERR           22u
#define MSS_SYS_BSTREAM_AUTH_INDEXIAP_NO_VALID_IMAGE_ERR          23u
#define MSS_SYS_BSTREAM_AUTH_NEWER_DESIGN_VERSION_ERR             24u
/*25            Reserved*/
#define MSS_SYS_BSTREAM_AUTH_INVALID_IMAGE_ERR                    26u
#define MSS_SYS_BSTREAM_AUTH_IMAGE_PROGRAM_FAILED_ERR             27u
#define MSS_SYS_BSTREAM_AUTH_ABORT_ERR                            127u
#define MSS_SYS_BSTREAM_AUTH_NVMVERIFY_ERR                        128u
#define MSS_SYS_BSTREAM_AUTH_PROTECTED_ERR                        129u
#define MSS_SYS_BSTREAM_AUTH_NOTENA                               130u
#define MSS_SYS_BSTREAM_AUTH_PNVMVERIFY                           131u
#define MSS_SYS_BSTREAM_AUTH_SYSTEM                               132u
#define MSS_SYS_BSTREAM_AUTH_BADCOMPONENT                         133u
#define MSS_SYS_BSTREAM_AUTH_HVPROGERR                            134u
#define MSS_SYS_BSTREAM_AUTH_HVSTATE                              135u

/*-------------------------------------------------------------------------*//**
 ## Digital Signature Service error code

  ### DIGITAL_SIGNATURE_FEK_FAILURE_ERROR
    Error retrieving FEK

  ### DIGITAL_SIGNATURE_DRBG_ERROR
    Failed to generate nonce

  ### DIGITAL_SIGNATURE_ECDSA_ERROR
    ECDSA failed
*/
#define MSS_SYS_DIGITAL_SIGNATURE_FEK_FAILURE_ERROR             0x01u
#define MSS_SYS_DIGITAL_SIGNATURE_DRBG_ERROR                    0x02u
#define MSS_SYS_DIGITAL_SIGNATURE_ECDSA_ERROR                   0x03u

/*-------------------------------------------------------------------------*//**
  ## Secure NVM write error codes

  ### SNVM_WRITE_INVALID_SNVMADDR
    Illegal page address

  ### SNVM_WRITE_FAILURE
    PNVM program/verify failed

  ### SNVM_WRITE_SYSTEM_ERROR
    PUF or storage failure

  ### SNVM_WRITE_NOT_PERMITTED
    Write is not permitted
*/
#define MSS_SYS_SNVM_WRITE_INVALID_SNVMADDR                     1u
#define MSS_SYS_SNVM_WRITE_FAILURE                              2u
#define MSS_SYS_SNVM_WRITE_SYSTEM_ERROR                         3u
#define MSS_SYS_SNVM_WRITE_NOT_PERMITTED                        4u

/*-------------------------------------------------------------------------*//**
  ## Secure NVM read error codes

  ### SNVM_READ_INVALID_SNVMADDR
    Illegal page address

  ### SNVM_READ_AUTHENTICATION_FAILURE
    Storage corrupt or incorrect USK

  ### SNVM_READ_SYSTEM_ERROR
    PUF or storage failure
*/
#define MSS_SYS_SNVM_READ_INVALID_SNVMADDR                      1u
#define MSS_SYS_SNVM_READ_AUTHENTICATION_FAILURE                2u
#define MSS_SYS_SNVM_READ_SYSTEM_ERROR                          3u

/*-------------------------------------------------------------------------*//**
   ## PUF emulation service error codes

   ### MSS_SYS_PUF_EMU_INTERNAL_ERR
       Internal error
 */
#define MSS_SYS_PUF_EMU_INTERNAL_ERR                            1u

/*-------------------------------------------------------------------------*//**
  ## Nonce Service Error Codes

  ### MSS_SYS_NONCE_PUK_FETCH_ERROR
    Error fetching PUK

  ### MSS_SYS_NONCE_SEED_GEN_ERROR
    Error generating seed
*/
#define MSS_SYS_NONCE_PUK_FETCH_ERROR                          1u
#define MSS_SYS_NONCE_SEED_GEN_ERROR                           2u

/*-------------------------------------------------------------------------*//**
  ## Digest Check service error code

  ### MSS_SYS_DIGEST_CHECK_DIGESTERR
    Digest mismatch occurred
*/
#define MSS_SYS_DIGEST_CHECK_DIGESTERR                            1u

/*-------------------------------------------------------------------------*//**
  ## SPI COPY SERVICE error codes

  ### MSS_SYS_SPI_MASTER_MODE_ERR
    Device is not configured for master mode

  ### MSS_SYS_SPI_AXI_ERR
    AXI error
*/
#define MSS_SYS_SPI_MASTER_MODE_ERR                               1u
#define MSS_SYS_SPI_AXI_ERR                                       2u

/*-------------------------------------------------------------------------*//**
  ## Probe services error codes

  ### MSS_SYS_PROBE_SECERR
  The operation was blocked by device security.  This will occur if the
  permanent debug lock UP_DEBUG is set or the user software debug lock
  SWL_DEBUG is active or the device is in the virgin state. No data is read
  and PRDATA is invalid.
 */
#define MSS_SYS_PROBE_SECERR                                      1u

/*-------------------------------------------------------------------------*//**
  ## MEM Services error codes

  ### MSS_SYS_MEM_SECERR
  The operation was blocked by device security.
  This will occur if the permanent debug lock UP_DEBUG is set or the user
  software debug lock SWL_DEBUG is active or the device is in the virgin state.

  ### MSS_SYS_MEM_TIMEOUTERR
  Timeout occurred.

  ### MSS_SYS_MEM_LOCKERR
  Target memory failed to lock
*/
#define MSS_SYS_MEM_SECERR                                        1u
#define MSS_SYS_MEM_TIMEOUTERR                                    2u
#define MSS_SYS_MEM_LOCKERR                                       3u

/*-------------------------------------------------------------------------*//**
  ## APB services error codes

  ### MSS_SYS_APB_SECERR
  The operation was blocked by device security.
  This will occur if the permanent debug lock UP_DEBUG is set or the user
  software debug lock SWL_DEBUG is active or the device is in the virgin state.

  ### MSS_SYS_APB_SLVERR
  The addressed fabric APB peripheral generated a SLVERR response to the bus
  transaction.

  ### MSS_SYS_APB_TIMEOUT
  The addressed fabric APB peripheral failed to respond before the user-defined
  APB timeout or the fabric power is not on.
*/
#define MSS_SYS_APB_SECERR                                        1u
#define MSS_SYS_APB_SLVERR                                        2u
#define MSS_SYS_APB_TIMEOUT                                       3u

/*-------------------------------------------------------------------------*//**
  ## Debug snapshot service error codes

   ### MSS_SYS_DEBUG_SNAPSHOT_SECERR
   The operation was blocked by device security.
   This will occur if the permanent debug lock UP_DEBUG is set or the user
   software debug lock SWL_DEBUG is active or the device is in the virgin state.

   ### MSS_SYS_DEBUG_SNAPSHOT_BUSERR
   A bus error occurred and the snapshot was aborted.  This may occur if:
       •   the fabric power is off, or
       •   the fabric APB slave flagged an error, or
       •   the fabric APB slave was too slow to assert PREADY
*/
#define MSS_SYS_DEBUG_SNAPSHOT_SECERR                             1u
#define MSS_SYS_DEBUG_SNAPSHOT_BUSERR                             2u

/*-------------------------------------------------------------------------*//**
  ## GENERATE OTP SERVICE

  ### MSS_SYS_SECERR
  Operation is blocked by device security

  ### MSS_SYS_PROTOCOLERR
  Invalid key provided
*/
#define MSS_SYS_GENERATE_OTP_SECERR                               1u
#define MSS_SYS_GENERATE_OTP_PROTOCOLERR                          2u

/*-------------------------------------------------------------------------*//**
  ## MATCH OTP SERVICE

  ### MSS_SYS_PROTOCOLERR
  Keymode not supported.

  ### MSS_SYS_MATCH_OTP_MISMATCHERR
  Calculated validator mismatch.
*/
#define MSS_SYS_MATCH_OTP_PROTOCOLERR                             1u
#define MSS_SYS_MATCH_OTP_MISMATCHERR                             2u

/*-------------------------------------------------------------------------*//**
  ## Unlock debug passcode service error codes

  ### MSS_SYS_UNLOCK_DEBUG_PASSCODE_SECERR
  The operation was blocked by device security.
  Occurs if the lock UL_PLAINTEXT is active or the permanent lock UP_DPK is set.

  ### MSS_SYS_UNLOCK_DEBUG_PASSCODE_ERR
  If the unlock operation fails for any reason then the tamper event
  PASSCODE_FAIL is generated and all unlocked passcodes are re-locked.
*/
#define MSS_SYS_UNLOCK_DEBUG_PASSCODE_SECERR                     1u
#define MSS_SYS_UNLOCK_DEBUG_PASSCODE_ERR                        2u

/*-------------------------------------------------------------------------*//**
  ## One way passcode service error codes

  ### MSS_SYS_OWP_OWPERR
  If the unlock operation fails for any reason then the tamper event
  PASSCODE_FAIL is generated and all unlocked passcodes are re-locked.
*/
#define MSS_SYS_OWP_OWPERR                                       1u

/*-------------------------------------------------------------------------*//**
  System service response data length
  ============================


  The following constants can be used to indicate the length of the data that
  is written into the mailbox by the system controller in response to the
  service being requested.

  ## MSS_SYS_NO_RESPONSE_LEN
    This constant is used to indicate that system controller does not return any
    mailbox data for the service which is being requested

  ## MSS_SYS_SERIAL_NUMBER_RESP_LEN
    Response length serial number service

  ## MSS_SYS_USERCODE_RESP_LEN
    Response length for Usercode service

  ## MSS_SYS_DESIGN_INFO_RESP_LEN
    Response length for Design info service

  ## MSS_SYS_DEVICE_CERTIFICATE_RESP_LEN
    Response length for Device certificate service

  ## MSS_SYS_READ_DIGEST_RESP_LEN
    Response length Read digest service

  ## MSS_SYS_QUERY_SECURITY_RESP_LEN
    Response length Query security service

  ## MSS_SYS_READ_DEBUG_INFO_RESP_LEN
    Response length Read debug info service

  ## MSS_SYS_NONCE_SERVICE_RESP_LEN
    Response length Nonce service

  ## MSS_SYS_READ_ENVM_PARAM_RESP_LEN
    Response length Read eNVM parameters service

  ## MSS_SYS_PROBE_READ_SERVICE_RESP_LEN
    Response length Probe read service

  ## MSS_SYS_GENERATE_OTP_RESP_LEN
    Response length Generate OTP service

  ## MSS_SYS_PUF_EMULATION_SERVICE_RESP_LEN
    Response length PUF emulation service

  ## MSS_SYS_DIGITAL_SIGNATURE_RAW_FORMAT_RESP_SIZE
    Response length for digital signature service raw format

  ## MSS_SYS_DIGITAL_SIGNATURE_DER_FORMAT_RESP_SIZE
    Response length for digital signature service DER format
*/
#define MSS_SYS_NO_RESPONSE_LEN                                 0u
#define MSS_SYS_SERIAL_NUMBER_RESP_LEN                          16u
#define MSS_SYS_USERCODE_RESP_LEN                               4u
#define MSS_SYS_DESIGN_INFO_RESP_LEN                            36u
#define MSS_SYS_DEVICE_CERTIFICATE_RESP_LEN                     1024u
#define MSS_SYS_READ_DIGEST_RESP_LEN                            576u
#define MSS_SYS_QUERY_SECURITY_RESP_LEN                         33u
#define MSS_SYS_READ_DEBUG_INFO_RESP_LEN                        94u
#define MSS_SYS_NONCE_SERVICE_RESP_LEN                          32u
#define MSS_SYS_READ_ENVM_PARAM_RESP_LEN                        256u
#define MSS_SYS_PUF_EMULATION_SERVICE_RESP_LEN                  32u
#define MSS_SYS_DIGEST_CHECK_SERVICE_RESP_LEN                   4u
#define MSS_SYS_DIGITAL_SIGNATURE_RAW_FORMAT_RESP_SIZE          96u
#define MSS_SYS_DIGITAL_SIGNATURE_DER_FORMAT_RESP_SIZE          104u
#define MSS_SYS_USER_SECRET_KEY_LEN                             12u
#define MSS_SYS_PROBE_READ_SERVICE_RESP_LEN                     4u
#define MSS_SYS_GENERATE_OTP_RESP_LEN                           16u

/*-------------------------Private constants--------------------------------*/

/*-------------------------------------------------------------------------*//**
  Service request command opcodes
  ============================

  The following constants can be used as parameter value of the functions to
  indicate the system service command opcode.
 */

/*-------------------------------------------------------------------------*//**
  Device and design information services request command opcodes
 */
 /// @cond private
#define MSS_SYS_SERIAL_NUMBER_REQUEST_CMD                       0x00u
#define MSS_SYS_USERCODE_REQUEST_CMD                            0x01u
#define MSS_SYS_DESIGN_INFO_REQUEST_CMD                         0x02u
#define MSS_SYS_DEVICE_CERTIFICATE_REQUEST_CMD                  0x03u
#define MSS_SYS_READ_DIGEST_REQUEST_CMD                         0x04u
#define MSS_SYS_QUERY_SECURITY_REQUEST_CMD                      0x05u
#define MSS_SYS_READ_DEBUG_INFO_REQUEST_CMD                     0x06u
#define MSS_SYS_READ_ENVM_PARAM_REQUEST_CMD                     0x07u

/*-------------------------------------------------------------------------*//**
  Design services request command opcodes
*/
#define MSS_SYS_BITSTREAM_AUTHENTICATE_CMD                      0x23u
#define MSS_SYS_IAP_BITSTREAM_AUTHENTICATE_CMD                  0x22u

/*-------------------------------------------------------------------------*//**
  Data security services request command opcodes
*/
#define MSS_SYS_DIGITAL_SIGNATURE_RAW_FORMAT_REQUEST_CMD        0x19u
#define MSS_SYS_DIGITAL_SIGNATURE_DER_FORMAT_REQUEST_CMD        0x1Au
#define MSS_SYS_SNVM_NON_AUTHEN_TEXT_REQUEST_CMD                0x10u
#define MSS_SYS_SNVM_AUTHEN_TEXT_REQUEST_CMD                    0x11u
#define MSS_SYS_SNVM_AUTHEN_CIPHERTEXT_REQUEST_CMD              0x12u
#define MSS_SYS_SNVM_READ_REQUEST_CMD                           0x18u
#define MSS_SYS_PUF_EMULATION_SERVICE_REQUEST_CMD               0x20u
#define MSS_SYS_NONCE_SERVICE_REQUEST_CMD                       0x21u

/*-------------------------------------------------------------------------*//**
  Fabric services request command opcodes
*/
#define MSS_SYS_DIGEST_CHECK_CMD                                0x47u
#define MSS_SYS_IAP_PROGRAM_BY_SPIIDX_CMD                       0x42u
#define MSS_SYS_IAP_VERIFY_BY_SPIIDX_CMD                        0x44u
#define MSS_SYS_IAP_PROGRAM_BY_SPIADDR_CMD                      0x43u
#define MSS_SYS_IAP_VERIFY_BY_SPIADDR_CMD                       0x45u
#define MSS_SYS_IAP_AUTOUPDATE_CMD                              0x46u

/*-------------------------------------------------------------------------*//**
  MSS services request command opcodes
*/
#define MSS_SYS_SPI_COPY_CMD                                    0X50U
#define MSS_SYS_PROBE_READ_DEBUG_CMD                            0X70U
#define MSS_SYS_PROBE_WRITE_DEBUG_CMD                           0X71U
#define MSS_SYS_LIVE_PROBE_A_DEBUG_CMD                          0X72U
#define MSS_SYS_LIVE_PROBE_B_DEBUG_CMD                          0X73U
#define MSS_SYS_MEM_SELECT_DEBUG_CMD                            0X74U
#define MSS_SYS_MEM_READ_DEBUG_CMD                              0X75U
#define MSS_SYS_MEM_WRITE_DEBUG_CMD                             0X76U
#define MSS_SYS_APB_READ_DEBUG_CMD                              0X77U
#define MSS_SYS_APB_WRITE_DEBUG_CMD                             0X78U
#define MSS_SYS_DEBUG_SNAPSHOT_CMD                              0X79U
#define MSS_SYS_GENERATE_OTP_CMD                                0X7AU
#define MSS_SYS_MATCH_OTP_CMD                                   0X7BU
#define MSS_SYS_UNLOCK_DEBUG_PASSCODE                           0X7CU
#define MSS_SYS_ONE_WAY_PASSCODE_CMD                            0X7DU
#define MSS_SYS_TERMINATE_DEBUG_CMD                             0X7EU

/*-------------------------------------------------------------------------*//**
  System service mailbox data length
  ============================

  The following constants are used to specify the mailbox data length of each
  service for the service that is being requested.
*/

/* This constant is used for the services where no mailbox input data is
 * required
 */
#define MSS_SYS_WITHOUT_CMD_DATA                                0u

#define MSS_SYS_PUF_EMULATION_SERVICE_CMD_LEN                   20u
#define MSS_SYS_DIGITAL_SIGNATURE_HASH_DATA_LEN                 48u

/*SNVMADDR + RESERVED + PT*/
#define MSS_SYS_AUTHENTICATED_TEXT_DATA_LEN                     252u

/*SNVMADDR + RESERVED + PT + USK*/
#define MSS_SYS_NON_AUTHENTICATED_TEXT_DATA_LEN                 256u

#define MSS_SYS_SECURE_NVM_READ_DATA_LEN                        16u
#define MSS_SYS_BITSTREAM_AUTHENTICATE_DATA_LEN                 4u
#define MSS_SYS_DIGEST_CHECK_DATA_LEN                           4u
#define MSS_SYS_IAP_SERVICE_DATA_LEN                            4u
#define MSS_SYS_SPI_COPY_MAILBOX_DATA_LEN                       17u
#define MSS_SYS_PROBE_READ_SERVICE_DATA_LEN                     2u
#define MSS_SYS_PROBE_WRITE_SERVICE_DATA_LEN                    11u
#define MSS_SYS_LIVE_PROBE_DEBUG_SERVICE_DATA_LEN               6u
#define MSS_SYS_MEM_SELECT_DATA_LEN                             6u
#define MSS_SYS_MEM_READ_WRITE_DATA_LEN                         12u
#define MSS_SYS_APB_SERVICE_DATA_LEN                            24u
#define MSS_SYS_DEBUG_SNAPSHOT_DATA_LEN                         5u
#define MSS_SYS_GENERATE_OTP_DATA_LEN                           20u
#define MSS_SYS_MATCH_OTP_DATA_LEN                              80u
#define MSS_SYS_UNLOCK_DEBUG_PASSCODE_DATA_LEN                  32u
#define MSS_SYS_ONE_WAY_PASSCODE_DATA_LEN                       480u

/*-------------------------------------------------------------------------*//**
  System Services mailbox data constants
  ============================
 */

/* KEY MODE for Generate OTP service
  KM_USER_KEY1      USER Key 1
  KM_USER_KEY2      USER Key 2
  KM_FACTORY_KEY    FK Diversified by UID
 */
#define MSS_SYS_KM_USER_KEY1                                    3u
#define MSS_SYS_KM_USER_KEY2                                    4u
#define MSS_SYS_KM_FACTORY_KEY                                  7u

/*Digest Check Input options
  DIGEST_CHECK_FABRIC
    Carry out digest check on Fabric

  DIGEST_CHECK_CC
    Carry out digest check on UFS Fabric Configuration (CC) segment

  DIGEST_CHECK_SNVM
    Carry out digest check on ROM digest in SNVM segment

  DIGEST_CHECK_UL
    Carry out digest check on UFS UL segment

  DIGEST_CHECK_UKDIGEST0
    Carry out digest check on UKDIGEST0 in User Key segment

  DIGEST_CHECK_UKDIGEST1
    Carry out digest check on UKDIGEST1 in User Key segment

  DIGEST_CHECK_UKDIGEST2
    Carry out digest check on UKDIGEST2 in User Key segment (UPK1)

  DIGEST_CHECK_UKDIGEST3
    Carry out digest check on UKDIGEST3 in User Key segment (UK1)

  DIGEST_CHECK_UKDIGEST4
    Carry out digest check on UKDIGEST4 in User Key segment (DPK)

  DIGEST_CHECK_UKDIGEST5
    Carry out digest check on UKDIGEST5 in User Key segment (UPK2)

  DIGEST_CHECK_UKDIGEST6
    Carry out digest check on UKDIGEST6 in User Key segment (UK2)

  DIGEST_CHECK_UPERM
    Carry out digest check on UFS Permanent lock (UPERM) segment

  DIGEST_CHECK_SYS
    Carry out digest check on Factory and Factory Key Segments.
*/
#define MSS_SYS_DIGEST_CHECK_FABRIC                           (0x01<<0x00u)
#define MSS_SYS_DIGEST_CHECK_CC                               (0x01<<0x01u)
#define MSS_SYS_DIGEST_CHECK_SNVM                             (0x01<<0x02u)
#define MSS_SYS_DIGEST_CHECK_UL                               (0x01<<0x03u)
#define MSS_SYS_DIGEST_CHECK_UKDIGEST0                        (0x01<<0x04u)
#define MSS_SYS_DIGEST_CHECK_UKDIGEST1                        (0x01<<0x05u)
#define MSS_SYS_DIGEST_CHECK_UKDIGEST2                        (0x01<<0x06u)
#define MSS_SYS_DIGEST_CHECK_UKDIGEST3                        (0x01<<0x07u)
#define MSS_SYS_DIGEST_CHECK_UKDIGEST4                        (0x01<<0x08u)
#define MSS_SYS_DIGEST_CHECK_UKDIGEST5                        (0x01<<0x09u)
#define MSS_SYS_DIGEST_CHECK_UKDIGEST6                        (0x01<<0x0au)
#define MSS_SYS_DIGEST_CHECK_UPERM                            (0x01<<0x0bu)
#define MSS_SYS_DIGEST_CHECK_SYS                              (0x01<<0x0cu)
#define MSS_SYS_DIGEST_CHECK_UKDIGEST7                        (0x01<<0x0du)
#define MSS_SYS_DIGEST_CHECK_ENVM                             (0x01<<0x0eu)
#define MSS_SYS_DIGEST_CHECK_UKDIGEST8                        (0x01<<0x0fu)
#define MSS_SYS_DIGEST_CHECK_UKDIGEST9                        (0x01<<0x10u)
#define MSS_SYS_DIGEST_CHECK_UKDIGEST10                       (0x01<<0x11u)

/*-------------------------------------------------------------------------*//**
  Mailbox ECC status
  Provides ECC status when the mailbox is read. The values are as follows:
    00: No ECC errors detected, data is correct.
    01: Exactly one bit erred and has been corrected.
    10: Exactly two bits erred, no correction performed.
    11: Reserved.
*/
#define MSS_SYS_MBOX_ECC_NO_ERROR_MASK                         0x00u
#define MSS_SYS_MBOX_ONEBIT_ERROR_CORRECTED_MASK               0x40u
#define MSS_SYS_MBOX_TWOBIT_ERROR_MASK                         0xC0u

/*-------------------------------------------------------------------------*//**
 * Options for system services
*/
/*   Permitted key modes for one way Pass-code service
 *   *NS -- Not Supported
 */
#define KM_INIT_FACTORY                                         0x00u/*NS*/
#define KM_ZERO_RECOVERY                                        0x01u/*NS*/
#define KM_DEFAULT_KEY                                          0x02u
#define KM_USER_KEY1                                            0x03u
#define KM_USER_KEY2                                            0x04u
#define KM_AUTH_CODE                                            0x06u/*NS*/
#define KM_FACTORY_KEY                                          0x07u
#define KM_FACTORY_EC                                           0x08u/*NS*/
#define KM_FACTORY_EC_E                                         0x09u/*NS*/
#define KM_USER_EC                                              0x12u/*NS*/
#define KM_USER_EC_E                                            0x13u/*NS*/
/// @endcond
/*-------------------------------------------------------------------------*//**
  Callback function handler
  The callback handler is used by the application to indicate the user about
  the event of interrupt when the driver is configured to execute the system
  services in interrupt mode.
  The callback function handler is is registered to the MSS system service
  driver through the call to MSS_SYS_select_service_mode() function.
  The actual name of callback function handler is not important. User can select
  any name.
 */
typedef void (*mss_sys_service_handler_t)(void);

/*-------------------------------------------------------------------------*//**
  The function MSS_SYS_read_response() is used to read the response after
  execution of system service in interrupt mode only. For polling mode call to
  MSS_SYS_read_response is not required, as the drive performs the response
  read operation.
  @param void
         This function does not have any parameters.
  @return
         This function returns the status code returned by the system controller
         for requested service.

  @example
  @code
       status = MSS_SYS_read_response();

  @endcode
 */
uint16_t
MSS_SYS_read_response
(
    void
);

/*-------------------------------------------------------------------------*//**
  The MSS_SYS_service_mode() function is for user to configure system service
  execution in polling mode or interrupt mode. This function also registers the
  callback handler to the driver which will be called when message interrupt
  occurs.

  @param sys_service_mode
                    User can decide whether to execute the service in polling
                    mode or interrupt mode.
                    Example:
                            MSS_SYS_SERVICE_INTERRUPT_MODE
                            MSS_SYS_SERVICE_POLLING_MODE

  @param mss_sys_service_interrupt_handler
                    Callback function to the application. This function is
                    invoked when message interrupt occurs.

  @return
        This function does not return any value.

  @example
  @code
       MSS_SYS_service_mode(MSS_SYS_SERVICE_POLLING_MODE,
                            mss_sys_service_interrupt_handler);
  @endcode
*/
void
MSS_SYS_select_service_mode
(
    uint8_t sys_service_mode,
    mss_sys_service_handler_t mss_sys_service_interrupt_handler
);

/*-------------------------------------------------------------------------*//**
  The MSS_SYS_get_serial_number() function is used to execute the device serial
  number service.

  Please refer to theory of operation -> reference documents section for more 
  information about the service.

  @param p_serial_number
                    The p_serial_number parameter is a pointer to a buffer
                    in which the 128-bit data returned by system controller will
                    be stored.

  @param mb_offset
                    The mb_offset parameter specifies the offset from the start
                    of mailbox where the data related to this service is
                    available. All accesses to the mailbox are of word length
                    (4 bytes). A value 10 (decimal) of this parameter would
                    mean that the data access area for this service, in the
                    mailbox starts from 11th word (offset 10).
  @return
                    This function returns the status code returned by the
                    system controller for this service. A '0' status code means
                    that the service was executed successfully.
*/
uint16_t
MSS_SYS_get_serial_number
(
    uint8_t * p_serial_number,
    uint16_t mb_offset
);

/*-------------------------------------------------------------------------*//**
  The function MSS_SYS_get_user_code() is used to execute "USERCODE" system
  service.

  Please refer to theory of operation -> reference documents section for more 
  information about the service.

  @param p_user_code
                    The p_user_code parameter is a pointer to a buffer
                    in which the 32-bit data returned by system controller will
                    be stored.
  @param mb_offset
                    The mb_offset parameter specifies the offset from the start
                    of mailbox where the data related to this service is
                    available. All accesses to the mailbox are of word length
                    (4 bytes). A value 10 (decimal) of this parameter would
                    mean that the data access area for this service, in the
                    mailbox starts from 11th word (offset 10).
  @return
                    This function returns the status code returned by the
                    system controller for this service. A '0' status code means
                    that the service was executed successfully.
*/
uint16_t
MSS_SYS_get_user_code
(
    uint8_t * p_user_code,
    uint16_t mb_offset
);

/*-------------------------------------------------------------------------*//**
  The function MSS_SYS_get_design_info() is used to execute "Get Design Info"
  system service.

  Please refer to theory of operation -> reference documents section for more 
  information about the service.

  @param p_design_info
                     The p_design_info parameter is a pointer to a buffer
                     in which the data returned by system controller will be
                     copied. Total size of debug information is 36 bytes.
                     The data from the system controller includes the 256-bit
                     user-defined design ID, 16-bit design version and 16-bit
                     design back level.
  @param mb_offset
                    The mb_offset parameter specifies the offset from the start
                    of mailbox where the data related to this service is
                    available. All accesses to the mailbox are of word length
                    (4 bytes). A value 10 (decimal) of this parameter would
                    mean that the data access area for this service, in the
                    mailbox starts from 11th word (offset 10).
  @return
                    This function returns a value to indicate whether the
                    service was executed successfully or not. A zero value
                    indicates that the service was executed successfully.
*/
uint16_t
MSS_SYS_get_design_info
(
    uint8_t * p_design_info,
    uint16_t mb_offset
);

/*-------------------------------------------------------------------------*//**
  The function MSS_SYS_get_device_certificate() is used to execute "Get Device
  Certificate" system service.

  Please refer to theory of operation -> reference documents section for more 
  information about the service.

  @param p_device_certificate
                    The p_device_certificate parameter is a pointer to a buffer
                    in which the data returned by the system controller will be
                    stored.
  @param mb_offset
                    The mb_offset parameter specifies the offset from the start
                    of mailbox where the data related to this service is
                    available. All accesses to the mailbox are of word length
                    (4 bytes). A value 10 (decimal) of this parameter would
                    mean that the data access area for this service, in the
                    mailbox starts from 11th word (offset 10).
  @return
                    This function returns a value to indicate whether the
                    service was executed successfully or not. A zero value
                    indicates that the service was executed successfully.

                    Please refer to the document link provided in the theory of
                    operation section to know more about the service and service
                    response
*/
uint16_t
MSS_SYS_get_device_certificate
(
    uint8_t * p_device_certificate,
    uint16_t mb_offset
);

/*-------------------------------------------------------------------------*//**
  The function MSS_SYS_read_digest() is used to execute "Read Digest" system
  service.

  Please refer to theory of operation -> reference documents section for more 
  information about the service.

  @param p_digest
                    The p_digest parameter is a pointer to a buffer
                    in which the data returned by system controller will be
                    copied.
  @param mb_offset
                    The mb_offset parameter specifies the offset from the start
                    of mailbox where the data related to this service is
                    available. All accesses to the mailbox are of word length
                    (4 bytes). A value 10 (decimal) of this parameter would
                    mean that the data access area for this service, in the
                    mailbox starts from 11th word (offset 10).
  @return
                    This function returns a value to indicate whether the
                    service was executed successfully or not. A zero value
                    indicates that the service was executed successfully.
*/
uint16_t
MSS_SYS_read_digest
(
   uint8_t * p_digest,
   uint16_t mb_offset
);

/*-------------------------------------------------------------------------*//**
  The function MSS_SYS_query_security() is used to execute "Query Security"
  system service.

  Please refer to theory of operation -> reference documents section for more 
  information about the service.

  @param p_security_locks
                    The p_security_locks parameter is a pointer to a buffer
                    in which the data returned by system controller will be
                    copied.
  @param mb_offset
                    The mb_offset parameter specifies the offset from the start
                    of mailbox where the data related to this service is
                    available. All accesses to the mailbox are of word length
                    (4 bytes). A value 10 (decimal) of this parameter would
                    mean that the data access area for this service, in the
                    mailbox starts from 11th word (offset 10).
  @return
                    This function returns a value to indicate whether the
                    service was executed successfully or not. A zero value
                    indicates that the service was executed successfully.
*/
uint16_t
MSS_SYS_query_security
(
    uint8_t * p_security_locks,
    uint16_t mb_offset
);

/*-------------------------------------------------------------------------*//**
  The function MSS_SYS_read_debug_info() is used to execute "Read Debug info"
  system service.

  Please refer to theory of operation -> reference documents section for more 
  information about the service.

  @param p_debug_info
                    The p_debug_info parameter is a pointer to a buffer
                    in which the data returned by system controller will be
                    copied.
  @param mb_offset
                    The mb_offset parameter specifies the offset from the start
                    of mailbox where the data related to this service is
                    available. All accesses to the mailbox are of word length
                    (4 bytes). A value 10 (decimal) of this parameter would
                    mean that the data access area for this service, in the
                    mailbox starts from 11th word (offset 10).
  @return
                    This function returns a value to indicate whether the
                    service was executed successfully or not. A zero value
                    indicates that the service was executed successfully.
*/
uint16_t
MSS_SYS_read_debug_info
(
    uint8_t * p_debug_info,
    uint16_t mb_offset
);

/*-------------------------------------------------------------------------*//**
  The function MSS_SYS_read_envm_parameter() is used to retrieve all parameters
  needed for eNVM operation and programming.

  Please refer to theory of operation -> reference documents section for more 
  information about the service.

  @param p_envm_param
                    The p_envm_param parameter specifies the the user buffer
                    which will be accumulated with envm parameters after the
                    service completes execution.
  @param mb_offset
                    The mb_offset parameter specifies the offset from the start
                    of mailbox where the data related to this service is
                    available. All accesses to the mailbox are of word length
                    (4 bytes). A value 10 (decimal) of this parameter would
                    mean that the data access area for this service, in the
                    mailbox starts from 11th word (offset 10).
  @return
                    This function returns a value to indicate whether the
                    service was executed successfully or not. A zero value
                    indicates that the service was executed successfully.
*/
uint16_t
MSS_SYS_read_envm_parameter
(
    uint8_t * p_envm_param,
    uint16_t mb_offset
);

/*-------------------------------------------------------------------------*//**
  The MSS_SYS_authenticate_bitstream() function is used to authenticate
  the Bitstream which is located in SPI through a system service routine. Prior
  to using the IAP service, it may be required to first validate the new
  bitstream before committing the device to reprogramming, thus avoiding the
  need to invoke recovery procedures if the bitstream is invalid.

  This service is applicable to bitstreams stored in SPI Flash memory only.

  Please refer to theory of operation -> reference documents section for more 
  information about the service.

  @param spi_flash_address
                    The spi_flash_address parameter specifies the address within
                    SPI Flash memory where the bit-stream is stored.
  @param mb_offset
                    The mb_offset parameter specifies the offset from the start
                    of mailbox where the data related to this service is
                    available. All accesses to the mailbox are of word length
                    (4 bytes). A value 10 (decimal) of this parameter would
                    mean that the data access area for this service, in the
                    mailbox starts from 11th word (offset 10).
  @return
                    This function returns a value to indicate whether the
                    service was executed successfully or not. A zero value
                    indicates that the service was executed successfully.
                    Please refer to theory of operation -> status response and
                    reference documents section for more information about the
                    service.
*/
uint16_t
MSS_SYS_authenticate_bitstream
(
    uint32_t spi_flash_address,
    uint16_t mb_offset
);

/*-------------------------------------------------------------------------*//**
  The MSS_SYS_authenticate_iap_image() function is used to authenticate
  the IAP image which is located in SPI through a system service routine. The
  service checks the image descriptor and the referenced bitstream and optional
  initialization data.  If the image is authenticated successfully, then the
  image is guaranteed to be valid when used by an IAP function.

  This service is applicable to bitstreams stored in SPI Flash memory only.

  Please refer to theory of operation -> reference documents section for more 
  information about the service.

  @param spi_idx
                    The spi_idx parameter specifies the index in the SPI
                    directory to be used where the IAP bit-stream is stored.

                    Note: To support recovery SPI_IDX=1 should be an empty slot
                    and the recovery image should be located in SPI_IDX=0. Since
                    SPI_IDX=1 should be an empty slot it shouldn’t be passed into
                    the system service.
  @return
                   The MSS_SYS_authenticate_iap_image function returns a value
                   to indicate whether the service was executed successfully or
                   not. A zero value indicates that the service was executed
                   successfully.

                   Please refer to theory of operation -> status response and
                   reference documents section for more information about the
                   service.
*/
uint16_t
MSS_SYS_authenticate_iap_image
(
    uint32_t spi_idx
);

/*-------------------------------------------------------------------------*//**
  The MSS_SYS_puf_emulation_service() function accept a challenge comprising a
  8-bit optype and 128-bit challenge and return a 256-bit response unique to
  the given challenge and the device.

  Please refer to theory of operation -> reference documents section for more 
  information about the service.

  @param p_challenge
                    The p_challenge parameter specifies the 128-bit challenge
                    to be used to generate the unique 256-bits unique
                    response.
  @param op_type
                    The op_type parameter specifies the operational parameter
                    to be used to generate the unique 256-bits unique
                    response.
  @param p_response
                    The p_response parameter is a pointer to a buffer in
                    which the data returned i.e. response by system controller
                    will be copied.
  @param mb_offset
                    The mb_offset parameter specifies the offset from the start
                    of mailbox where the data related to this service is
                    available. All accesses to the mailbox are of word length
                    (4 bytes). A value 10 (decimal) of this parameter would
                    mean that the data access area for this service, in the
                    mailbox starts from 11th word (offset 10).
  @return
                    This function returns a value to indicate whether the
                    service was executed successfully or not. A zero value
                    indicates that the service was executed successfully.

                    Please refer to theory of operation -> status response and
                    reference documents section for more information about the
                    service.

*/
uint16_t
MSS_SYS_puf_emulation_service
(
    uint8_t * p_challenge,
    uint8_t op_type,
    uint8_t* p_response,
    uint16_t mb_offset
);

/*-------------------------------------------------------------------------*//**
  The MSS_SYS_digital_signature_service() function is used to generate P-384
  ECDSA signature based on SHA384 hash value.

  Please refer to theory of operation -> reference documents section for more 
  information about the service.

  @param p_hash
                    The p_hash parameter is a pointer to the buffer which
                    contain the 48 bytes SHA384 Hash value(input value).
  @param format
                    The format parameter specifies the output format of
                    generated SIGNATURE field. The different types of output
                    signature formats are as follow:
                     - DIGITAL_SIGNATURE_RAW_FORMAT
                     - DIGITAL_SIGNATURE_DER_FORMAT
  @param p_response
                    The p_response parameter is a pointer to a buffer which
                    contain the generated ECDSA signature. The field may be
                    96 bytes or 104 bytes depending upon the output format.
  @param mb_offset
                    The mb_offset parameter specifies the offset from the start
                    of mailbox where the data related to this service is
                    available. All accesses to the mailbox are of word length
                    (4 bytes). A value 10 (decimal) of this parameter would
                    mean that the data access area for this service, in the
                    mailbox starts from 11th word (offset 10).
  @return
                    This function returns a value to indicate whether the
                    service was executed successfully or not. A zero value
                    indicates that the service was executed successfully.

                    Please refer to theory of operation -> status response and
                    reference documents section for more information about the
                    service.
*/
uint16_t
MSS_SYS_digital_signature_service
(
    uint8_t* p_hash,
    uint8_t format,
    uint8_t* p_response,
    uint16_t mb_offset
);

/*-------------------------------------------------------------------------*//**
  The MSS_SYS_secure_nvm_write() function is used to provide write access/write the
  data in the sNVM region. Data can be stored in the following format:
   -  Non-authenticated plaintext,
   -  Authenticated plaintext
   -  Authenticated ciphertext

  Please refer to theory of operation -> reference documents section for more 
  information about the service.

  @param format
                    The format parameter specifies the format used to write
                    data in sNVM region. The different type of text formats
                    are as follow:
                     - NON_AUTHENTICATED_PLAINTEXT_FORMAT
                     - AUTHENTICATED_PLAINTEXT_FORMAT
                     - AUTHENTICATED_CIPHERTEXT_FORMAT
  @param snvm_module
                    The snvm_module parameter specifies the the sNVM module
                    in which the data need to be written.
  @param p_data
                    The p_data parameter is a pointer to a buffer which
                    contains the data to be stored in sNVM region. The data length
                    to be written is if fixed depending on the format parameter.
                    If NON_AUTHENTICATED_PLAINTEXT_FORMAT is selected then you
                    can write 252 bytes in the sNVM module. For other two formats
                    the data length is 236 bytes.
  @param p_user_key
                    The p_user_key parameter is a pointer to a buffer which
                    contain the 96-bit key USK (user secret key). This user
                    secret key will enhance the security when authentication
                    is used.(i.e. When Authenticated plaintext and
                    Authenticated ciphertext format is selected).
  @param mb_offset
                    The mb_offset parameter specifies the offset from the start
                    of mailbox where the data related to this service is
                    available. All accesses to the mailbox are of word length
                    (4 bytes). A value 10 (decimal) of this parameter would
                    mean that the data access area for this service, in the
                    mailbox starts from 11th word (offset 10).
  @return
                    This function returns a value to indicate whether the
                    service was executed successfully or not. A zero value
                    indicates that the service was executed successfully.

                    Please refer to theory of operation -> status response and
                    reference documents section for more information about the
                    service.
*/
uint16_t
MSS_SYS_secure_nvm_write
(
    uint8_t format,
    uint8_t snvm_module,
    uint8_t* p_data,
    uint8_t* p_user_key,
    uint16_t mb_offset
);

/*-------------------------------------------------------------------------*//**
  The MSS_SYS_secure_nvm_read() function is used to read data present in sNVM
  region. User should provide USK key, if the data was programmed using
  authentication.

  Please refer to theory of operation -> reference documents section for more 
  information about the service.

  @param snvm_module
                    The snvm_module parameter specifies the sNVM module
                    from which the data need to be read.
  @param p_user_key
                    The p_user_key parameter is a pointer to a buffer which
                    contain the 96-bit key USK (user secret key). User should
                    provide same secret key which is previously used for
                    authentication while writing data in sNVM region.
  @param p_admin
                    The p_admin parameter is a pointer to the buffer where
                    the output page admin data will be stored. The page admin
                    data is 4 bytes long.
  @param p_data
                    The p_data parameter is a pointer to a buffer which
                    contains the data read from sNVM region. User should
                    provide the buffer large enough to store the read data.
  @param data_len
                    The data_len parameter specifies the number of bytes to be
                    read from sNVM.
                    The application should know whether the data written in the
                    chose sNVM module was previously stored using Authentication
                    or not.
                    The data_len should be 236 bytes, for authenticated data,
                    for not authenticated data the data_len should be 252 bytes.
  @param mb_offset
                    The mb_offset parameter specifies the offset from the start
                    of mailbox where the data related to this service is
                    available. All accesses to the mailbox are of word length
                    (4 bytes). A value 10 (decimal) of this parameter would
                    mean that the data access area for this service, in the
                    mailbox starts from 11th word (offset 10).
  @return
                    This function returns a value to indicate whether the
                    service was executed successfully or not. A zero value
                    indicates that the service was executed successfully.
*/
uint16_t
MSS_SYS_secure_nvm_read
(
    uint8_t snvm_module,
    uint8_t* p_user_key,
    uint8_t* p_admin,
    uint8_t* p_data,
    uint16_t data_len,
    uint16_t mb_offset
);

/*-------------------------------------------------------------------------*//**
  The function MSS_SYS_nonce_service() is used to issue "Nonce Service" system
  service to the system controller.

  Please refer to theory of operation -> reference documents section for more 
  information about the service.

  @param p_nonce
                    The p_nonce parameter is a pointer to a buffer
                    in which the data returned by system controller will be
                    copied.
  @param mb_offset
                    The mb_offset parameter specifies the offset from the start
                    of mailbox where the data related to this service is
                    available. All accesses to the mailbox are of word length
                    (4 bytes). A value 10 (decimal) of this parameter would
                    mean that the data access area for this service, in the
                    mailbox starts from 11th word (offset 10).
  @return           This function returns the status code returned by the
                    system controller for this service. A '0' status code means
                    that the service was executed successfully.
*/
uint16_t
MSS_SYS_nonce_service
(
    uint8_t * p_nonce,
    uint16_t mb_offset
);

/*-------------------------------------------------------------------------*//**
  The MSS_SYS_digest_check() function is used to Recalculates and compares
  digests of selected non-volatile memories.

  This service is applicable to bitstream stored in SPI Flash memory only.

  Please refer to theory of operation -> reference documents section for more 
  information about the service.

  Information : parameter options
  | Options[i]  |    Description                              |
  |-------------|---------------------------------------------|
  |   0x01      |   Fabric digest                             |
  |   0x02      |   Fabric Configuration (CC) segment         |
  |   0x04      |   ROM digest in SNVM segment                |
  |   0x08      |   UL segment                                |
  |   0x10      |   UKDIGEST0 in User Key segment             |
  |   0x20      |   UKDIGEST1 in User Key segment             |
  |   0x40      |   UKDIGEST2 in User Key segment (UPK1)      |
  |   0x80      |   UKDIGEST3 in User Key segment (UK1)       |
  |   0x100     |   UKDIGEST4 in User Key segment (DPK)       |
  |   0x200     |   UKDIGEST5 in User Key segment (UPK2)      |
  |   0x400     |   UKDIGEST6 in User Key segment (UK2)       |
  |   0x800     |   UFS Permanent lock (UPERM) segment        |
  |   0x1000    |   Factory and Factory Key Segments.         |
  |   0x2000    |   UKDIGEST7 in User Key segment (HWM)       |
  |   0x4000    |   ENVMDIGEST                                |
  |   0x8000    |   UKDIGEST8 for MSS Boot Info               |
  |   0x10000   |   SNVM_RW_ACCESS_MAP Digest                 |
  |   0x20000   |   SBIC revocation digest                    |

  Information : parameter digesterr 
  | DIGESTERR[i]|    Description                              |
  |-------------|---------------------------------------------|
  |   0x01      |   Fabric digest                             |
  |   0x02      |   Fabric Configuration (CC) segment         |
  |   0x04      |   ROM digest in SNVM segment                |
  |   0x08      |   UL segment                                |
  |   0x10      |   UKDIGEST0 in User Key segment             |
  |   0x20      |   UKDIGEST1 in User Key segment             |
  |   0x40      |   UKDIGEST2 in User Key segment (UPK1)      |
  |   0x80      |   UKDIGEST3 in User Key segment (UK1)       |
  |   0x100     |   UKDIGEST4 in User Key segment (DPK)       |
  |   0x200     |   UKDIGEST5 in User Key segment (UPK2)      |
  |   0x400     |   UKDIGEST6 in User Key segment (UK2)       |
  |   0x800     |   UFS Permanent lock (UPERM) segment        |
  |   0x1000    |   Factory and Factory Key Segments.         |
  |   0x2000    |   UKDIGEST7 in User Key segment (HWM)       |
  |   0x4000    |   ENVMDIGEST                                |
  |   0x8000    |   UKDIGEST8 for MSS Boot Info               |
  |   0x10000   |   SNVM_RW_ACCESS_MAP Digest                 |
  |   0x20000   |   SBIC revocation digest                    |

  @param options
               The options parameter specifies the digest check options which
               indicate the area on which the digest check should be performed.
               Below is the list of options. You can OR these options to indicate
               to perform digest check on multiple segments. Please refer 
               function description for more information of options parameter.

  @param mb_offset
                    The mb_offset parameter specifies the offset from the start
                    of mailbox where the data related to this service is
                    available. All accesses to the mailbox are of word length
                    (4 bytes). A value 10 (decimal) of this parameter would
                    mean that the data access area for this service, in the
                    mailbox starts from 11th word (offset 10).
  @param digesterr
                    The digesterr parameter specifies the set bit in case of
                    DIGESTERR. Please refer function description for more 
                    information of digesterr parameter.

  @return
                    This function returns a value to indicate whether the
                    service was executed successfully or not. A zero value
                    indicates that the service was executed successfully.
*/
uint16_t
MSS_SYS_digest_check
(
    uint32_t options,
    uint8_t* digesterr,
    uint16_t mb_offset
);

/*-------------------------------------------------------------------------*//**
  The MSS_SYS_execute_iap() function is used to IAP service. The IAP service
  allows the user to reprogram the device without the need for an external
  master. The user design writes the bitstream to be programmed into a SPI Flash
  connected to the SPI port.  When the service is invoked, the System Controller
  automatically reads the bitstream from the SPI flash and programs the device.
  The service allows the image to be executed in either VERIFY or PROGRAM modes.
  Another option for IAP is to perform the auto-update sequence. In this case
  the newest image of the first two images in the SPI directory is chosen to be
  programmed.

  Please refer to theory of operation -> reference documents section for more 
  information about the service.

  Information : parameter iap_cmd 
  |     iap_cmd                 |  Description                     |
  |-----------------------------|----------------------------------|
  | IAP_PROGRAM_BY_SPIIDX_CMD   |  IAP program.                    |
  | IAP_VERIFY_BY_SPIIDX_CMD    |  Fabric Configuration (CC) segment|
  | IAP_PROGRAM_BY_SPIADDR_CMD  |  ROM digest in SNVM segment      |
  | IAP_VERIFY_BY_SPIADDR_CMD   |  UL segment                      |
  | IAP_AUTOUPDATE_CMD          |  UKDIGEST0 in User Key segment   |

  Information : parameter spiaddr 
  |         iap_cmd              |        spiaddr                 |
  |------------------------------|--------------------------------|
  | IAP_PROGRAM_BY_SPIIDX_CMD    |Index in the SPI directory.     |
  | IAP_VERIFY_BY_SPIIDX_CMD     |Index in the SPI directory.     |
  | IAP_PROGRAM_BY_SPIADDR_CMD   |SPI address in the SPI Flash memory
  | IAP_VERIFY_BY_SPIADDR_CMD    |SPI address in the SPI Flash memory
  | IAP_AUTOUPDATE_CMD           |spiaddr is ignored as No index  |
  |                               address required for this com
  Note: For the IAP services with command IAP_PROGRAM_BY_SPIIDX_CMD
       and IAP_VERIFY_BY_SPIIDX_CMD To support recovery SPI_IDX = 1
       should be an empty slot and the recovery image should be
       located in SPI_IDX = 0. Since SPI_IDX = 1 should be an empty
       slot it shouldn’t be passed into the system service.

  @param iap_cmd
               The iap_cmd parameter specifies the specific IAP command which
               depends upon VERIFY or PROGRAM modes and the SPI address method.
               Please refer function description for more information of 
               iap_cmd parameter.
  @param spiaddr
               The spiaddr parameter specifies the either the either the index
               in the SPI directory or the SPI address in the SPI Flash memory.
               Below is the list of the possible meaning of spiaddr parameter
               in accordance with the iap_cmd parameter. Please refer function 
               description for more information of spiaddr parameter.
  @param mb_offset
                    The mb_offset parameter specifies the offset from the start
                    of mailbox where the data related to this service is
                    available. All accesses to the mailbox are of word length
                    (4 bytes). A value 10 (decimal) of this parameter would
                    mean that the data access area for this service, in the
                    mailbox starts from 11th word (offset 10).
  @return
                    This function returns a value to indicate whether the
                    service was executed successfully or not. A zero value
                    indicates that the service was executed successfully.

                    Please refer to theory of operation -> status response and
                    reference documents section for more information about the
                    service.
*/
uint16_t
MSS_SYS_execute_iap
(
    uint8_t iap_cmd,
    uint32_t spiaddr,
    uint16_t mb_offset
);

/*-------------------------------------------------------------------------*//**
  The MSS_SYS_spi_copy() function allows data to be copied from the system
  controller SPI flash to MSS memory. The SPI SCK frequency is specified by a
  user-defined option with valid values shown in parameter description.

  Please refer to theory of operation -> reference documents section for more 
  information about the service.

  Information : parameter options
  |options |Clock     |
  |--------|----------|
  | 1      | 40MHz    |
  | 2      | 20MHz    |
  | 3      | 13.33MHz |
  7:2 RESERVED        Reserved for future use

  @param mss_dest_addr
                    The 64-bit mss_dest_addr parameter specifies the destination
                    address in MSS where system controller copies data from SPI
                    flash.
  @param mss_spi_flash
                    The 32-bit mss_spi_flash parameter specifies the source
                    address of data to be copied in MSS.
  @param n_bytes
                    The n_bytes parameter specifies the number of bytes to
                    transfer.
  @param options
                    The 8 bit options parameter specifies the clock frequency
                    used for the SPI transfers.
                    Please refer function description for more information 
                    of options parameter.
  @param mb_offset
                    The mb_offset parameter specifies the offset from the start
                    of mailbox where the data related to this service is
                    available. All accesses to the mailbox are of word length
                    (4 bytes). A value 10 (decimal) of this parameter would
                    mean that the data access area for this service, in the
                    mailbox starts from 11th word (offset 10).
  @return
                    This function returns a value to indicate whether the
                    service was executed successfully or not. A zero value
                    indicates that the service was executed successfully.
*/
uint16_t
MSS_SYS_spi_copy
(
    uint64_t mss_dest_addr,
    uint32_t mss_spi_flash,
    uint32_t n_bytes,
    uint8_t options,
    uint16_t mb_offset
);

/*-------------------------------------------------------------------------*//**
  The MSS_SYS_debug_read_probe() function will read the content of a
  probe module (59 x 18b words).

  Please refer to theory of operation -> reference documents section for more 
  information about the service.

  Note: The IPROWADDR & IPSEGADDR addresses are not incremented as the
        associated address space is not contiguous. If PRBADDR is 63 it will
        wrap back to 0.

  @param ipseg_addr
                    The ipseg_addr parameter specifies the probe segment
                    address.
  @param iprow_addr
                    The iprow_addr parameter specifies the probe row address.
                    ipseg_addr and iprow_addr parameters specifies the target
                    address of probe module.
  @param prdata
                    The prdata parameter specifies the read data for probe
                    word i(0 to 58) within the probe module.
  @param mb_offset
                    The mb_offset parameter specifies the offset from the start
                    of mailbox where the data related to this service is
                    available. All accesses to the mailbox are of word length
                    (4 bytes). A value 10 (decimal) of this parameter would
                    mean that the data access area for this service, in the
                    mailbox starts from 11th word (offset 10).
  @param resp_offset
                    The resp_offset parameter specifies the offset of the
                    start of Mailbox response where the data received from
                    the service will be available.
  @return
                    This function returns a value to indicate whether the
                    service was executed successfully or not. A zero value
                    indicates that the service was executed successfully.
*/
uint16_t
MSS_SYS_debug_read_probe
(
    uint8_t ipseg_addr,
    uint8_t iprow_addr,
    uint8_t *prdata,
    uint16_t mb_offset,
    uint8_t resp_offset
);

/*-------------------------------------------------------------------------*//**
  The MSS_SYS_debug_write_probe() function will issue the probe write debug
  service to the system controller. It service will writes up to 18 bits of data
  to selected probe address.

  Please refer to theory of operation -> reference documents section for more 
  information about the service.

  @param prb_addr
                    The prb_addr parameter specifies the probe address.
  @param ipseg_addr
                    The ipseg_addr parameter specifies the probe segment
                    address.
  @param iprow_addr
                    The iprow_addr parameter specifies the probe row address.
                    prb_addr, ipseg_addr and iprow_addr parameters specifies
                    the target address of one of the 59 words within a probe
                    module.
  @param pwmask
                    The pwmask parameter specifies the which of the 18 bits of
                    pwdata shall be written to selected probe module.
  @param pwdata
                    The pwdata parameter specifies the value to be written on
                    selected probe registers.
                    Example: If PWMASK[i] is ‘1’ then probe register i will
                    be written to the value specified by PWDATA[i].
  @param mb_offset
                    The mb_offset parameter specifies the offset from the start
                    of mailbox where the data related to this service is
                    available. All accesses to the mailbox are of word length
                    (4 bytes). A value 10 (decimal) of this parameter would
                    mean that the data access area for this service, in the
                    mailbox starts from 11th word (offset 10).
  @return
                    This function returns a value to indicate whether the
                    service was executed successfully or not. A zero value
                    indicates that the service was executed successfully.
*/
uint16_t
MSS_SYS_debug_write_probe
(
    uint8_t prb_addr,
    uint8_t ipseg_addr,
    uint8_t iprow_addr,
    uint32_t pwmask,
    uint32_t pwdata,
    uint16_t mb_offset
);

/*-------------------------------------------------------------------------*//**
  The MSS_SYS_debug_live_probe() function will issue the 'Live Probe Debug Service'
  to the system controller. This service will configures channel 'a' or
  'b' of the live probe system.

  Note:
   - When configuring channel 'a', channel 'b' is not affected and vice versa.
   - Live probes are intentionally not cleared by JTAG reset.
     They remain in effect until either manually disabled or the device is
     reset.

  Please refer to theory of operation -> reference documents section for more 
  information about the service.

  @param x_addr
                    The parameter x_addr specifies the x coordinate within
                    target probe module.
  @param y_addr
                    The parameter y_addr specifies the y coordinate within
                    the target probe module.
  @param ipseg_addr
                    The ipseg_addr parameter specifies the probe segment
                    address.
  @param iprow_addr
                    The iprow_addr parameter specifies the probe row address.
                    ipseg_addr and iprow_addr parameters specifies the target
                    address of probe module.
  @param clear
                    The clear parameter(of size 1-bit) is used to clear the
                    configurations of local channels a or b. If CLEAR is ‘1’,
                    all local channel x (the applicable channel a or b)
                    configurations are cleared before applying the new
                    configuration
  @param ioen
                    The ioen parameter(of size 1-bit) is used to activate
                    the probe output pad. If IOEN is ‘1’ then the corresponding
                    live probe output pad is activated.  Note that setting IOEN
                    to ‘0’ does not disable the internal live probe
                    configuration.
  @param mb_offset
                    The mb_offset parameter specifies the offset from the start
                    of mailbox where the data related to this service is
                    available. All accesses to the mailbox are of word length
                    (4 bytes). A value 10 (decimal) of this parameter would
                    mean that the data access area for this service, in the
                    mailbox starts from 11th word (offset 10).
  @param service_cmd
                    The service_cmd parameter specifies the channel(channel
                    a or b) selected by the user. User have to provide one of
                    the predefined macros to select the channel for live probe
                    debug operation.
  @return
                    This function returns a value to indicate whether the
                    service was executed successfully or not. A zero value
                    indicates that the service was executed successfully.
*/
uint16_t
MSS_SYS_debug_live_probe
(
    uint8_t x_addr,
    uint8_t y_addr,
    uint8_t ipseg_addr,
    uint8_t iprow_addr,
    uint8_t clear,
    uint8_t ioen,
    uint16_t mb_offset,
    uint8_t service_cmd
);

/*-------------------------------------------------------------------------*//**
  The MSS_SYS_debug_select_mem() function will issues the MEM Select Debug Service.
  This service will specify a target fabric memory to be accessed by the MEM
  read & MEM write services.

  Please refer to theory of operation -> reference documents section for more 
  information about the service.

  Information : parameter memtype 
  | MEMTYPE| Peripheral | MEMSIZE(words)| Access Width |
  |--------|------------|---------------|--------------|
  |     0  | LSRAM x1   | 16384         |     1        |
  |     1  | LSRAM x2   | 8192          |     2        |
  |     2  | LSRAM x5   | 4096          |     5        |
  |     3  | LSRAM x10  | 2048          |     10       |
  |     4  | LSRAM x20  | 1024          |     20       |
  |     5  | µRAM       | 64            |     12       |
  |     6  | µPROM      | 256           |     9        |
  |     7  | LSRAM x20  | 1024          |     20       |

  @param ipblk_addr
                    The ipblk_addr parameter specifies the block address of
                    fabric memory.
  @param ipseg_addr
                    The ipseg_addr parameter specifies the segment address.
  @param iprow_addr
                    The iprow_addr parameter specifies the row address of
                    fabric memory to be accessed by MEM read and MEM write
                    services.
  @param memtype
                    The memtype parameter specifies the type of fabric
                    memory to be used for MEM read and write services.
                    Please refer function description for more information 
                    of memtype parameter.

  @param memlock_mode
                    The memlock_mode parameter specifies the the memory
                    lock states for supported MEMLOCKMODE values.
  @param timeout
                    When a lock is requested we must consider a scenario
                    where the user design may not complete the request
                    handshake.  To prevent the firmware from waiting
                    indefinitely, the user must specify a timeout after
                    which time the handshake is aborted.
  @param mb_offset
                    The mb_offset parameter specifies the offset from the start
                    of mailbox where the data related to this service is
                    available. All accesses to the mailbox are of word length
                    (4 bytes). A value 10 (decimal) of this parameter would
                    mean that the data access area for this service, in the
                    mailbox starts from 11th word (offset 10).
  @return
                    This function returns a value to indicate whether the
                    service was executed successfully or not. A zero value
                    indicates that the service was executed successfully.
*/
uint16_t
MSS_SYS_debug_select_mem
(
    uint8_t ipblk_addr,
    uint8_t ipseg_addr,
    uint8_t iprow_addr,
    uint8_t memtype,
    uint8_t memlock_mode,
    uint16_t timeout,
    uint16_t mb_offset
);

/*-------------------------------------------------------------------------*//**
  The MSS_SYS_debug_read_mem() function issues the MEM Read Debug Service to the
  system controller. This service provides an interface to read data from the
  memory peripheral that is specified.

  Please refer to theory of operation -> reference documents section for more 
  information about the service.

  @param mem_addr
                    The mem_addr parameter sets the target address within the
                    currently selected memory peripheral for subsequent
                    mem write & mem read instructions.
  @param n_words
                    The n_words parameter value depends on memtype. The
                    maximum limit is the size of memory.
  @param mss_addr
                    The mss_addr parameter specifies the MSS RAM area where
                    to copy the MEM Read data to. Note that all accesses will
                    be done with MSS User privileges.
  @param mb_offset
                    The mb_offset parameter specifies the offset from the start
                    of mailbox where the data related to this service is
                    available. All accesses to the mailbox are of word length
                    (4 bytes). A value 10 (decimal) of this parameter would
                    mean that the data access area for this service, in the
                    mailbox starts from 11th word (offset 10).
  @return
                    This function returns a value to indicate whether the
                    service was executed successfully or not. A zero value
                    indicates that the service was executed successfully.
*/
uint16_t
MSS_SYS_debug_read_mem
(
    uint16_t mem_addr,
    uint16_t n_words,
    uint64_t mss_addr,
    uint16_t mb_offset
);

/*-------------------------------------------------------------------------*//**
  The MSS_SYS_debug_write_mem() function issues the MEM Write Debug Service to
  the system controller. This service provides an interface to write data from
  the memory peripheral that is specified.

  Please refer to theory of operation -> reference documents section for more 
  information about the service.

  @param mem_addr
                    The mem_addr parameter sets the target address within the
                    currently selected memory peripheral for subsequent
                    mem write & mem read instructions.
  @param n_words
                    The n_words parameter value depends on memtype. The
                    maximum limit is the size of memory.
  @param mss_addr
                    The mss_addr parameter specifies the MSS RAM area where
                    to copy the MEM Read data to. Note that all accesses will
                    be done with MSS User privileges.
  @param mb_offset
                    The mb_offset parameter specifies the offset from the start
                    of mailbox where the data related to this service is
                    available. All accesses to the mailbox are of word length
                    (4 bytes). A value 10 (decimal) of this parameter would
                    mean that the data access area for this service, in the
                    mailbox starts from 11th word (offset 10).
  @return
                    This function returns a value to indicate whether the
                    service was executed successfully or not. A zero value
                    indicates that the service was executed successfully.
*/
uint16_t
MSS_SYS_debug_write_mem
(
    uint16_t mem_addr,
    uint16_t n_words,
    uint64_t mss_addr,
    uint16_t mb_offset
);

/*-------------------------------------------------------------------------*//**
  The MSS_SYS_debug_read_apb() function issues the APB Read Debug Service to the
  system controller. This service will read a specified number of bytes from
  the fabric APB bus to the specified MSS RAM area.

  Please refer to theory of operation -> reference documents section for more 
  information about the service.

  Note: This service will return only the status of service execution. Actual
  data read by the system controller will be available at the specified MSS RAM
  area. It is expected that the application code should perform read operation
  to the specified MSS RAM area after successful execution of the service.

  @param apb_addr
                    The apb_addr parameter specifies the target address and
                    transfer size for the apb write & apb read operations.
  @param apb_wsize
                    The apb_wsize parameter specifies the data transfer size
                    to be used by the apb write & apb read operations.
  @param max_bytes
                    The max_bytes parameter is used in calculation specified
                    number of bytes from the fabric APB bus to the Shared
                    Buffer.
                    NBYTES = MAXBYTES + 1
  @param mss_addr
                    The mss_addr parameter specifies the MSS RAM area where
                    to copy the MEM Read data to. Note that all accesses will
                    be done with MSS User privileges.
  @param mb_offset
                    The mb_offset parameter specifies the offset from the start
                    of mailbox where the data related to this service is
                    available. All accesses to the mailbox are of word length
                    (4 bytes). A value 10 (decimal) of this parameter would
                    mean that the data access area for this service, in the
                    mailbox starts from 11th word (offset 10).
  @return
                    This function returns a value to indicate whether the
                    service was executed successfully or not. A zero value
                    indicates that the service was executed successfully.

                    Please refer to theory of operation -> status response and
                    reference documents section for more information about the
                    service.
*/
uint16_t
MSS_SYS_debug_read_apb
(
    uint32_t apb_addr,
    uint8_t  apb_wsize,
    uint16_t max_bytes,
    uint64_t mss_addr,
    uint16_t mb_offset
);

/*-------------------------------------------------------------------------*//**
  The MSS_SYS_debug_write_apb() function will issue the APB Write Debug Service.
  This service will write bytes of data to the current fabric APB address as
  specified by APBADDR.

  Please refer to theory of operation -> reference documents section for more 
  information about the service.

  @param apb_addr
                    The apb_addr parameter specifies the target address and
                    transfer size for the apb write & apb read operations.
  @param apb_wsize
                    The apb_wsize parameter specifies the data transfer size
                    to be used by the apb write & apb read operations.
  @param max_bytes
                    The max_bytes parameter is used in calculation specified
                    number of bytes from the fabric APB bus to the Shared
                    Buffer.
                    NBYTES = MAXBYTES + 1
  @param mss_addr
                    The mss_addr parameter specifies the MSS RAM area where
                    to copy the MEM Read data to. Note that all accesses will
                    be done with MSS User privileges.
  @param mb_offset
                    The mb_offset parameter specifies the offset from the start
                    of mailbox where the data related to this service is
                    available. All accesses to the mailbox are of word length
                    (4 bytes). A value 10 (decimal) of this parameter would
                    mean that the data access area for this service, in the
                    mailbox starts from 11th word (offset 10).
  @return
                    This function returns a value to indicate whether the
                    service was executed successfully or not. A zero value
                    indicates that the service was executed successfully.
*/
uint16_t
MSS_SYS_debug_write_apb
(
    uint32_t apb_addr,
    uint8_t  apb_wsize,
    uint16_t max_bytes,
    uint64_t mss_addr,
    uint16_t mb_offset
);

/*-------------------------------------------------------------------------*//**
  The MSS_SYS_debug_fabric_snapshot() will issue the Debug Snapshot Service to
  the system controller. This service generates a snapshot of the volatile
  fabric content. Data is read from each LSRAM, µRAM and probe module and copied
  to the fabric APB debug port.

  Please refer to theory of operation -> reference documents section for more 
  information about the service.

  @param port_addr
                    The port_addr parameter sets the address of the APB port
                    to be used for bulk access debug instructions which are
                    used in conjunction with Microchip fabric debug IP.
                    The debug port is a single location on the fabric APB
                    bus through which debug data is streamed.
  @param apb_fast_write
                    The apb_fast_write parameter specifies whether to use
                    the fast apb protocol. If apb_fast_write is ‘1’ then,
                    during write transfers, the fast APB protocol is used
                    and the address range is limited to port_addr[15:2]  and
                    port_addr[28:16] is ignored.
  @param mb_offset
                    The mb_offset parameter specifies the offset from the start
                    of mailbox where the data related to this service is
                    available. All accesses to the mailbox are of word length
                    (4 bytes). A value 10 (decimal) of this parameter would
                    mean that the data access area for this service, in the
                    mailbox starts from 11th word (offset 10).
  @return
                    This function returns a value to indicate whether the
                    service was executed successfully or not. A zero value
                    indicates that the service was executed successfully.
*/
uint16_t
MSS_SYS_debug_fabric_snapshot
(
    uint32_t port_addr,
    uint8_t apb_fast_write,
    uint16_t mb_offset
);

/*-------------------------------------------------------------------------*//**
  The MSS_SYS_otp_generate() function will issue the Generate OTP Service to the
  system controller. This service used to set up the device to receive a
  one-time passcode.

  Please refer to theory of operation -> reference documents section for more 
  information about the service.

  Information : parameter keymode 
  Supported values for KEYMODE are shown
  |PCTYPE| KEYMODE|  Key Mode      | KROOT |  Note              |
  |------|--------|----------------|-------|--------------------|
  |  1   |   3    |  KM_USER_KEY1  |  UK1  |  User key 1        |
  |  1   |   4    |  KM_USER_KEY2  |  UK2  |  User key 2        |
  |  1   |   7    | KM_FACTORY_KEY |  DFK  |FK diversified by UID|

  @param keymode
                   keymode parameter specifies the key mode to be used to
                   transport the encrypted passcode.
                   The KEYMODE parameter is not checked for validity until the
                   MATCH OTP service is executed.  Both PCTYPE and KEYMODE are
                   stored in volatile memory for use by the MATCH OTP service.
                   Please refer function description for more information of 
                   keymode parameter.
  @param n_user
                   The n_user parameter specifies the user nonce, is supplied
                   by the user.
  @param n_fpga
                    The n_fpga parameter specifies the 128-bit nonce, NFPGA, is
                    generated and stored in volatile memory for later use in the
                    rest of the protocol.
  @param mb_offset
                    The mb_offset parameter specifies the offset from the start
                    of mailbox where the data related to this service is
                    available. All accesses to the mailbox are of word length
                    (4 bytes). A value 10 (decimal) of this parameter would
                    mean that the data access area for this service, in the
                    mailbox starts from 11th word (offset 10).
  @param ret_offset
                    The ret_offset parameter specifies the offset of the
                    start of Mailbox response where the data received from
                    the service will be available.
  @return
                    This function returns a value to indicate whether the
                    service was executed successfully or not. A zero value
                    indicates that the service was executed successfully.
*/
uint16_t
MSS_SYS_otp_generate
(
    uint8_t keymode,
    uint8_t* n_user,
    uint8_t* n_fpga,
    uint16_t mb_offset,
    uint16_t resp_offset
);

/*-------------------------------------------------------------------------*//**
  The MSS_SYS_otp_match() function will issue the Match OTP Service to the
  system controller. This service is the second part of the one-time
  passcode protocol. Before using this service the GENERATE OTP service must
  first be used to obtain a nonce, NFPGA from the device.

  Please refer to theory of operation -> reference documents section for more 
  information about the service.

  @param user_id
                    The UID parameter is only used if the KEYMODE used for
                    the GENERATE OTP service was KEYMODE_FACTORY_KEY and the
                    passcode was not the Factory Passcode.
  @param validator
                    The 256-bit validator parameter store the validator key.
  @param otp
                    The otp parameter stores the otp value from generate otp
                    service.
  @param mb_offset
                    The mb_offset parameter specifies the offset from the start
                    of mailbox where the data related to this service is
                    available. All accesses to the mailbox are of word length
                    (4 bytes). A value 10 (decimal) of this parameter would
                    mean that the data access area for this service, in the
                    mailbox starts from 11th word (offset 10).
  @param resp_offset
                    The resp_offset parameter specifies the offset from the
                    start of Mailbox response where the data received from
                    the service will be available.
  @return
                    This function returns a value to indicate whether the
                    service was executed successfully or not. A zero value
                    indicates that the service was executed successfully.
*/
uint16_t
MSS_SYS_otp_match
(
    uint8_t * user_id,
    uint8_t * validator,
    uint8_t * otp,
    uint16_t mb_offset,
    uint16_t resp_offset
);

/*-------------------------------------------------------------------------*//**
  The MSS_SYS_unlock_debug_passcode() will issue the Unlock Debug Passcode
  Service to the system controller. This service will Attempt to match
  the user debug pass code using the key loaded into the mailbox.  If the match
  is successful the software debug lock SWL_DEBUG is temporarily inactive.

  Please refer to theory of operation -> reference documents section for more 
  information about the service.

  @param cmd_data
                    The parameter cmd_data specifies the device's debug
                    passcode (DPK), configured by the user via bitstream.
  @param mb_offset
                    The mb_offset parameter specifies the offset from the start
                    of mailbox where the data related to this service is
                    available. All accesses to the mailbox are of word length
                    (4 bytes). A value 10 (decimal) of this parameter would
                    mean that the data access area for this service, in the
                    mailbox starts from 11th word (offset 10).
  @param ret_offset
                    The ret_offset parameter specifies the offset from the
                    start of Mailbox response where the data received from
                    the service will be available.
  @return
                    This function returns a value to indicate whether the
                    service was executed successfully or not. A zero value
                    indicates that the service was executed successfully.

                    Please refer to theory of operation -> status response and
                    reference documents section for more information about the
                    service.
*/
uint16_t
MSS_SYS_unlock_debug_passcode
(
    uint8_t* cmd_data,
    uint16_t mb_offset,
    uint16_t resp_offset
);

/*-------------------------------------------------------------------------*//**
  The MSS_SYS_one_way_passcode() function is used to provide a
  mechanism for overriding the software debug lock  SWL_DEBUG without requiring
  any interaction with an external intelligence.

  Please refer to theory of operation -> reference documents section for more 
  information about the service.

  Information : parameter keymode 
  | KEYID|     Key Mode       |   Permitted |
  |------|--------------------|-------------|
  |    0 |  KM_INIT_FACTORY   |      No     |
  |    1 |  KM_ZERO_RECOVERY  |      No     |
  |    2 |  KM_DEFAULT_KEY    |      Yes    |
  |    3 |  KM_USER_KEY1      |      Yes    |
  |    4 |  KM_USER_KEY2      |      Yes    |
  |    5 |      -             |             |
  |    6 |  KM_AUTH_CODE      |      No     |
  |    7 |  KM_FACTORY_KEY    |      Yes    |
  |    8 |  KM_FACTORY_EC     |      No     |
  |    9 |  KM_FACTORY_EC_E   |      No     |
  |   10 |      -             |             |
  |   11 |      -             |             |
  |   12 |  KM_USER_EC        |      No     |
  |   13 |  KM_USER_EC_E      |      No     |
  |   14 |      -             |             |
  |   15 |      -             |             |

  @param msg_id
                    The msg_id parameter stores the value of message ID.
  @param validator
                    The 256-bit validator parameter store the validator key.
  @param keymode
                    The keymode parameter specifies the permitted keymodes for
                    OWP service. Please refer function description for more 
                    information of keymode parameter.
  @param dsn
                    The dsn parameter stores the value of device serial number.
  @param hash
                    The hash parameter stores 256-bit hash value.

  @param plaintext_passcode
                    The plaintext_passcode parameter stores the passcode value.
  @param hwm
                    The hwm parameter stores the high water mark value.
  @param mb_offset
                    The mb_offset parameter specifies the offset from the start
                    of mailbox where the data related to this service is
                    available. All accesses to the mailbox are of word length
                    (4 bytes). A value 10 (decimal) of this parameter would
                    mean that the data access area for this service, in the
                    mailbox starts from 11th word (offset 10).
  @param ret_offset
                    The ret_offset parameter specifies the offset from the
                    start of Mailbox response where the data received from
                    the service will be available.
  @return
                    This function returns a value to indicate whether the
                    service was executed successfully or not. A zero value
                    indicates that the service was executed successfully.

                    Please refer to theory of operation -> status response and
                    reference documents section for more information about the
                    service.
*/
uint16_t
MSS_SYS_one_way_passcode
(
    uint8_t *msg_id,
    uint8_t *validator,
    uint8_t *header,
    uint8_t *payload,
    uint8_t *tnext,
    uint16_t mb_offset,
    uint16_t resp_offset
);

/*-------------------------------------------------------------------------*//**
  The MSS_SYS_debug_terminate() function will issue the Terminate Debug Service
  to the system controller. This service will terminate the debug session.

  @param mb_offset
                    The mb_offset parameter specifies the offset from the start
                    of mailbox where the data related to this service is
                    available. All accesses to the mailbox are of word length
                    (4 bytes). A value 10 (decimal) of this parameter would
                    mean that the data access area for this service, in the
                    mailbox starts from 11th word (offset 10).
  @param resp_offset
                    The resp_offset parameter specifies the offset from the
                    start of Mailbox, where the data received from
                    the service will be available.
  @return
                    This function returns a value to indicate whether the
                    service was executed successfully or not. A zero value
                    indicates that the service was executed successfully.
*/
uint16_t
MSS_SYS_debug_terminate
(
    uint16_t mb_offset,
    uint16_t resp_offset
);

typedef struct
{
    volatile uint32_t SOFT_RESET;
    volatile uint32_t VDETECTOR;
    volatile uint32_t TVS_CONTROL;
    volatile uint32_t TVS_TEMP_A;
    volatile uint32_t TVS_TEMP_B;
    volatile uint32_t TVS_TEMP_C;
    volatile uint32_t TVS_VOLT_A;
    volatile uint32_t TVS_VOLT_B;
    volatile uint32_t TVS_VOLT_C;
    volatile uint32_t TVS_OUTPUT0;
    volatile uint32_t TVS_OUTPUT1;
    volatile uint32_t TVS_TRIGGER;
    volatile uint32_t TRIM_VDET1P05;
    volatile uint32_t TRIM_VDET1P8;
    volatile uint32_t TRIM_VDET2P5;
    volatile uint32_t TRIM_TVS;
    volatile uint32_t TRIM_GDET1P05;
    volatile uint32_t RESERVED0;
    volatile uint32_t RESERVED1;
    volatile uint32_t RESERVED2;
    volatile uint32_t SERVICES_CR;
    volatile uint32_t SERVICES_SR;
    volatile uint32_t USER_DETECTOR_SR;
    volatile uint32_t USER_DETECTOR_CR;
    volatile uint32_t MSS_SPI_CR;

} SCBCTRL_TypeDef;

#define MSS_SCBCTRL                    ((SCBCTRL_TypeDef*) (0x37020000UL))

/*2kB bytes long mailbox.*/
#define MSS_SCBMAILBOX                 ((uint32_t*) (0x37020800UL))

/*SCB message register*/
#define MSS_SCBMESSAGE                 ((uint32_t*) (0x20003190UL))

/*SCB message interrupt register*/
#define MSS_SCBMESSAGE_INT             ((uint32_t*) (0x2000318CUL))

#ifdef __cplusplus
}
#endif

#endif /* MSS_SYS_SERVICES_H_ */
