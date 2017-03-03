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

#ifndef GENERIC_RENDERER_H
#define GENERIC_RENDERER_H

#include <vector>

#include "../renderer.h"
#include "../../material.h"
#include "./buffer_manager.h"
#include "../batching/render_queue.h"

namespace smlt {

class GL2RenderGroupImpl;
class GenericRenderer;

struct RenderState {
    Renderable* renderable;
    MaterialPass* pass;
    Light* light;
    batcher::Iteration iteration;
    GL2RenderGroupImpl* render_group_impl;
};

class GL2RenderQueueVisitor : public batcher::RenderQueueVisitor {
public:
    GL2RenderQueueVisitor(GenericRenderer* renderer, CameraPtr camera, const Colour& colour);

    void start_traversal(const batcher::RenderQueue& queue, uint64_t frame_id);
    void visit(Renderable* renderable, MaterialPass* pass, Light* light, batcher::Iteration);
    void end_traversal(const batcher::RenderQueue &queue);
    void change_render_group(const batcher::RenderGroup *prev, const batcher::RenderGroup *next);
    void change_material_pass(const MaterialPass* prev, const MaterialPass* next);

private:
    GenericRenderer* renderer_;
    CameraPtr camera_;
    Colour global_ambient_;

    ShaderID last_shader_id_;
    GL2RenderGroupImpl* current_group_ = nullptr;
    bool render_group_changed_ = true;

    bool queue_blended_objects_ = true;

    /*
     * All entries are ordered by distance from the near frustum descending (back-to-front)
     */
    std::multimap<float, RenderState, std::greater<float> > blended_object_queue_;

    void do_visit(Renderable* renderable, MaterialPass* material_pass, Light* light, batcher::Iteration iteration);
};

class GenericRenderer : public Renderer {
public:
    GenericRenderer(WindowBase* window):
        Renderer(window),
        buffer_manager_(new GL2BufferManager(this)) {

    }

    batcher::RenderGroup new_render_group(Renderable *renderable, MaterialPass *material_pass);
    void init_context();

    std::shared_ptr<batcher::RenderQueueVisitor> get_render_queue_visitor(CameraPtr camera, const Colour &global_ambient);
private:
    std::unique_ptr<HardwareBufferManager> buffer_manager_;

    HardwareBufferManager* _get_buffer_manager() const {
        return buffer_manager_.get();
    }

    void set_light_uniforms(GPUProgramInstance* program_instance, Light* light);
    void set_material_uniforms(GPUProgramInstance* program_instance, MaterialPass *pass);
    void set_auto_uniforms_on_shader(GPUProgramInstance *pass, CameraPtr camera, Renderable* subactor, const Colour &global_ambient);
    void set_auto_attributes_on_shader(Renderable &buffer);
    void set_blending_mode(BlendType type);
    void send_geometry(Renderable* renderable);

    friend class GL2RenderQueueVisitor;
};

}

#endif // GENERIC_RENDERER_H
