#include <gtest/gtest.h>
#include <iostream>
#include <filesystem>
#include "Engine/Image.h"
#include "Engine/Utils.h"

namespace Engine::Test
{
    TEST(Image, DISABLED_ConvertFolder)
    {
        std::string folder = "..\\Tactics\\Data\\Particles";
        for (const auto & entry : std::filesystem::directory_iterator(folder))
        {
            if (!entry.is_regular_file())
                continue;
            if (UpperCase(entry.path().extension().string()) != ".PNG")
                continue;
            Image::Data data(entry.path().string());
            Image::Data luminAlpha(data.width, data.height, 2);
            for (unsigned p = 0; p < data.Pixels(); ++p)
            {
                luminAlpha.data[p * 2] = 0xFF;
                auto pixel = data.data[p*data.channels];
                luminAlpha.data[p * 2 + 1] = pixel;
            }
            auto newPath = std::filesystem::path(folder) / "LuminAlpha" / entry.path().filename();
            luminAlpha.Write(newPath.string());
        }
    }
}