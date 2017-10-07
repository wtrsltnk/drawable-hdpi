#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <direct.h>

#include <system.io.fileinfo.h>
#include <system.io.path.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

using namespace System::IO;

int main(int argc, char* argv[])
{
    std::map<std::string, int> drawable_sizes = {
        std::make_pair("drawable-hdpi", 72),
        std::make_pair("drawable-mdpi", 48),
        std::make_pair("drawable-xhdpi", 96),
        std::make_pair("drawable-xxhdpi", 144)
    };

    if (argc < 2)
    {
        std::cout << "ERROR: too few arguments, I need an input image filename\n";
        return -1;
    }

    auto file = FileInfo(argv[1]);

    if (!file.Exists())
    {
        std::cout << "ERROR: given file does not exist: " << file.FullName() << "\n";
        return -2;
    }

    auto directory = file.Directory();

    int x, y, channels;
    auto pixels = stbi_load(file.FullName().c_str(), &x, &y, &channels, 4);

    if (pixels == nullptr)
    {
        std::cout << "ERROR: could not load image from given filename: " << file.FullName() << "\n";
        return -2;
    }

    for (std::pair<std::string, int> p : drawable_sizes)
    {
        auto drawableDir = DirectoryInfo(Path::Combine(directory.FullName(), p.first));
        if (!drawableDir.Exists())
        {
            mkdir(drawableDir.FullName().c_str());
        }

        stbi_uc* outputpixels = new stbi_uc[p.second * p.second * channels];
        if (stbir_resize_uint8(pixels, x, y, 0, outputpixels, p.second, p.second, 0, channels))
        {
            auto ic_launcher = FileInfo(Path::Combine(drawableDir.FullName(), "ic_launcher.png"));
            if (stbi_write_png(ic_launcher.FullName().c_str(), p.second, p.second, channels, outputpixels, 0))
            {
                std::cout << "Successfully written " << p.first << " with size " << p.second << "x" << p.second << " to " << drawableDir.FullName() << "\n";
            }
            else
            {
                std::cout << "Failed to write " << p.first << " with size " << p.second << "x" << p.second << " to " << drawableDir.FullName() << "\n";
            }
        }
    }

    return 0;
}
