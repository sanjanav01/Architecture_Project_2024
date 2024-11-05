#include "binaryio.hpp"
#include <fstream>
#include <stdexcept>
#include <iostream>
#include "image_types.hpp"
#include <cstdint>
#include <vector>

constexpr int MaxByteValue = 255;

Image read_ppm(const std::string& file_path) {
    std::ifstream file(file_path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Error: Could not open file " + file_path);
    }

    std::string magic_number;
    file >> magic_number;
    if (magic_number != "P6") {
        throw std::runtime_error("Error: Invalid PPM format (not P6)");
    }

    Image image;
    file >> image.width >> image.height >> image.max_color_value;
    if (image.width <= 0 || image.height <= 0 || image.max_color_value <= 0) {
        throw std::runtime_error("Error: Invalid width, height, or max color value in PPM header");
    }

    file.ignore(); // Ignore single whitespace character after max color value

    size_t total_pixels = static_cast<size_t>(image.width) * static_cast<size_t>(image.height);
    image.pixels.resize(total_pixels);

    if (image.max_color_value < MaxByteValue) {
        // Handle 8-bit color with scaling
        for (size_t i = 0; i < total_pixels; ++i) {
            uint8_t rgb[3];
            file.read(reinterpret_cast<char*>(rgb), 3);
            if (file.gcount() != 3) {
                throw std::runtime_error("Error reading pixel data for 8-bit color with scaling");
            }
            // Debug output for each pixel
            std::cout << "Pixel " << i << " - R: " << static_cast<int>(rgb[0])
                      << " G: " << static_cast<int>(rgb[1])
                      << " B: " << static_cast<int>(rgb[2]) << std::endl;

            image.pixels[i] = {
                static_cast<uint8_t>(rgb[0] * MaxByteValue / image.max_color_value),
                static_cast<uint8_t>(rgb[1] * MaxByteValue / image.max_color_value),
                static_cast<uint8_t>(rgb[2] * MaxByteValue / image.max_color_value)
            };
        }
    } else if (image.max_color_value == MaxByteValue) {
        // Handle 8-bit color without scaling
        for (size_t i = 0; i < total_pixels; ++i) {
            uint8_t rgb[3];
            file.read(reinterpret_cast<char*>(rgb), 3);
            if (file.gcount() != 3) {
                throw std::runtime_error("Error reading pixel data for 8-bit color without scaling");
            }
            // Debug output for each pixel
            std::cout << "Pixel " << i << " - R: " << static_cast<int>(rgb[0])
                      << " G: " << static_cast<int>(rgb[1])
                      << " B: " << static_cast<int>(rgb[2]) << std::endl;

            image.pixels[i] = { rgb[0], rgb[1], rgb[2] };
        }
    } else {
        // Handle 16-bit color (2 bytes per channel) with scaling
        for (size_t i = 0; i < total_pixels; ++i) {
            uint8_t r_high, r_low, g_high, g_low, b_high, b_low;
            file.read(reinterpret_cast<char*>(&r_high), 1);
            file.read(reinterpret_cast<char*>(&r_low), 1);
            file.read(reinterpret_cast<char*>(&g_high), 1);
            file.read(reinterpret_cast<char*>(&g_low), 1);
            file.read(reinterpret_cast<char*>(&b_high), 1);
            file.read(reinterpret_cast<char*>(&b_low), 1);

            if (file.gcount() != 1) {
                throw std::runtime_error("Error reading pixel data for 16-bit color");
            }

            // Combine high and low bytes for each color channel
            uint16_t r = static_cast<uint16_t>((static_cast<uint16_t>(r_high) << 8) | r_low);
            uint16_t g = static_cast<uint16_t>((static_cast<uint16_t>(g_high) << 8) | g_low);
            uint16_t b = static_cast<uint16_t>((static_cast<uint16_t>(b_high) << 8) | b_low);

            // Debug output for each pixel before scaling
            std::cout << "Pixel " << i << " - R: " << r << " G: " << g << " B: " << b << std::endl;

            // Scale down from 16-bit to 8-bit range
            image.pixels[i] = {
                static_cast<uint8_t>(r * MaxByteValue / image.max_color_value),
                static_cast<uint8_t>(g * MaxByteValue / image.max_color_value),
                static_cast<uint8_t>(b * MaxByteValue / image.max_color_value)
            };
        }
    }

    if (file.fail()) {
        throw std::runtime_error("Error: Unexpected end of file or read error");
    }

    file.close();
    return image;
}

