#include "LM.h"
#include "Core/OS/OS.h"

namespace Lumos
{
    class WindowsOS : public OS
    {
	public:
        WindowsOS() = default;
        ~WindowsOS() = default;

        void Run() override;
	};
}