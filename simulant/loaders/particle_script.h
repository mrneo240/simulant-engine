/* *   Copyright (c) 2011-2017 Luke Benstead https://simulant-engine.appspot.com
 *
 *     This file is part of Simulant.
 *
 *     Simulant is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 *
 *     Simulant is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with Simulant.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PARTICLE_SCRIPT_H
#define PARTICLE_SCRIPT_H

#include "../loader.h"

namespace smlt {
namespace loaders {

class KGLPLoader : public Loader {
public:
    KGLPLoader(const unicode& filename, std::shared_ptr<std::stringstream> data):
        Loader(filename, data) {}

    void into(Loadable& resource, const LoaderOptions& options = LoaderOptions());
};

class KGLPLoaderType : public LoaderType {
public:
    KGLPLoaderType() {

    }

    ~KGLPLoaderType() {}

    unicode name() { return "particle"; }
    bool supports(const unicode& filename) const override {
        return filename.lower().contains(".kglp");
    }

    Loader::ptr loader_for(const unicode& filename, std::shared_ptr<std::stringstream> data) const {
        return Loader::ptr(new KGLPLoader(filename, data));
    }
};

}
}

#endif // PARTICLE_SCRIPT_H
