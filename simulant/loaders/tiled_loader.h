#ifndef TILED_LOADER_H
#define TILED_LOADER_H

#include "../loader.h"

namespace smlt {
namespace loaders {

class TiledLoader : public Loader {
public:
    TiledLoader(const unicode& filename, std::shared_ptr<std::stringstream> data):
        Loader(filename, data) {}

    void into(Loadable& resource, const LoaderOptions& options = LoaderOptions());
};

class TiledLoaderType : public LoaderType {
public:
    TiledLoaderType() {

    }

    ~TiledLoaderType() {}

    unicode name() { return "tiled"; }
    bool supports(const unicode& filename) const override {        
        bool ret = filename.lower().ends_with(".tmx");
        return ret;
    }

    Loader::ptr loader_for(const unicode& filename, std::shared_ptr<std::stringstream> data) const {
        return Loader::ptr(new TiledLoader(filename, data));
    }
};

}
}

#endif // TILED_LOADER_H