cmake_minimum_required(VERSION 3.22)

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

project(lionkey_nrf52840 C ASM)

set(NRFX  ${CMAKE_CURRENT_LIST_DIR}/vendor/nrfx)
set(CMSIS ${CMAKE_CURRENT_LIST_DIR}/vendor/CMSIS_6/CMSIS/Core/Include)
set(TUSB  ${CMAKE_SOURCE_DIR}/lib/tinyusb)

message("Build type: " ${CMAKE_BUILD_TYPE})

# Create an executable object type
add_executable(${PROJECT_NAME})

# TinyCBOR
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
# enabling this option will prevent adding src/cborparser_dup_string.c and src/cborpretty_stdio.c
# to the tinycbor target sources, but it will also add the -ffreestanding compile option
# that for some reason breaks the build, so we have to override it
# by adding the -fno-freestanding option further below (it will negate the -ffreestanding option)
set(WITH_FREESTANDING ON)
set(WITH_CBOR2JSON OFF)
set(WITH_FLOATING_POINT OFF)
add_subdirectory(../../lib/tinycbor ${CMAKE_BINARY_DIR}/lib/tinycbor)
target_compile_definitions(tinycbor PUBLIC
		CBOR_PARSER_MAX_RECURSIONS=4
)
target_compile_options(tinycbor PUBLIC
		$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-fno-freestanding>
)

# 		../../crypto/micro-ecc
# )

add_library(hash INTERFACE)
target_include_directories(hash INTERFACE ../../crypto/hash)

add_library(hmac STATIC)
target_sources(hmac PRIVATE ../../crypto/hmac/hmac.c)
target_include_directories(hmac INTERFACE ../../crypto/hmac)
target_link_libraries(hmac PUBLIC hash)

add_library(hkdf STATIC)
target_sources(hkdf PRIVATE ../../crypto/hkdf/hkdf.c)
target_include_directories(hkdf INTERFACE ../../crypto/hkdf)
target_link_libraries(hkdf PUBLIC hash hmac)

#TODO: hw crypto
# add_library(app_hw_crypto STATIC)
# target_sources(app_hw_crypto PUBLIC
# 		Src/hw_crypto.c
# 		Src/hw_crypto_curves.c
# )
# target_include_directories(app_hw_crypto PUBLIC ../../core)
# target_link_libraries(app_hw_crypto
# 		PUBLIC hash stm32cubemx
# )

add_library(core STATIC)
target_sources(core PUBLIC
		../../core/ctap.c
		../../core/ctap_asn1.c
		../../core/ctap_credential_management.c
		../../core/ctap_credential_store.c
		../../core/ctap_credentials.c
		../../core/ctap_encode.c
		../../core/ctap_get_info.c
		# ../../core/ctap_memory_storage.c
		../../core/ctap_parse.c
		../../core/ctap_pin.c
		../../core/ctap_pin_protocol.c
		../../core/ctap_pin_uv_auth_token.c
		../../core/ctap_reset.c
		../../core/ctap_selection.c
		../../core/ctap_string.c
		../../core/ctaphid.c
		../../core/utils.c
)
target_include_directories(core PUBLIC ../../core Inc)
target_link_libraries(core PUBLIC
		tinycbor hkdf hmac
)
target_compile_definitions(core PUBLIC
		LIONKEY_DEBUG_LEVEL=${LIONKEY_DEBUG_LEVEL}
		# TODO: arm-none-eabi does not support %zu? see https://stackoverflow.com/questions/76837281/zu-format-specifier-with-c99-not-working
		PRIsz="u"
)

if (LIONKEY_DEVELOPMENT_OVERRIDE)
	target_compile_definitions(core PUBLIC
			LIONKEY_DEVELOPMENT_OVERRIDE=1
	)
endif ()

if (DEFINED LIONKEY_DEVELOPMENT_AUTO_USER_PRESENCE)
	target_compile_definitions(core PUBLIC
			LIONKEY_DEVELOPMENT_AUTO_USER_PRESENCE=${LIONKEY_DEVELOPMENT_AUTO_USER_PRESENCE}
	)
endif ()


