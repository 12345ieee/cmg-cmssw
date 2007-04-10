#include "IOPool/TrivialFileCatalog/interface/TrivialFileCatalog.h"
#include "FileCatalog/FCImplPluginFactory.h"
#include "FileCatalog/FCMetaImplPluginFactory.h"
#include "FWCore/PluginManager/interface/ModuleDef.h"

namespace pool
{
DEFINE_SEAL_MODULE();
DEFINE_SEAL_PLUGIN( FCImplPluginFactory, TrivialFileCatalog, "trivialcatalog");
} // End of namespace pool
