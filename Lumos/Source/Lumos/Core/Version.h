#pragma once

namespace Lumos
{
    struct Version
    {
        int major = 0;
        int minor = 3;
        int patch = 8;
    };

    constexpr Version const LumosVersion = Version();
}
