// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_BASE_MEDIA_LOG_PROPERTIES_H_
#define MEDIA_BASE_MEDIA_LOG_PROPERTIES_H_

#include <string>
#include <vector>

#include "media/base/audio_decoder_config.h"
#include "media/base/media_export.h"
#include "media/base/media_log_type_enforcement.h"
#include "media/base/video_decoder_config.h"
#include "ui/gfx/geometry/size.h"

namespace media {

// A list of all properties that can be set by a MediaLog. To add a new
// property, it must be added in this enum and have it's type defined below
// using MEDIA_LOG_PROPERTY_SUPPORTS_TYPE(<name>, <type>) or with a custom
// specializer. See MEDIA_LOG_PROEPRTY_SUPPORTS_GFX_SIZE as an example.
enum class MediaLogProperty {
  // Video resolution.
  kResolution,

  // Size of the media content in bytes.
  kTotalBytes,

  // How many bits-per-second this media uses.
  kBitrate,

  // The maximum duration of the media in seconds.
  kMaxDuration,

  // The time at which media starts, in seconds.
  kStartTime,

  // If the video decoder is using a decrypting decoder to playback media.
  kIsVideoEncrypted,

  // Represents whether the media source supports range requests. A truthful
  // value here means that range requests aren't supported and seeking probably
  // wont be supported.
  kIsStreaming,

  // The url and title of the frame containing the document that this media
  // player is loaded into.
  kFrameUrl,
  kFrameTitle,

  // Whether the media content coming from the same origin as the frame in which
  // the player is loaded.
  kIsSingleOrigin,

  // Can the url loading the data support the range http header, allowing chunks
  // to be sent rather than entire file.
  kIsRangeHeaderSupported,

  // The name of the decoder implementation currently being used to play the
  // media stream. All audio/video decoders have names, such as
  // FFMpegVideoDecoder or D3D11VideoDecoder.
  kVideoDecoderName,
  kAudioDecoderName,

  // Whether this decoder is using hardware accelerated decoding.
  kIsPlatformVideoDecoder,
  kIsPlatformAudioDecoder,

  // Whether this media player is using a decrypting demuxer for the given
  // audio or video stream.
  kIsVideoDecryptingDemuxerStream,
  kIsAudioDecryptingDemuxerStream,

  // Track metadata.
  kAudioTracks,
  kVideoTracks,
};

MEDIA_LOG_PROPERTY_SUPPORTS_TYPE(kResolution, gfx::Size);
MEDIA_LOG_PROPERTY_SUPPORTS_TYPE(kTotalBytes, int64_t);
MEDIA_LOG_PROPERTY_SUPPORTS_TYPE(kBitrate, int);
MEDIA_LOG_PROPERTY_SUPPORTS_TYPE(kMaxDuration, float);
MEDIA_LOG_PROPERTY_SUPPORTS_TYPE(kStartTime, float);
MEDIA_LOG_PROPERTY_SUPPORTS_TYPE(kIsVideoEncrypted, bool);
MEDIA_LOG_PROPERTY_SUPPORTS_TYPE(kIsVideoEncrypted, std::string);
MEDIA_LOG_PROPERTY_SUPPORTS_TYPE(kIsStreaming, bool);
MEDIA_LOG_PROPERTY_SUPPORTS_TYPE(kFrameUrl, std::string);
MEDIA_LOG_PROPERTY_SUPPORTS_TYPE(kFrameTitle, std::string);
MEDIA_LOG_PROPERTY_SUPPORTS_TYPE(kIsSingleOrigin, bool);
MEDIA_LOG_PROPERTY_SUPPORTS_TYPE(kVideoDecoderName, std::string);
MEDIA_LOG_PROPERTY_SUPPORTS_TYPE(kIsPlatformVideoDecoder, bool);
MEDIA_LOG_PROPERTY_SUPPORTS_TYPE(kIsRangeHeaderSupported, bool);
MEDIA_LOG_PROPERTY_SUPPORTS_TYPE(kIsVideoDecryptingDemuxerStream, bool);
MEDIA_LOG_PROPERTY_SUPPORTS_TYPE(kAudioDecoderName, std::string);
MEDIA_LOG_PROPERTY_SUPPORTS_TYPE(kIsPlatformAudioDecoder, bool);
MEDIA_LOG_PROPERTY_SUPPORTS_TYPE(kIsAudioDecryptingDemuxerStream, bool);
MEDIA_LOG_PROPERTY_SUPPORTS_TYPE(kAudioTracks, std::vector<AudioDecoderConfig>);
MEDIA_LOG_PROPERTY_SUPPORTS_TYPE(kVideoTracks, std::vector<VideoDecoderConfig>);

// Convert the enum to a string (used for the front-end enum matching).
MEDIA_EXPORT std::string MediaLogPropertyKeyToString(MediaLogProperty property);

}  // namespace media

#endif  // MEDIA_BASE_MEDIA_LOG_PROPERTIES_H_
