//
//   Copyright (c) 2011-2017 Luke Benstead https://simulant-engine.appspot.com
//
//     This file is part of Simulant.
//
//     Simulant is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     Simulant is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with Simulant.  If not, see <http://www.gnu.org/licenses/>.
//

#include <cassert>

#include "../../deps/kazlog/kazlog.h"
#include "buffer_object.h"
#include "../../utils/gl_thread_check.h"
#include "../../utils/gl_error.h"

namespace smlt {

BufferObject::BufferObject(BufferObjectType type, BufferObjectUsage usage):
    usage_(usage),
    gl_target_(0),
    buffer_id_(0) {

    //FIXME: Totally need to support more than this
    switch(type) {
        case BUFFER_OBJECT_VERTEX_DATA:
            gl_target_ = GL_ARRAY_BUFFER;
        break;
        case BUFFER_OBJECT_INDEX_DATA:
            gl_target_ = GL_ELEMENT_ARRAY_BUFFER;
        break;
        default:
            L_WARN("We don't yet support this shizzle");
    }    
}

BufferObject::~BufferObject() {
    try {
        release();
    } catch(...) {
        return;
    }
}

void BufferObject::release() {
    if(buffer_id_) {
        GLCheck(glDeleteBuffers, 1, &buffer_id_);
    }
}

void BufferObject::bind() {
    if(!buffer_id_) {
        GLCheck(glGenBuffers, 1, &buffer_id_);
    }

    GLCheck(glBindBuffer, gl_target_, buffer_id_);
}

GLenum BufferObject::usage() const {
    GLenum usage;
    switch(usage_) {
        case MODIFY_ONCE_USED_FOR_LIMITED_RENDERING:
            usage = GL_STREAM_DRAW;
        break;
        case MODIFY_ONCE_USED_FOR_LIMITED_QUERYING:
#ifndef __ANDROID__
            usage = GL_STREAM_READ;
#else
            usage = GL_STREAM_DRAW;
#endif
        break;
        case MODIFY_ONCE_USED_FOR_LIMITED_QUERYING_AND_RENDERING:
#ifndef __ANDROID__
            usage = GL_STREAM_COPY;
#else
            usage = GL_STREAM_DRAW;
#endif
        break;
        case MODIFY_ONCE_USED_FOR_RENDERING:
            usage = GL_STATIC_DRAW;
        break;
        case MODIFY_ONCE_USED_FOR_QUERYING:
#ifndef __ANDROID__
            usage = GL_STATIC_READ;
#else
            usage = GL_STATIC_DRAW;
#endif
        break;
        case MODIFY_ONCE_USED_FOR_QUERYING_AND_RENDERING:
#ifndef __ANDROID__
            usage = GL_STATIC_COPY;
#else
            usage = GL_STATIC_DRAW;
#endif
        break;
        case MODIFY_REPEATEDLY_USED_FOR_RENDERING:
            usage = GL_DYNAMIC_DRAW;
        break;
        case MODIFY_REPEATEDLY_USED_FOR_QUERYING:
#ifndef __ANDROID__
            usage = GL_DYNAMIC_READ;
#else
            usage = GL_DYNAMIC_DRAW;
#endif
        break;
        case MODIFY_REPEATEDLY_USED_FOR_QUERYING_AND_RENDERING:
#ifndef __ANDROID__
            usage = GL_DYNAMIC_COPY;
#else
            usage = GL_DYNAMIC_DRAW;
#endif
        break;
        default:
            throw std::logic_error("What the...?");
    }

    return usage;
}

void BufferObject::build(uint32_t byte_size, const void* data) {
    if(!buffer_id_) {
        GLCheck(glGenBuffers, 1, &buffer_id_);
    }

    assert(buffer_id_);

    GLCheck(glBindBuffer, gl_target_, buffer_id_);
    GLCheck(glBufferData, gl_target_, byte_size, data, usage());
}

void BufferObject::modify(uint32_t offset, uint32_t byte_size, const void* data) {
    assert(buffer_id_);

    GLCheck(glBindBuffer, gl_target_, buffer_id_);
    GLCheck(glBufferSubData, gl_target_, offset, byte_size, data);
}

VertexArrayObject::VertexArrayObject(BufferObjectUsage vertex_usage, BufferObjectUsage index_usage){
    vertex_buffer_ = BufferObject::create(BUFFER_OBJECT_VERTEX_DATA, vertex_usage);
    index_buffer_ = BufferObject::create(BUFFER_OBJECT_INDEX_DATA, index_usage);
}

VertexArrayObject::VertexArrayObject(BufferObject::ptr vertex_buffer, BufferObjectUsage index_usage):
    vertex_buffer_(vertex_buffer) {

    assert(vertex_buffer);
    assert(vertex_buffer->target() == GL_ARRAY_BUFFER);

    index_buffer_ = BufferObject::create(BUFFER_OBJECT_INDEX_DATA, index_usage);
}

VertexArrayObject::~VertexArrayObject() {

}

void VertexArrayObject::bind() {
    vertex_buffer_bind();
    index_buffer_bind();
}

void VertexArrayObject::vertex_buffer_update(uint32_t byte_size, const void* data) {
    vertex_buffer_->build(byte_size, data);
}

void VertexArrayObject::vertex_buffer_update_partial(uint32_t offset, uint32_t byte_size, const void* data) {
    vertex_buffer_->modify(offset, byte_size, data);
}

void VertexArrayObject::index_buffer_update(uint32_t byte_size, const void* data) {
    index_buffer_->build(byte_size, data);
}

void VertexArrayObject::index_buffer_update_partial(uint32_t offset, uint32_t byte_size, const void* data) {
    index_buffer_->modify(offset, byte_size, data);
}

}
