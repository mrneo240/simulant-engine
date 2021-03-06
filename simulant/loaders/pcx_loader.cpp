//
//   Copyright (c) 2011-2017 Luke Benstead https://simulant-engine.appspot.com
//
//     This file is part of Simulant.
//
//     Simulant is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     Simulant is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with Simulant.  If not, see <http://www.gnu.org/licenses/>.
//

#include "pcx_loader.h"

namespace smlt {
namespace loaders {

#pragma pack(push)
#pragma pack(1)

/* PCX header */
struct Header {
    uint8_t manufacturer;
    uint8_t version;
    uint8_t encoding;
    uint8_t bits_per_pixel;

    uint16_t xmin, ymin;
    uint16_t xmax, ymax;
    uint16_t horz_res, vert_res;

    uint8_t palette[48];
    uint8_t reserved;
    uint8_t num_color_planes;

    uint16_t bytes_per_scan_line;
    uint16_t palette_type;
    uint16_t horz_size, vert_size;

    uint8_t padding[54];
};

#pragma pack(pop)

TextureLoadResult PCXLoader::do_load(const std::vector<uint8_t> &buffer) {
    TextureLoadResult result;

    Header* header = (Header*)&buffer[0];

    if(header->manufacturer != 0x0a) {
        throw std::runtime_error("Unsupported PCX manufacturer");
    }

    result.width = header->xmax - header->xmin + 1;
    result.height = header->ymax - header->ymin + 1;
    result.channels = 3;
    result.format = TEXTURE_FORMAT_RGB888;
    result.texel_type = TEXTURE_TEXEL_TYPE_UNSIGNED_BYTE;
    result.data.resize(result.width * result.height * result.channels);

    auto bitcount = header->bits_per_pixel * header->num_color_planes;
    if(bitcount != 8) {
        throw std::runtime_error("Unsupported PCX bitcount");
    }

    uint8_t palette_marker = buffer[buffer.size() - 769];

    const uint8_t* palette = (palette_marker == 12) ? &buffer[buffer.size() - 768] : header->palette;

    int32_t rle_count = 0;
    int32_t rle_value = 0;

    const uint8_t* image_data = &buffer[128];

    for(uint32_t idx = 0; idx < (result.width * result.height * result.channels); idx += result.channels) {
        if(rle_count == 0) {
            rle_value = *image_data;
            ++image_data;
            if((rle_value & 0xc0) == 0xc0) {
                rle_count = rle_value & 0x3f;
                rle_value = *image_data;
                ++image_data;
            } else {
                rle_count = 1;
            }
        }

        rle_count--;

        assert(rle_value * 3 < 768);

        result.data[idx + 0] = palette[(rle_value * 3) + 0];
        result.data[idx + 1] = palette[(rle_value * 3) + 1];
        result.data[idx + 2] = palette[(rle_value * 3) + 2];
    }

    return result;
}

}
}
