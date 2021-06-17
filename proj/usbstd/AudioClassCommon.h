/********************************************************************************************************
 * @file     AudioClassCommon.h 
 *
 * @brief    for TLSR chips
 *
 * @author	 telink
 * @date     Sep. 30, 2010
 *
 * @par      Copyright (c) 2010, Telink Semiconductor (Shanghai) Co., Ltd.
 *           All rights reserved.
 *           
 *			 The information contained herein is confidential and proprietary property of Telink 
 * 		     Semiconductor (Shanghai) Co., Ltd. and is available under the terms 
 *			 of Commercial License Agreement between Telink Semiconductor (Shanghai) 
 *			 Co., Ltd. and the licensee in separate contract or the terms described here-in. 
 *           This heading MUST NOT be removed from this file.
 *
 * 			 Licensees are granted free, non-transferable use of the information in this 
 *			 file under Mutual Non-Disclosure Agreement. NO WARRENTY of ANY KIND is provided. 
 *           
 *******************************************************************************************************/

#pragma once

/* Includes: */
#include "../common/types.h"
#include "StdDescriptors.h"

/* Enable C linkage for C++ Compilers: */
#if defined(__cplusplus)
extern "C" {
#endif

/* Macros: */
/** \name Audio Channel Masks */
//@{
/** Supported channel mask for an Audio class terminal descriptor. See the Audio class specification for more details. */
#define AUDIO_CHANNEL_LEFT_FRONT           (1 << 0)

/** Supported channel mask for an Audio class terminal descriptor. See the Audio class specification for more details. */
#define AUDIO_CHANNEL_RIGHT_FRONT          (1 << 1)

/** Supported channel mask for an Audio class terminal descriptor. See the Audio class specification for more details. */
#define AUDIO_CHANNEL_CENTER_FRONT         (1 << 2)

/** Supported channel mask for an Audio class terminal descriptor. See the Audio class specification for more details. */
#define AUDIO_CHANNEL_LOW_FREQ_ENHANCE     (1 << 3)

/** Supported channel mask for an Audio class terminal descriptor. See the Audio class specification for more details. */
#define AUDIO_CHANNEL_LEFT_SURROUND        (1 << 4)

/** Supported channel mask for an Audio class terminal descriptor. See the Audio class specification for more details. */
#define AUDIO_CHANNEL_RIGHT_SURROUND       (1 << 5)

/** Supported channel mask for an Audio class terminal descriptor. See the Audio class specification for more details. */
#define AUDIO_CHANNEL_LEFT_OF_CENTER       (1 << 6)

/** Supported channel mask for an Audio class terminal descriptor. See the Audio class specification for more details. */
#define AUDIO_CHANNEL_RIGHT_OF_CENTER      (1 << 7)

/** Supported channel mask for an Audio class terminal descriptor. See the Audio class specification for more details. */
#define AUDIO_CHANNEL_SURROUND             (1 << 8)

/** Supported channel mask for an Audio class terminal descriptor. See the Audio class specification for more details. */
#define AUDIO_CHANNEL_SIDE_LEFT            (1 << 9)

/** Supported channel mask for an Audio class terminal descriptor. See the Audio class specification for more details. */
#define AUDIO_CHANNEL_SIDE_RIGHT           (1 << 10)

/** Supported channel mask for an Audio class terminal descriptor. See the Audio class specification for more details. */
#define AUDIO_CHANNEL_TOP                  (1 << 11)
//@}

/** \name Audio Feature Masks */
//@{
/** Supported feature mask for an Audio class feature unit descriptor. See the Audio class specification for more details. */
#define AUDIO_FEATURE_MUTE                 (1 << 0)

/** Supported feature mask for an Audio class feature unit descriptor. See the Audio class specification for more details. */
#define AUDIO_FEATURE_VOLUME               (1 << 1)

/** Supported feature mask for an Audio class feature unit descriptor. See the Audio class specification for more details. */
#define AUDIO_FEATURE_BASS                 (1 << 2)

/** Supported feature mask for an Audio class feature unit descriptor. See the Audio class specification for more details. */
#define AUDIO_FEATURE_MID                  (1 << 3)

/** Supported feature mask for an Audio class feature unit descriptor. See the Audio class specification for more details. */
#define AUDIO_FEATURE_TREBLE               (1 << 4)

/** Supported feature mask for an Audio class feature unit descriptor. See the Audio class specification for more details. */
#define AUDIO_FEATURE_GRAPHIC_EQUALIZER    (1 << 5)

/** Supported feature mask for an Audio class feature unit descriptor. See the Audio class specification for more details. */
#define AUDIO_FEATURE_AUTOMATIC_GAIN       (1 << 6)

/** Supported feature mask for an Audio class feature unit descriptor. See the Audio class specification for more details. */
#define AUDIO_FEATURE_DELAY                (1 << 7)

/** Supported feature mask for an Audio class feature unit descriptor. See the Audio class specification for more details. */
#define AUDIO_FEATURE_BASS_BOOST           (1 << 8)

/** Supported feature mask for an Audio class feature unit descriptor. See the Audio class specification for more details. */
#define AUDIO_FEATURE_BASS_LOUDNESS        (1 << 9)
//@}

/** \name Audio Terminal Types */
//@{
/** Terminal type constant for an Audio class terminal descriptor. See the Audio class specification for more details. */
#define AUDIO_TERMINAL_UNDEFINED           0x0100

/** Terminal type constant for an Audio class terminal descriptor. See the Audio class specification for more details. */
#define AUDIO_TERMINAL_STREAMING           0x0101

/** Terminal type constant for an Audio class terminal descriptor. See the Audio class specification for more details. */
#define AUDIO_TERMINAL_VENDOR              0x01FF

/** Terminal type constant for an Audio class terminal descriptor. See the Audio class specification for more details. */
#define AUDIO_TERMINAL_IN_UNDEFINED        0x0200

/** Terminal type constant for an Audio class terminal descriptor. See the Audio class specification for more details. */
#define AUDIO_TERMINAL_IN_MIC              0x0201

/** Terminal type constant for an Audio class terminal descriptor. See the Audio class specification for more details. */
#define AUDIO_TERMINAL_IN_DESKTOP_MIC      0x0202

/** Terminal type constant for an Audio class terminal descriptor. See the Audio class specification for more details. */
#define AUDIO_TERMINAL_IN_PERSONAL_MIC     0x0203

/** Terminal type constant for an Audio class terminal descriptor. See the Audio class specification for more details. */
#define AUDIO_TERMINAL_IN_OMNIDIR_MIC      0x0204

/** Terminal type constant for an Audio class terminal descriptor. See the Audio class specification for more details. */
#define AUDIO_TERMINAL_IN_MIC_ARRAY        0x0205

/** Terminal type constant for an Audio class terminal descriptor. See the Audio class specification for more details. */
#define AUDIO_TERMINAL_IN_PROCESSING_MIC   0x0206

/** Terminal type constant for an Audio class terminal descriptor. See the Audio class specification for more details. */
#define AUDIO_TERMINAL_IN_OUT_UNDEFINED    0x0300

/** Terminal type constant for an Audio class terminal descriptor. See the Audio class specification for more details. */
#define AUDIO_TERMINAL_OUT_SPEAKER         0x0301

/** Terminal type constant for an Audio class terminal descriptor. See the Audio class specification for more details. */
#define AUDIO_TERMINAL_OUT_HEADPHONES      0x0302

/** Terminal type constant for an Audio class terminal descriptor. See the Audio class specification for more details. */
#define AUDIO_TERMINAL_OUT_HEAD_MOUNTED    0x0303

/** Terminal type constant for an Audio class terminal descriptor. See the Audio class specification for more details. */
#define AUDIO_TERMINAL_OUT_DESKTOP         0x0304

/** Terminal type constant for an Audio class terminal descriptor. See the Audio class specification for more details. */
#define AUDIO_TERMINAL_OUT_ROOM            0x0305

/** Terminal type constant for an Audio class terminal descriptor. See the Audio class specification for more details. */
#define AUDIO_TERMINAL_OUT_COMMUNICATION   0x0306

/** Terminal type constant for an Audio class terminal descriptor. See the Audio class specification for more details. */
#define AUDIO_TERMINAL_OUT_LOWFREQ         0x0307
//@}

/** Convenience macro to fill a 24-bit \ref USB_Audio_SampleFreq_t structure with the given sample rate as a 24-bit number.
 *
 *  \param[in] freq  Required audio sampling frequency in HZ
 */
#define AUDIO_SAMPLE_FREQ(freq)           {.Byte1 = ((uint32_t)freq & 0xFF), .Byte2 = (((uint32_t)freq >> 8) & 0xFF), .Byte3 = (((uint32_t)freq >> 16) & 0xFF)}

/** Mask for the attributes parameter of an Audio class-specific Endpoint descriptor, indicating that the endpoint
 *  accepts only filled endpoint packets of audio samples.
 */
#define AUDIO_EP_FULL_PACKETS_ONLY        (1 << 7)

/** Mask for the attributes parameter of an Audio class-specific Endpoint descriptor, indicating that the endpoint
 *  will accept partially filled endpoint packets of audio samples.
 */
#define AUDIO_EP_ACCEPTS_SMALL_PACKETS    (0 << 7)

/** Mask for the attributes parameter of an Audio class-specific Endpoint descriptor, indicating that the endpoint
 *  allows for sampling frequency adjustments to be made via control requests directed at the endpoint.
 */
#define AUDIO_EP_SAMPLE_FREQ_CONTROL      (1 << 0)

/** Mask for the attributes parameter of an Audio class-specific Endpoint descriptor, indicating that the endpoint
 *  allows for pitch adjustments to be made via control requests directed at the endpoint.
 */
#define AUDIO_EP_PITCH_CONTROL            (1 << 1)

/* Enums: */
/** Enum for possible Class, Subclass and Protocol values of device and interface descriptors relating to the Audio
 *  device class.
 */
enum Audio_Descriptor_ClassSubclassProtocol_t
{
	AUDIO_CSCP_AudioClass                     = 0x01, /**< Descriptor Class value indicating that the device or
	                                                   *   interface belongs to the USB Audio 1.0 class.
	                                                   */
	AUDIO_CSCP_ControlSubclass                = 0x01, /**< Descriptor Subclass value indicating that the device or
	                                                   *   interface belongs to the Audio Control subclass.
	                                                   */
	AUDIO_CSCP_ControlProtocol                = 0x00, /**< Descriptor Protocol value indicating that the device or
	                                                   *   interface belongs to the Audio Control protocol.
	                                                   */
	AUDIO_CSCP_AudioStreamingSubclass         = 0x02, /**< Descriptor Subclass value indicating that the device or
	                                                   *   interface belongs to the MIDI Streaming subclass.
	                                                   */
	AUDIO_CSCP_MIDIStreamingSubclass          = 0x03, /**< Descriptor Subclass value indicating that the device or
	                                                   *   interface belongs to the Audio streaming subclass.
	                                                   */
	AUDIO_CSCP_StreamingProtocol              = 0x00, /**< Descriptor Protocol value indicating that the device or
	                                                   *   interface belongs to the Streaming Audio protocol.
	                                                   */
};

/** Audio class specific interface description subtypes, for the Audio Control interface. */
enum Audio_CSInterface_AC_SubTypes_t
{
	AUDIO_DSUBTYPE_CSInterface_Header         = 0x01, /**< Audio class specific control interface header. */
	AUDIO_DSUBTYPE_CSInterface_InputTerminal  = 0x02, /**< Audio class specific control interface Input Terminal. */
	AUDIO_DSUBTYPE_CSInterface_OutputTerminal = 0x03, /**< Audio class specific control interface Output Terminal. */
	AUDIO_DSUBTYPE_CSInterface_Mixer          = 0x04, /**< Audio class specific control interface Mixer Unit. */
	AUDIO_DSUBTYPE_CSInterface_Selector       = 0x05, /**< Audio class specific control interface Selector Unit. */
	AUDIO_DSUBTYPE_CSInterface_Feature        = 0x06, /**< Audio class specific control interface Feature Unit. */
	AUDIO_DSUBTYPE_CSInterface_Processing     = 0x07, /**< Audio class specific control interface Processing Unit. */
	AUDIO_DSUBTYPE_CSInterface_Extension      = 0x08, /**< Audio class specific control interface Extension Unit. */
};

/** Audio class specific interface description subtypes, for the Audio Streaming interface. */
enum Audio_CSInterface_AS_SubTypes_t
{
	AUDIO_DSUBTYPE_CSInterface_General        = 0x01, /**< Audio class specific streaming interface general descriptor. */
	AUDIO_DSUBTYPE_CSInterface_FormatType     = 0x02, /**< Audio class specific streaming interface format type descriptor. */
	AUDIO_DSUBTYPE_CSInterface_FormatSpecific = 0x03, /**< Audio class specific streaming interface format information descriptor. */
};

/** Audio class specific endpoint description subtypes, for the Audio Streaming interface. */
enum Audio_CSEndpoint_SubTypes_t
{
	AUDIO_DSUBTYPE_CSEndpoint_General         = 0x01, /**< Audio class specific endpoint general descriptor. */
};

/** Enum for the Audio class specific control requests that can be issued by the USB bus host. */
enum Audio_ClassRequests_t
{
	AUDIO_REQ_SetCurrent    = 0x01, /**< Audio class-specific request to set the current value of a parameter within the device. */
	AUDIO_REQ_SetMinimum    = 0x02, /**< Audio class-specific request to set the minimum value of a parameter within the device. */
	AUDIO_REQ_SetMaximum    = 0x03, /**< Audio class-specific request to set the maximum value of a parameter within the device. */
	AUDIO_REQ_SetResolution = 0x04, /**< Audio class-specific request to set the resolution value of a parameter within the device. */
	AUDIO_REQ_SetMemory     = 0x05, /**< Audio class-specific request to set the memory value of a parameter within the device. */
	AUDIO_REQ_GetCurrent    = 0x81, /**< Audio class-specific request to get the current value of a parameter within the device. */
	AUDIO_REQ_GetMinimum    = 0x82, /**< Audio class-specific request to get the minimum value of a parameter within the device. */
	AUDIO_REQ_GetMaximum    = 0x83, /**< Audio class-specific request to get the maximum value of a parameter within the device. */
	AUDIO_REQ_GetResolution = 0x84, /**< Audio class-specific request to get the resolution value of a parameter within the device. */
	AUDIO_REQ_GetMemory     = 0x85, /**< Audio class-specific request to get the memory value of a parameter within the device. */
	AUDIO_REQ_GetStatus     = 0xFF, /**< Audio class-specific request to get the device status. */
};

/** Enum for Audio class specific Endpoint control modifiers which can be set and retrieved by a USB host, if the corresponding
 *  endpoint control is indicated to be supported in the Endpoint's Audio-class specific endpoint descriptor.
 */
enum Audio_EndpointControls_t
{
	AUDIO_EPCONTROL_SamplingFreq = 0x01, /**< Sampling frequency adjustment of the endpoint. */
	AUDIO_EPCONTROL_Pitch        = 0x02, /**< Pitch adjustment of the endpoint. */
};

/* Type Defines: */
/** \brief Audio class-specific Input Terminal Descriptor (LUFA naming conventions).
 *
 *  Type define for an Audio class-specific input terminal descriptor. This indicates to the host that the device
 *  contains an input audio source, either from a physical terminal on the device, or a logical terminal (for example,
 *  a USB endpoint). See the USB Audio specification for more details.
 *
 *  \see \ref USB_Audio_StdDescriptor_InputTerminal_t for the version of this type with standard element names.
 *
 *  \note Regardless of CPU architecture, these values should be stored as little endian.
 */
typedef struct
{
	USB_Descriptor_Header_t Header; /**< Regular descriptor header containing the descriptor's type and length. */
	u8                 Subtype; /**< Sub type value used to distinguish between audio class-specific descriptors,
	                                  *   must be \ref AUDIO_DSUBTYPE_CSInterface_InputTerminal.
	                                  */

	u8                 TerminalID; /**< ID value of this terminal unit - must be a unique value within the device. */
	u16                TerminalType; /**< Type of terminal, a \c TERMINAL_* mask. */
	u8                 AssociatedOutputTerminal; /**< ID of associated output terminal, for physically grouped terminals
	                                                   *   such as the speaker and microphone of a phone handset.
	                                                   */
	u8                 TotalChannels; /**< Total number of separate audio channels within this interface (right, left, etc.) */
	u16                ChannelConfig; /**< \c CHANNEL_* masks indicating what channel layout is supported by this terminal. */

	u8                 ChannelStrIndex; /**< Index of a string descriptor describing this channel within the device. */
	u8                 TerminalStrIndex; /**< Index of a string descriptor describing this descriptor within the device. */
}USB_Audio_Descriptor_InputTerminal_t;

/** \brief Audio class-specific Input Terminal Descriptor (USB-IF naming conventions).
 *
 *  Type define for an Audio class-specific input terminal descriptor. This indicates to the host that the device
 *  contains an input audio source, either from a physical terminal on the device, or a logical terminal (for example,
 *  a USB endpoint). See the USB Audio specification for more details.
 *
 *  \see \ref USB_Audio_Descriptor_InputTerminal_t for the version of this type with non-standard LUFA specific
 *       element names.
 *
 *  \note Regardless of CPU architecture, these values should be stored as little endian.
 */
typedef struct
{
	u8  bLength; /**< Size of the descriptor, in bytes. */
	u8  bDescriptorType; /**< Type of the descriptor, either a value in \ref USB_DescriptorTypes_t or a value
	                           *   given by the specific class.
	                           */

	u8  bDescriptorSubtype; /**< Sub type value used to distinguish between audio class-specific descriptors,
	                              *   must be \ref AUDIO_DSUBTYPE_CSInterface_InputTerminal.
	                              */
	u8  bTerminalID; /**< ID value of this terminal unit - must be a unique value within the device. */
	u16 wTerminalType; /**< Type of terminal, a \c TERMINAL_* mask. */
	u8  bAssocTerminal; /**< ID of associated output terminal, for physically grouped terminals
	                          *   such as the speaker and microphone of a phone handset.
	                          */
	u8  bNrChannels; /**< Total number of separate audio channels within this interface (right, left, etc.) */
	u16 wChannelConfig; /**< \c CHANNEL_* masks indicating what channel layout is supported by this terminal. */

	u8  iChannelNames; /**< Index of a string descriptor describing this channel within the device. */
	u8  iTerminal; /**< Index of a string descriptor describing this descriptor within the device. */
}USB_Audio_StdDescriptor_InputTerminal_t;

/** \brief Audio class-specific Output Terminal Descriptor (LUFA naming conventions).
 *
 *  Type define for an Audio class-specific output terminal descriptor. This indicates to the host that the device
 *  contains an output audio sink, either to a physical terminal on the device, or a logical terminal (for example,
 *  a USB endpoint). See the USB Audio specification for more details.
 *
 *  \see \ref USB_Audio_StdDescriptor_OutputTerminal_t for the version of this type with standard element names.
 *
 *  \note Regardless of CPU architecture, these values should be stored as little endian.
 */
typedef struct
{
	USB_Descriptor_Header_t Header; /**< Regular descriptor header containing the descriptor's type and length. */
	u8                 Subtype; /**< Sub type value used to distinguish between audio class-specific descriptors,
	                                  *   must be \ref AUDIO_DSUBTYPE_CSInterface_OutputTerminal.
	                                  */

	u8                 TerminalID; /**< ID value of this terminal unit - must be a unique value within the device. */
	u16                TerminalType; /**< Type of terminal, a \c TERMINAL_* mask. */
	u8                 AssociatedInputTerminal; /**< ID of associated input terminal, for physically grouped terminals
	                                                    *   such as the speaker and microphone of a phone handset.
	                                                    */
	u8                 SourceID; /**< ID value of the unit this terminal's audio is sourced from. */

	u8                 TerminalStrIndex; /**< Index of a string descriptor describing this descriptor within the device. */
}USB_Audio_Descriptor_OutputTerminal_t;

/** \brief Audio class-specific Output Terminal Descriptor (USB-IF naming conventions).
 *
 *  Type define for an Audio class-specific output terminal descriptor. This indicates to the host that the device
 *  contains an output audio sink, either to a physical terminal on the device, or a logical terminal (for example,
 *  a USB endpoint). See the USB Audio specification for more details.
 *
 *  \see \ref USB_Audio_Descriptor_OutputTerminal_t for the version of this type with non-standard LUFA specific
 *       element names.
 *
 *  \note Regardless of CPU architecture, these values should be stored as little endian.
 */
typedef struct
{
	u8  bLength; /**< Size of the descriptor, in bytes. */
	u8  bDescriptorType; /**< Sub type value used to distinguish between audio class-specific descriptors,
	                           *   must be \ref AUDIO_DSUBTYPE_CSInterface_OutputTerminal.
	                           */

	u8  bDescriptorSubtype; /**< Sub type value used to distinguish between audio class-specific descriptors,
	                              *   a value from the \ref Audio_CSInterface_AC_SubTypes_t enum.
	                              */
	u8  bTerminalID; /**< ID value of this terminal unit - must be a unique value within the device. */
	u16 wTerminalType; /**< Type of terminal, a \c TERMINAL_* mask. */
	u8  bAssocTerminal; /**< ID of associated input terminal, for physically grouped terminals
	                          *   such as the speaker and microphone of a phone handset.
	                          */
	u8  bSourceID; /**< ID value of the unit this terminal's audio is sourced from. */

	u8  iTerminal; /**< Index of a string descriptor describing this descriptor within the device. */
}USB_Audio_StdDescriptor_OutputTerminal_t;

/** \brief Audio class-specific Interface Descriptor (LUFA naming conventions).
 *
 *  Type define for an Audio class-specific interface descriptor. This follows a regular interface descriptor to
 *  supply extra information about the audio device's layout to the host. See the USB Audio specification for more
 *  details.
 *
 *  \see \ref USB_Audio_StdDescriptor_Interface_AC_t for the version of this type with standard element names.
 *
 *  \note Regardless of CPU architecture, these values should be stored as little endian.
 */
typedef struct
{
	USB_Descriptor_Header_t Header; /**< Regular descriptor header containing the descriptor's type and length. */
	u8                 Subtype; /**< Sub type value used to distinguish between audio class-specific descriptors,
	                                  *   a value from the \ref Audio_CSInterface_AS_SubTypes_t enum.
	                                  */

	u8                 ACSpecification[2]; /**< Binary coded decimal value, indicating the supported Audio Class specification version. */
	u8                 TotalLength[2]; /**< Total length of the Audio class-specific descriptors, including this descriptor. */

	u8                 InCollection; /**< Total number of Audio Streaming interfaces linked to this Audio Control interface (must be 1). */
	u8                 InterfaceNumber; /**< Interface number of the associated Audio Streaming interface. */
}USB_Audio_Descriptor_Interface_AC_t;

/*surport mic && speaker*/
typedef struct
{
	USB_Descriptor_Header_t Header; /**< Regular descriptor header containing the descriptor's type and length. */
	u8                 Subtype; /**< Sub type value used to distinguish between audio class-specific descriptors,
	                                  *   a value from the \ref Audio_CSInterface_AS_SubTypes_t enum.
	                                  */

	u8                 ACSpecification[2]; /**< Binary coded decimal value, indicating the supported Audio Class specification version. */
	u8                 TotalLength[2]; /**< Total length of the Audio class-specific descriptors, including this descriptor. */

	u8                 InCollection; /**< Total number of Audio Streaming interfaces linked to this Audio Control interface. */
	u8                 InterfaceNumber_speaker; /**< Interface number of the associated Audio Streaming interface for speaker. */
	u8                 InterfaceNumber_mic; /**< Interface number of the associated Audio Streaming interface for mic. */
}USB_Audio_Descriptor_Interface_AC_TL_t;

/** \brief Audio class-specific Interface Descriptor (USB-IF naming conventions).
 *
 *  Type define for an Audio class-specific interface descriptor. This follows a regular interface descriptor to
 *  supply extra information about the audio device's layout to the host. See the USB Audio specification for more
 *  details.
 *
 *  \see \ref USB_Audio_Descriptor_Interface_AC_t for the version of this type with non-standard LUFA specific
 *       element names.
 *
 *  \note Regardless of CPU architecture, these values should be stored as little endian.
 */
typedef struct
{
	u8  bLength; /**< Size of the descriptor, in bytes. */
	u8  bDescriptorType; /**< Type of the descriptor, either a value in \ref USB_DescriptorTypes_t or a value
	                           *   given by the specific class.
	                           */

	u8  bDescriptorSubtype;/**< Sub type value used to distinguish between audio class-specific descriptors,
	                             *   a value from the \ref Audio_CSInterface_AS_SubTypes_t enum.
	                             */

	u8  bcdADC[2]; /**< Binary coded decimal value, indicating the supported Audio Class specification version. */
	u8 wTotalLength[2]; /**< Total length of the Audio class-specific descriptors, including this descriptor. */

	u8  bInCollection; /**< Total number of Audio Streaming interfaces linked to this Audio Control interface (must be 1). */
	u8  bInterfaceNumbers; /**< Interface number of the associated Audio Streaming interface. */
}USB_Audio_StdDescriptor_Interface_AC_t;

/** \brief Audio class-specific Feature Unit Descriptor (LUFA naming conventions).
 *
 *  Type define for an Audio class-specific Feature Unit descriptor. This indicates to the host what features
 *  are present in the device's audio stream for basic control, such as per-channel volume. See the USB Audio
 *  specification for more details.
 *
 *  \see \ref USB_Audio_StdDescriptor_FeatureUnit_t for the version of this type with standard element names.
 *
 *  \note Regardless of CPU architecture, these values should be stored as little endian.
 */
typedef struct
{
	USB_Descriptor_Header_t Header; /**< Regular descriptor header containing the descriptor's type and length. */
	u8                 Subtype; /**< Sub type value used to distinguish between audio class-specific descriptors,
	                                  *   must be \ref AUDIO_DSUBTYPE_CSInterface_Feature.
	                                  */

	u8                 UnitID; /**< ID value of this feature unit - must be a unique value within the device. */
	u8                 SourceID; /**< Source ID value of the audio source input into this feature unit. */

	u8                 ControlSize; /**< Size of each element in the \c ChannelControls array. */
	u8                 ChannelControls[3]; /**< Feature masks for the control channel, and each separate audio channel. */

	u8                 FeatureUnitStrIndex; /**< Index of a string descriptor describing this descriptor within the device. */
}USB_Audio_Descriptor_FeatureUnit_t;

/*just for usb_mic*/
typedef struct
{
	USB_Descriptor_Header_t Header; /**< Regular descriptor header containing the descriptor's type and length. */
	u8                 Subtype; /**< Sub type value used to distinguish between audio class-specific descriptors,
	                                  *   must be \ref AUDIO_DSUBTYPE_CSInterface_Feature.
	                                  */

	u8                 UnitID; /**< ID value of this feature unit - must be a unique value within the device. */
	u8                 SourceID; /**< Source ID value of the audio source input into this feature unit. */

	u8                 ControlSize; /**< Size of each element in the \c ChannelControls array. */
	u8                 ChannelControls[2]; /**< Feature masks for the control channel, and each separate audio channel. */

	u8                 FeatureUnitStrIndex; /**< Index of a string descriptor describing this descriptor within the device. */
}USB_Audio_Descriptor_FeatureUnit_Mic_t;

/** \brief Audio class-specific Feature Unit Descriptor (USB-IF naming conventions).
 *
 *  Type define for an Audio class-specific Feature Unit descriptor. This indicates to the host what features
 *  are present in the device's audio stream for basic control, such as per-channel volume. See the USB Audio
 *  specification for more details.
 *
 *  \see \ref USB_Audio_Descriptor_FeatureUnit_t for the version of this type with non-standard LUFA specific
 *       element names.
 *
 *  \note Regardless of CPU architecture, these values should be stored as little endian.
 */
typedef struct
{
	u8 bLength; /**< Size of the descriptor, in bytes. */
	u8 bDescriptorType; /**< Type of the descriptor, either a value in \ref USB_DescriptorTypes_t or a value
	                          *   given by the specific class.
	                          */

	u8 bDescriptorSubtype; /**< Sub type value used to distinguish between audio class-specific descriptors,
	                             *   must be \ref AUDIO_DSUBTYPE_CSInterface_Feature.
	                             */

	u8 bUnitID; /**< ID value of this feature unit - must be a unique value within the device. */
	u8 bSourceID; /**< Source ID value of the audio source input into this feature unit. */

	u8 bControlSize; /**< Size of each element in the \c ChannelControls array. */
	u8 bmaControls[3]; /**< Feature masks for the control channel, and each separate audio channel. */

	u8 iFeature; /**< Index of a string descriptor describing this descriptor within the device. */
}USB_Audio_StdDescriptor_FeatureUnit_t;

/** \brief Audio class-specific Streaming Audio Interface Descriptor (LUFA naming conventions).
 *
 *  Type define for an Audio class-specific streaming interface descriptor. This indicates to the host
 *  how audio streams within the device are formatted. See the USB Audio specification for more details.
 *
 *  \see \ref USB_Audio_StdDescriptor_Interface_AS_t for the version of this type with standard element names.
 *
 *  \note Regardless of CPU architecture, these values should be stored as little endian.
 */
typedef struct
{
	USB_Descriptor_Header_t Header; /**< Regular descriptor header containing the descriptor's type and length. */
	u8                 Subtype; /**< Sub type value used to distinguish between audio class-specific descriptors,
	                                  *   a value from the \ref Audio_CSInterface_AS_SubTypes_t enum.
	                                  */

	u8                 TerminalLink; /**< ID value of the output terminal this descriptor is describing. */

	u8                 FrameDelay; /**< Delay in frames resulting from the complete sample processing from input to output. */
	u8                 AudioFormat[2]; /**< Format of the audio stream, see Audio Device Formats specification. */
}USB_Audio_Descriptor_Interface_AS_t;

/** \brief Audio class-specific Streaming Audio Interface Descriptor (USB-IF naming conventions).
 *
 *  Type define for an Audio class-specific streaming interface descriptor. This indicates to the host
 *  how audio streams within the device are formatted. See the USB Audio specification for more details.
 *
 *  \see \ref USB_Audio_Descriptor_Interface_AS_t for the version of this type with non-standard LUFA specific
 *       element names.
 *
 *  \note Regardless of CPU architecture, these values should be stored as little endian.
 */
typedef struct
{
	u8  bLength; /**< Size of the descriptor, in bytes. */
	u8  bDescriptorType; /**< Type of the descriptor, either a value in \ref USB_DescriptorTypes_t or a value
	                           *   given by the specific class.
	                           */

	u8  bDescriptorSubtype; /**< Sub type value used to distinguish between audio class-specific descriptors,
	                              *   a value from the \ref Audio_CSInterface_AS_SubTypes_t enum.
	                              */

	u8  bTerminalLink; /**< ID value of the output terminal this descriptor is describing. */

	u8  bDelay; /**< Delay in frames resulting from the complete sample processing from input to output. */
	u16 wFormatTag; /**< Format of the audio stream, see Audio Device Formats specification. */
}USB_Audio_StdDescriptor_Interface_AS_t;

/** \brief Audio class-specific Format Descriptor (LUFA naming conventions).
 *
 *  Type define for an Audio class-specific audio format descriptor. This is used to give the host full details
 *  about the number of channels, the sample resolution, acceptable sample frequencies and encoding method used
 *  in the device's audio streams. See the USB Audio specification for more details.
 *
 *  \note This descriptor <b>must</b> be followed by one or more \ref USB_Audio_SampleFreq_t elements containing
 *        the continuous or discrete sample frequencies.
 *
 *  \see \ref USB_Audio_StdDescriptor_Format_t for the version of this type with standard element names.
 *
 *  \note Regardless of CPU architecture, these values should be stored as little endian.
 */
typedef struct
{
	USB_Descriptor_Header_t Header; /**< Regular descriptor header containing the descriptor's type and length. */
	u8                 Subtype; /**< Sub type value used to distinguish between audio class-specific descriptors,
	                                  *   must be \ref AUDIO_DSUBTYPE_CSInterface_FormatType.
	                                  */

	u8                 FormatType; /**< Format of the audio stream, see Audio Device Formats specification. */
	u8                 Channels; /**< Total number of discrete channels in the stream. */

	u8                 SubFrameSize; /**< Size in bytes of each channel's sample data in the stream. */
	u8                 BitResolution; /**< Bits of resolution of each channel's samples in the stream. */

	u8                 TotalDiscreteSampleRates; /**< Total number of discrete sample frequencies supported by the device. When
	                                                   *   zero, this must be followed by the lower and upper continuous sampling
	                                                   *   frequencies supported by the device; otherwise, this must be followed
	                                                   *   by the given number of discrete sampling frequencies supported.
	                                                   */
}USB_Audio_Descriptor_Format_t;

/** \brief 24-Bit Audio Frequency Structure.
 *
 *  Type define for a 24bit audio sample frequency structure. As GCC does not contain a built in 24-bit datatype,
 *  this this structure is used to build up the value instead. Fill this structure with the \ref AUDIO_SAMPLE_FREQ() macro.
 *
 *  \note Regardless of CPU architecture, these values should be stored as little endian.
 */
typedef struct
{
	u8 Byte1; /**< Lowest 8 bits of the 24-bit value. */
	u8 Byte2; /**< Middle 8 bits of the 24-bit value. */
	u8 Byte3; /**< Upper 8 bits of the 24-bit value. */
}USB_Audio_SampleFreq_t;

/** \brief Audio class-specific Format Descriptor (USB-IF naming conventions).
 *
 *  Type define for an Audio class-specific audio format descriptor. This is used to give the host full details
 *  about the number of channels, the sample resolution, acceptable sample frequencies and encoding method used
 *  in the device's audio streams. See the USB Audio specification for more details.
 *
 *  \note This descriptor <b>must</b> be followed by one or more 24-bit integer elements containing the continuous
 *        or discrete sample frequencies.
 *
 *  \see \ref USB_Audio_Descriptor_Format_t for the version of this type with non-standard LUFA specific
 *       element names.
 *
 *  \note Regardless of CPU architecture, these values should be stored as little endian.
 */
typedef struct
{
	u8 bLength; /**< Size of the descriptor, in bytes. */
	u8 bDescriptorType; /**< Sub type value used to distinguish between audio class-specific descriptors,
	                          *   must be \ref AUDIO_DSUBTYPE_CSInterface_FormatType.
	                          */

	u8 bDescriptorSubtype;/**< Sub type value used to distinguish between audio class-specific descriptors,
	                            *   a value from the \ref Audio_CSInterface_AS_SubTypes_t enum.
	                            */

	u8 bFormatType; /**< Format of the audio stream, see Audio Device Formats specification. */
	u8 bNrChannels; /**< Total number of discrete channels in the stream. */

	u8 bSubFrameSize; /**< Size in bytes of each channel's sample data in the stream. */
	u8 bBitResolution; /**< Bits of resolution of each channel's samples in the stream. */

	u8 bSampleFrequencyType; /**< Total number of sample frequencies supported by the device. When
	                               *   zero, this must be followed by the lower and upper continuous sampling
	                               *   frequencies supported by the device; otherwise, this must be followed
	                               *   by the given number of discrete sampling frequencies supported.
	                               */
}USB_Audio_StdDescriptor_Format_t;

/** \brief Audio class-specific Streaming Endpoint Descriptor (LUFA naming conventions).
 *
 *  Type define for an Audio class-specific endpoint descriptor. This contains a regular endpoint
 *  descriptor with a few Audio-class-specific extensions. See the USB Audio specification for more details.
 *
 *  \see \ref USB_Audio_StdDescriptor_StreamEndpoint_Std_t for the version of this type with standard element names.
 *
 *  \note Regardless of CPU architecture, these values should be stored as little endian.
 */
typedef struct
{
	USB_Descriptor_Endpoint_t Endpoint; /**< Standard endpoint descriptor describing the audio endpoint. */

	u8                   Refresh; /**< Always set to zero for Audio class devices. */
	u8                   SyncEndpointNumber; /**< Endpoint address to send synchronization information to, if needed (zero otherwise). */
}USB_Audio_Descriptor_StreamEndpoint_Std_t;

/** \brief Audio class-specific Streaming Endpoint Descriptor (USB-IF naming conventions).
 *
 *  Type define for an Audio class-specific endpoint descriptor. This contains a regular endpoint
 *  descriptor with a few Audio-class-specific extensions. See the USB Audio specification for more details.
 *
 *  \see \ref USB_Audio_Descriptor_StreamEndpoint_Std_t for the version of this type with non-standard LUFA specific
 *       element names.
 *
 *  \note Regardless of CPU architecture, these values should be stored as little endian.
 */
typedef struct
{
	u8  bLength; /**< Size of the descriptor, in bytes. */
	u8  bDescriptorType; /**< Type of the descriptor, either a value in \ref USB_DescriptorTypes_t or a
	                           *   value given by the specific class.
	                           */
	u8  bEndpointAddress; /**< Logical address of the endpoint within the device for the current
	                            *   configuration, including direction mask.
	                            */
	u8  bmAttributes; /**< Endpoint attributes, comprised of a mask of the endpoint type (\c EP_TYPE_*)
	                        *   and attributes (\c ENDPOINT_ATTR_*) masks.
	                        */
	u16 wMaxPacketSize; /**< Size of the endpoint bank, in bytes. This indicates the maximum packet size
	                          *   that the endpoint can receive at a time.
	                          */
	u8  bInterval; /**< Polling interval in milliseconds for the endpoint if it is an INTERRUPT or
	                     *   ISOCHRONOUS type.
	                     */

	u8  bRefresh; /**< Always set to zero for Audio class devices. */
	u8  bSynchAddress; /**< Endpoint address to send synchronization information to, if needed (zero otherwise). */
}USB_Audio_StdDescriptor_StreamEndpoint_Std_t;

/** \brief Audio class-specific Extended Endpoint Descriptor (LUFA naming conventions).
 *
 *  Type define for an Audio class-specific extended endpoint descriptor. This contains extra information
 *  on the usage of endpoints used to stream audio in and out of the USB Audio device, and follows an Audio
 *  class-specific extended endpoint descriptor. See the USB Audio specification for more details.
 *
 *  \see \ref USB_Audio_StdDescriptor_StreamEndpoint_Spc_t for the version of this type with standard element names.
 *
 *  \note Regardless of CPU architecture, these values should be stored as little endian.
 */
typedef struct
{
	USB_Descriptor_Header_t Header; /**< Regular descriptor header containing the descriptor's type and length. */
	u8                 Subtype; /**< Sub type value used to distinguish between audio class-specific descriptors,
	                                  *   a value from the \ref Audio_CSEndpoint_SubTypes_t enum.
	                                  */

	u8                 Attributes; /**< Audio class-specific endpoint attributes, such as \ref AUDIO_EP_FULL_PACKETS_ONLY. */

	u8                 LockDelayUnits; /**< Units used for the LockDelay field, see Audio class specification. */
	u8                 LockDelay[2]; /**< Time required to internally lock endpoint's internal clock recovery circuitry. */
}USB_Audio_Descriptor_StreamEndpoint_Spc_t;

/** \brief Audio class-specific Extended Endpoint Descriptor (USB-IF naming conventions).
 *
 *  Type define for an Audio class-specific extended endpoint descriptor. This contains extra information
 *  on the usage of endpoints used to stream audio in and out of the USB Audio device, and follows an Audio
 *  class-specific extended endpoint descriptor. See the USB Audio specification for more details.
 *
 *  \see \ref USB_Audio_Descriptor_StreamEndpoint_Spc_t for the version of this type with non-standard LUFA specific
 *       element names.
 *
 *  \note Regardless of CPU architecture, these values should be stored as little endian.
 */
typedef struct
{
	u8  bLength; /**< Size of the descriptor, in bytes. */
	u8  bDescriptorType; /**< Type of the descriptor, either a value in \ref USB_DescriptorTypes_t or a value
	                           *   given by the specific class.
	                           */

	u8  bDescriptorSubtype; /**< Sub type value used to distinguish between audio class-specific descriptors,
	                              *   a value from the \ref Audio_CSEndpoint_SubTypes_t enum.
	                              */

	u8  bmAttributes; /**< Audio class-specific endpoint attributes, such as \ref AUDIO_EP_FULL_PACKETS_ONLY. */

	u8  bLockDelayUnits; /**< Units used for the LockDelay field, see Audio class specification. */
	u16 wLockDelay; /**< Time required to internally lock endpoint's internal clock recovery circuitry. */
}USB_Audio_StdDescriptor_StreamEndpoint_Spc_t;

/* Disable C linkage for C++ Compilers: */
#if defined(__cplusplus)
}
#endif

