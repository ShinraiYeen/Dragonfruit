#pragma once

#include <pulse/pulseaudio.h>

#include "dragonfruit_engine/sound.hpp"

namespace dragonfruit::utils {

/**
 * @brief Get the pulse audio format based on a given wav format and bit depth.
 *
 * @param fmt_code Wav format code.
 * @param bit_depth the bit depth of the sample data.
 * @return A pa_sample_format based on the given parameters.
 */
inline pa_sample_format GetPulseFormat(WavFormatCode fmt_code, int bit_depth) {
    switch (fmt_code) {
        case WavFormatCode::PCM: {
            switch (bit_depth) {
                case 8:
                    return pa_sample_format::PA_SAMPLE_U8;
                case 16:
                    return pa_sample_format::PA_SAMPLE_S16LE;
                case 24:
                    return pa_sample_format::PA_SAMPLE_S24LE;
                case 32:
                    return pa_sample_format::PA_SAMPLE_S32LE;
                default:
                    return pa_sample_format::PA_SAMPLE_INVALID;
            }
        }

        case WavFormatCode::IEEE_FLOAT: {
            switch (bit_depth) {
                case 32:
                    return pa_sample_format::PA_SAMPLE_FLOAT32LE;
                default:
                    return pa_sample_format::PA_SAMPLE_INVALID;
            }
        }

        default: {
            return pa_sample_format::PA_SAMPLE_INVALID;
        }
    }
}
}  // namespace dragonfruit::utils