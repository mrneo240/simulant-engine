#ifndef RECTANGLE_H_INCLUDED
#define RECTANGLE_H_INCLUDED

#include "kglt/mesh.h"

namespace kglt {
namespace procedural {
namespace mesh {

void rectangle(kglt::Mesh& mesh, float width, float height, float x_offset=0.0, float y_offset=0.0);
void rectangle_outline(kglt::Mesh& mesh, float width, float height, float x_offset=0.0, float y_offset=0.0);

}
}
}


#endif // RECTANGLE_H_INCLUDED
