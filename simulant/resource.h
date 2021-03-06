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

#ifndef RESOURCE_H
#define RESOURCE_H

#include <cassert>
#include <mutex>
#include <string>
#include <chrono>

#include "generic/property.h"
#include "generic/data_carrier.h"

namespace smlt {

class ResourceManager;

class Resource {
public:
    Resource(ResourceManager* manager):
        manager_(manager) {
        created_ = std::chrono::system_clock::now();
    }

    virtual ~Resource() {}

    ResourceManager& resource_manager() { assert(manager_); return *manager_; }
    const ResourceManager& resource_manager() const { assert(manager_); return *manager_; }

    int age() const {
        return std::chrono::duration_cast<std::chrono::seconds>(
                    created_ - std::chrono::system_clock::now()
               ).count();
    }

    Property<Resource, generic::DataCarrier> data = { this, &Resource::data_ };
private:
    ResourceManager* manager_;

    std::chrono::time_point<std::chrono::system_clock> created_;

    generic::DataCarrier data_;
};

}
#endif // RESOURCE_H
