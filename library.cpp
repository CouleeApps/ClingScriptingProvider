#include "library.h"
#include "ClingScriptingProvider.h"

using namespace BinaryNinja;


extern "C" {
    BN_DECLARE_CORE_ABI_VERSION

    BINARYNINJAPLUGIN bool CorePluginInit()
    {
        ClingScriptingProvider::init();
        return true;
    }
}