set(TINYUSB_SRC ../../lib/tinyusb/src)
add_library(tinyusb INTERFACE)
target_sources(tinyusb PUBLIC
		# common
		../../lib/tinyusb/src/tusb.c
		../../lib/tinyusb/src/common/tusb_fifo.c
		# device
		../../lib/tinyusb/src/device/usbd.c
		../../lib/tinyusb/src/device/usbd_control.c
		../../lib/tinyusb/src/class/audio/audio_device.c
		../../lib/tinyusb/src/class/cdc/cdc_device.c
		../../lib/tinyusb/src/class/dfu/dfu_device.c
		../../lib/tinyusb/src/class/dfu/dfu_rt_device.c
		../../lib/tinyusb/src/class/hid/hid_device.c
		../../lib/tinyusb/src/class/midi/midi_device.c
		../../lib/tinyusb/src/class/msc/msc_device.c
		../../lib/tinyusb/src/class/net/ecm_rndis_device.c
		../../lib/tinyusb/src/class/net/ncm_device.c
		../../lib/tinyusb/src/class/usbtmc/usbtmc_device.c
		../../lib/tinyusb/src/class/vendor/vendor_device.c
		../../lib/tinyusb/src/class/video/video_device.c
		# host
		../../lib/tinyusb/src/host/usbh.c
		../../lib/tinyusb/src/host/hub.c
		../../lib/tinyusb/src/class/cdc/cdc_host.c
		../../lib/tinyusb/src/class/hid/hid_host.c
		../../lib/tinyusb/src/class/midi/midi_host.c
		../../lib/tinyusb/src/class/msc/msc_host.c
		../../lib/tinyusb/src/class/vendor/vendor_host.c
		# typec
		../../lib/tinyusb/src/typec/usbc.c
)
target_sources(tinyusb PUBLIC
		../../lib/tinyusb/src/portable/nordic/nrf5x/dcd_nrf5x.c
)
target_compile_definitions(tinyusb INTERFACE
		CFG_TUSB_MCU=OPT_MCU_NRF5X
)
target_include_directories(tinyusb INTERFACE ${TINYUSB_SRC})

target_link_directories(${PROJECT_NAME} PRIVATE)


target_sources(${PROJECT_NAME} PRIVATE
		#TODO;nrf implementation
		${NRFX}/mdk/gcc_startup_nrf52840.S
		${NRFX}/mdk/system_nrf52840.c
    	${NRFX}/drivers/src/nrfx_clock.c
    	${NRFX}/drivers/src/nrfx_power.c
		Src/usb_descriptors.c
		Src/main.c
		Src/app.c
		Src/usb.c
		# Src/app_test.c
		# Src/flash_storage.c
		# Src/nfc_test.c
		# Src/demo_ce.c
		# Src/eval_utils.c
)

target_include_directories(${PROJECT_NAME} PRIVATE
	${NRFX}
	${CMAKE_CURRENT_LIST_DIR}/Inc
    ${NRFX}/mdk
    ${NRFX}/hal
    ${NRFX}/drivers
    ${NRFX}/drivers/include
    ${NRFX}/helpers
    ${CMSIS}
)


target_compile_definitions(${PROJECT_NAME} PRIVATE
	NRF52840_XXAA
	NRF_VTOR_CONFIG=0x00001000UL
)

#TODO: REMOVE AND IMPLEMENT HW CRYPTO

add_library(sha256 STATIC)
target_sources(sha256 PRIVATE ../../crypto/sha256/sha256.c)
target_include_directories(sha256 INTERFACE ../../crypto/sha256)
target_link_libraries(sha256 PUBLIC hash)

add_library(tinymt32 STATIC)
target_sources(tinymt32 PRIVATE ../../crypto/tinymt/tinymt/tinymt32.c)
target_include_directories(tinymt32 INTERFACE ../../crypto/tinymt/tinymt)

add_library(tinyaes STATIC)
target_sources(tinyaes PRIVATE ../../crypto/tiny-AES-c/aes.c)
target_include_directories(tinyaes INTERFACE ../../crypto/tinyaes)

add_library(micro_ecc STATIC)
target_sources(micro_ecc PRIVATE ../../crypto/micro-ecc/uECC.c)
target_include_directories(micro_ecc INTERFACE ../../crypto/micro-ecc)

add_library(ctap_crypto_software STATIC)
target_sources(ctap_crypto_software PUBLIC
		../../core/ctap_crypto_software.c
)
target_include_directories(ctap_crypto_software PUBLIC 
		../../core
		../../crypto/tinymt/tinymt
		../../crypto/tiny-AES-c
		../../crypto/micro-ecc
		../../crypto/hash
		../../crypto/hkdf
		../../crypto/hmac
		../../crypto/sha256
		)
target_link_libraries(ctap_crypto_software
		PUBLIC tinyaes micro_ecc tinymt32 sha256
)
##

# Add linked libraries
target_link_libraries(${PROJECT_NAME}
		tinyusb
		core
		#app_hw_crypto
		# st25rfal
		# nfc
		ctap_crypto_software
)

set(HEX_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${PROJECT_NAME}> ${HEX_FILE}
		COMMENT "Building ${HEX_FILE}"
)
set_property(TARGET ${CMAKE_PROJECT_NAME} APPEND PROPERTY ADDITIONAL_CLEAN_FILES ${HEX_FILE})

set(BIN_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${PROJECT_NAME}> ${BIN_FILE}
		COMMENT "Building ${BIN_FILE}"
)
set_property(TARGET ${CMAKE_PROJECT_NAME} APPEND PROPERTY ADDITIONAL_CLEAN_FILES ${BIN_FILE})
