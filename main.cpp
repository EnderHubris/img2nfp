// https://github.com/nothings/stb/raw/refs/heads/master/stb_image.h
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"

// https://github.com/CLIUtils/CLI11/releases/download/v2.3.2/CLI11.hpp
#include "CLI11.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <math.h>

typedef unsigned char uchar;
class Pixel;
typedef std::vector<std::vector<Pixel>> PixelGroup;

class Pixel {
public:
    Pixel() {};
    Pixel(int r, int g, int b): r(r), g(g), b(b) {};
    Pixel(int r, int g, int b, char c): r(r), g(g), b(b), nfp_char(c) {};
    Pixel(uchar r, uchar g, uchar b): Pixel((int)r, (int)g, (int)b) {};

    Pixel(const Pixel& other) {
        r = other.r;
        g = other.g;
        b = other.b;
        nfp_char = other.nfp_char;
    };
    Pixel& operator=(const Pixel& other) {
        if (this != &other) {
            r = other.r;
            g = other.g;
            b = other.b;
            nfp_char = other.nfp_char;
        }
        return *this;
    }

    bool operator==(const Pixel& other) const {
        if (r != other.r) return false;
        if (g != other.g) return false;
        return b == other.b;
    }

    friend std::ostream& operator<<(std::ostream& out, const Pixel& p) {
        out << "(" << p.r << "," << p.g << "," << p.b << ")";
        return out;
    }

    void render() {
        std::cout << "\033[38;2;" << r << ";" << g << ";" << b << "m" << "█" << "\033[0m";
    }

    int r = 0;
    int g = 0;
    int b = 0;
    char nfp_char = '\0';
};

/**
 * Return the basename of a given path
 */
std::string basename(const std::string& path) {
    // path -> lorem.png
    if (path.find('/') == std::string::npos)
        return path;

    // path -> /var/www/imgs/lorem.png | ../../lorem.png | ./imgs/lorem.png
    return path.substr(path.find_last_of('/')+1);
}

std::vector<Pixel> color_map = {
    {240,240,240, '0'},
    {242,178,51,  '1'},
    {229,127,216, '2'},
    {153,178,242, '3'},
    {222,222,108, '4'},
    {127,204,25,  '5'},
    {242,178,204, '6'},
    {76,76,76,    '7'},
    {153,153,153, '8'},
    {76,153,178,  '9'},
    {178,102,229, 'a'},
    {51,102,204,  'b'},
    {127,102,76,  'c'},
    {87,166,78,   'd'},
    {204,76,76,   'e'},
    {17,17,17,    'f'}
};

double pixelDistance(const Pixel& a, const Pixel& b) {
    int deltaR = b.r - a.r;
    int deltaG = b.g - a.g;
    int deltaB = b.b - a.b;
    
    int alpha = 0;
    alpha += pow(deltaR, 2);
    alpha += pow(deltaG, 2);
    alpha += pow(deltaB, 2);

    return sqrt(alpha);
}

Pixel translatePixel(const uchar& r, const uchar& g, const uchar& b) {
    Pixel p = {
        (int)(r),
        (int)(g),
        (int)(b)
    };

    double closest = -1;
    Pixel ret;
    for (auto entry : color_map) {
        double dist = pixelDistance(p, entry);
        if (closest == -1 || dist < closest) {
            closest = dist;
            ret = entry;
        }
    }

    return ret;
}

/**
 * Generate a Nitrogen Fingers Paint (Tweaked: CC) file
 * based on the given pixel collection
 */
void generateNFP(const std::string outputFile, const PixelGroup& pixels) {
    std::ofstream nfpFile(outputFile.c_str());
    if (!nfpFile.is_open()) {
        std::cerr << "[-] Error: Could not create '" << outputFile <<  "'\n";
        return;
    }

    for (const std::vector<Pixel>& row : pixels) {
        for (const Pixel& p : row) {
            // get char from pixel
            nfpFile << p.nfp_char;
        }
        nfpFile << "\n";
    }

    std::cout << "[+] Generated: " << outputFile << "\n";
    nfpFile.close();
}

int main(int argc, char** argv) {
    std::string inputFile;
    std::string outputFile;

    int targetWidth = 0;
    int targetHeight = 0;

    bool preview = false;

    CLI::App app{"img2nfp (Image Conversion Tool)"};
    app.add_option("input",         inputFile,      "Path to image file [png,jpg,jpeg]")
        ->required()
        ->type_name("Image File");
    app.add_option("-o, --output",  outputFile,     "Name of ntp output file");
    app.add_flag("-p, --preview",   preview,        "Preview Output in terminal");
    app.add_option("-W,--width",    targetWidth,    "Target width to scale to")
        ->required();
    app.add_option("-H,--height",   targetHeight,   "Target height to scale to")
        ->required();
    
    CLI11_PARSE(app, argc, argv);

    // if the output file is not provided, simply append Nitrogen Fingers Paint (Tweaked: CC)
    if (outputFile.empty()) { outputFile = basename(inputFile) + ".nfp"; }

    // load only 3 color channels (RBG)
    int width, height, channels;
    uchar* data = stbi_load(inputFile.c_str(), &width, &height, &channels, 3);

    if (!data) {
        std::cerr << "[-] Failed to load image\n";
        return 1;
    }

    // create a rescaled copy in memory (preserve original input)
    if (targetWidth > 0 && targetHeight > 0) {
        uchar* resized = (uchar*)malloc(targetWidth * targetHeight * 3);

        stbir_resize_uint8_linear(
            data, width, height, 0,
            resized, targetWidth, targetHeight, 0,
            STBIR_RGB
        );
        stbi_image_free(data);
        
        data = resized;
        width = targetWidth;
        height = targetHeight;
    } else {
        std::cerr << "[-] Width and Height must be positive integers!";
        return 1;
    }

    PixelGroup pixels;

    for (int y = 0; y < targetHeight; y++) {
        pixels.push_back({}); // create new row
        for (int x = 0; x < targetWidth; x++) {
            int index = (y * targetWidth + x) * 3;
            uchar r = data[index];
            uchar g = data[index + 1];
            uchar b = data[index + 2];

            // Pixel(r,g,b).render();
            pixels.back().push_back(translatePixel(r,g,b));
            if (preview) pixels.back().back().render();
        }
        if (preview) std::cout << "\n";
    }

    generateNFP(outputFile, pixels);

    std::cout << "[*] Image Scale: " << targetWidth << "x" << targetHeight << "\n";

    stbi_image_free(data);
    return 0;
}