void write_ppm(const std::string& file_path, const Image& image) {
    // Open the file in binary mode
    std::ofstream out_file(file_path, std::ios::binary);
    if (!out_file) {
        throw std::runtime_error("Could not open file for writing: " + file_path);
    }

    // Write the PPM header
    out_file << "P6\n" << image.width << " " << image.height << "\n" << image.max_color_value << "\n";

    // Determine if each color channel is 1 byte or 2 bytes based on max_color_value
    bool use_1_byte_per_channel = (image.max_color_value <= MaxByteValue);

    // Write pixel data
    for (const auto& pixel : image.pixels) {
        if (use_1_byte_per_channel) {
            // 1 byte per color channel (8-bit)
            uint8_t r = static_cast<uint8_t>(pixel.r);
            uint8_t g = static_cast<uint8_t>(pixel.g);
            uint8_t b = static_cast<uint8_t>(pixel.b);
            out_file.write(reinterpret_cast<const char*>(&r), sizeof(r));
            out_file.write(reinterpret_cast<const char*>(&g), sizeof(g));
            out_file.write(reinterpret_cast<const char*>(&b), sizeof(b));
        } else {
            // 2 bytes per color channel (16-bit), in big endian
            uint16_t r = pixel.r;
            uint16_t g = pixel.g;
            uint16_t b = pixel.b;

            uint8_t r_high = static_cast<uint8_t>((r >> 8) & 0xFF);
            uint8_t r_low = static_cast<uint8_t>(r & 0xFF);
            uint8_t g_high = static_cast<uint8_t>((g >> 8) & 0xFF);
            uint8_t g_low = static_cast<uint8_t>(g & 0xFF);
            uint8_t b_high = static_cast<uint8_t>((b >> 8) & 0xFF);
            uint8_t b_low = static_cast<uint8_t>(b & 0xFF);

            out_file.write(reinterpret_cast<const char*>(&r_high), sizeof(r_high));
            out_file.write(reinterpret_cast<const char*>(&r_low), sizeof(r_low));
            out_file.write(reinterpret_cast<const char*>(&g_high), sizeof(g_high));
            out_file.write(reinterpret_cast<const char*>(&g_low), sizeof(g_low));
            out_file.write(reinterpret_cast<const char*>(&b_high), sizeof(b_high));
            out_file.write(reinterpret_cast<const char*>(&b_low), sizeof(b_low));
        }
    }

    if (!out_file) {
        throw std::runtime_error("Error writing to file: " + file_path);
    }
}

void write_cppm(const std::string& file_path, const CompressedImage& image) {
    std::ofstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Error: Could not open file for writing: " + file_path);
    }

    // Write the CPPM header
    file << "C6\n";  // Magic number
    file << image.width << " " << image.height << "\n";
    file << image.max_color << "\n";
    file << image.color_table.size() << "\n";  // Color table size

    // Write the color table
    for (const auto& color : image.color_table) {
        file.write(reinterpret_cast<const char*>(&color), sizeof(uint32_t));
    }

    // Determine the byte length for each pixel index based on color table size
    size_t index_byte_length;
    if (image.color_table.size() <= 256) {
        index_byte_length = 1;  // 1 byte per index
    } else if (image.color_table.size() <= 65536) {
        index_byte_length = 2;  // 2 bytes per index
    } else {
        index_byte_length = 4;  // 4 bytes per index
    }

    // Write the compressed pixel data as indices referring to the color table
    for (const auto& index : image.pixel_indices) {
        if (index_byte_length == 1) {
            uint8_t idx = static_cast<uint8_t>(index);
            file.write(reinterpret_cast<const char*>(&idx), 1);
        } else if (index_byte_length == 2) {
            uint16_t idx = static_cast<uint16_t>(index);
            file.write(reinterpret_cast<const char*>(&idx), 2);
        } else {
            uint32_t idx = static_cast<uint32_t>(index);
            file.write(reinterpret_cast<const char*>(&idx), 4);
        }
    }

    if (!file) {
        throw std::runtime_error("Error: Failed to write to file: " + file_path);
    }

    file.close();
}

//for testing
CompressedImage read_cppm(const std::string& file_path) {
    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Error: Could not open file for reading: " + file_path);
    }

    CompressedImage image;

    // Read the CPPM header
    std::string magic;
    file >> magic;
    if (magic != "CPPM") {
        throw std::runtime_error("Error: Invalid CPPM format");
    }
    file >> image.width >> image.height >> image.max_color;
    size_t color_table_size;
    file >> color_table_size;
    file.ignore();  // Skip newline after the header

    // Read the color table
    image.color_table.resize(color_table_size);
    for (auto& color : image.color_table) {
        file.read(reinterpret_cast<char*>(&color), sizeof(uint32_t));
    }

    // Determine byte length for each pixel index
    size_t index_byte_length;
    if (color_table_size <= 256) {
        index_byte_length = 1;
    } else if (color_table_size <= 65536) {
        index_byte_length = 2;
    } else {
        index_byte_length = 4;
    }

    // Read the compressed pixel data
    while (file) {
        uint32_t index = 0;
        if (index_byte_length == 1) {
            uint8_t idx;
            file.read(reinterpret_cast<char*>(&idx), 1);
            index = idx;
        } else if (index_byte_length == 2) {
            uint16_t idx;
            file.read(reinterpret_cast<char*>(&idx), 2);
            index = idx;
        } else {
            uint32_t idx;
            file.read(reinterpret_cast<char*>(&idx), 4);
            index = idx;
        }
        if (file) {
            image.pixel_indices.push_back(index);
        }
    }

    file.close();
    return image;
}
