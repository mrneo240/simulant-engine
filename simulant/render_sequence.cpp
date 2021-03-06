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

#include <unordered_map>

#include "profiler.h"

#include "generic/algorithm.h"
#include "render_sequence.h"
#include "stage.h"
#include "nodes/actor.h"
#include "nodes/camera.h"
#include "nodes/light.h"

#include "meshes/mesh.h"
#include "window.h"
#include "partitioner.h"
#include "loader.h"

namespace smlt {

Pipeline::Pipeline(PipelineID id,
        RenderSequence* render_sequence):
    generic::Identifiable<PipelineID>(id),
    sequence_(render_sequence),
    priority_(0),
    is_active_(false) {

}

void Pipeline::set_priority(int32_t priority) {
    if(priority_ != priority) {
        priority_ = priority;

        /* If the priority changed, we need to update the render sequence */
        sequence_->sort_pipelines(/*acquire_lock=*/true);
    }
}

Pipeline::~Pipeline() {
    deactivate();
}

void Pipeline::deactivate() {
    if(!is_active_) return;

    is_active_ = false;

    if(stage_) {
        sequence_->window->stage(stage_)->decrement_render_count();
    }
}

void Pipeline::activate() {
    if(is_active_) return;

    is_active_ = true;

    if(stage_) {
        sequence_->window->stage(stage_)->increment_render_count();
    }
}

RenderSequence::RenderSequence(Window *window):
    window_(window),
    renderer_(window->renderer.get()) {

    //Set up the default render options
    render_options.wireframe_enabled = false;
    render_options.texture_enabled = true;
    render_options.backface_culling_enabled = true;
    render_options.point_size = 1;
}

void RenderSequence::activate_pipelines(const std::vector<PipelineID>& pipelines) {
    for(PipelineID p: pipelines) {
        auto pip = pipeline(p);
        if(!pip->is_active()) {
            pip->activate();
        }
    }
}

std::vector<PipelineID> RenderSequence::active_pipelines() const {
    std::vector<PipelineID> result;

    for(Pipeline::ptr p: ordered_pipelines_) {
        if(p->is_active()) {
            result.push_back(p->id());
        }
    }

    return result;
}

void RenderSequence::deactivate_all_pipelines() {
    for(Pipeline::ptr p: ordered_pipelines_) {
        if(p->is_active()) {
            p->deactivate();
        }
    }
}

PipelinePtr RenderSequence::pipeline(PipelineID pipeline) {
    return PipelineManager::get(pipeline).lock().get();
}

void RenderSequence::delete_pipeline(PipelineID pipeline_id) {
    if(!PipelineManager::contains(pipeline_id)) {
        return;
    }

    auto pip = pipeline(pipeline_id);
    if(pip->is_active()) {
        pip->deactivate();
    }

    ordered_pipelines_.remove_if([=](Pipeline::ptr pipeline) -> bool { return pipeline->id() == pipeline_id;});
    PipelineManager::destroy(pipeline_id);
}

void RenderSequence::delete_all_pipelines() {
    PipelineManager::destroy_all();

    for(auto pip: ordered_pipelines_) {
        if(pip->is_active()) {
            pip->deactivate();
        }
    }

    ordered_pipelines_.clear();
}

void RenderSequence::sort_pipelines(bool acquire_lock) {
    auto do_sort = [&]() {
        ordered_pipelines_.sort(
            [](Pipeline::ptr lhs, Pipeline::ptr rhs) { return lhs->priority() < rhs->priority(); }
        );
    };

    if(acquire_lock) {
        std::lock_guard<std::mutex> lock(pipeline_lock_);
        do_sort();
    } else {
        do_sort();
    }
}

PipelinePtr RenderSequence::new_pipeline(StageID stage, CameraID camera, const Viewport& viewport, TextureID target, int32_t priority) {
    PipelineID new_p = PipelineManager::make(this);

    auto pipeline = PipelineManager::get(new_p).lock();

    pipeline->set_stage(stage);
    pipeline->set_camera(camera);
    pipeline->set_viewport(viewport);
    pipeline->set_target(target);
    pipeline->set_priority(priority);
    pipeline->activate();

    std::lock_guard<std::mutex> lock(pipeline_lock_);
    ordered_pipelines_.push_back(pipeline);
    sort_pipelines();

    return pipeline.get();
}

void RenderSequence::set_renderer(Renderer* renderer) {
    renderer_ = renderer;
}

void RenderSequence::run() {
    targets_rendered_this_frame_.clear();

    int actors_rendered = 0;
    for(Pipeline::ptr pipeline: ordered_pipelines_) {
        run_pipeline(pipeline, actors_rendered);
    }

    window->stats->set_subactors_rendered(actors_rendered);
}


uint64_t generate_frame_id() {
    static uint64_t frame_id = 0;
    return ++frame_id;
}

void RenderSequence::run_pipeline(Pipeline::ptr pipeline_stage, int &actors_rendered) {
    Profiler profiler(__func__);

    uint64_t frame_id = generate_frame_id();

    if(!pipeline_stage->is_active()) {
        return;
    }

    RenderTarget& target = *window_; //FIXME: Should be window or texture

    /*
     *  Render targets can specify whether their buffer should be cleared at the start of each frame. We do this the first
     *  time we hit a render target when processing the pipelines. We keep track of the targets that have been rendered each frame
     *  and this list is cleared at the start of run().
     */
    if(targets_rendered_this_frame_.find(&target) == targets_rendered_this_frame_.end()) {
        if(target.clear_every_frame_flags()) {
            Viewport view(smlt::VIEWPORT_TYPE_FULL, target.clear_every_frame_colour());
            view.clear(target, target.clear_every_frame_flags());
        }

        targets_rendered_this_frame_.insert(&target);
    }

    auto& viewport = pipeline_stage->viewport;

    uint32_t clear = pipeline_stage->clear_flags();
    if(clear) {
        viewport->clear(target, clear); //Implicitly calls apply
    } else {
        viewport->apply(target); //FIXME apply shouldn't exist, it ties Viewport to OpenGL...
    }

    signal_pipeline_started_(*pipeline_stage);

    CameraID camera_id = pipeline_stage->camera_id();
    StageID stage_id = pipeline_stage->stage_id();

    auto stage = window->stage(stage_id);
    auto camera = stage->camera(camera_id);

    profiler.checkpoint("prepare");

    // Trigger a signal to indicate the stage is about to be rendered
    stage->signal_stage_pre_render()(camera_id, viewport);

    profiler.checkpoint("pre_render");

    // Apply any outstanding writes to the partitioner
    stage->partitioner->_apply_writes();

    profiler.checkpoint("apply_writes");

    static std::vector<LightID> light_ids;
    static std::vector<StageNode*> nodes_visible;

    /* Empty out, but leave capacity to prevent constant allocations */
    light_ids.resize(0);
    nodes_visible.resize(0);

    // Gather the lights and geometry visible to the camera
    stage->partitioner->lights_and_geometry_visible_from(camera_id, light_ids, nodes_visible);

    // Get the actual lights from the IDs
    auto lights_visible = map<decltype(light_ids), std::vector<LightPtr>>(
        light_ids, [&](const LightID& light_id) -> LightPtr { return stage->light(light_id); }
    );

    profiler.checkpoint("gather");

    uint32_t renderables_rendered = 0;
    // Mark the visible objects as visible
    for(auto& node: nodes_visible) {
        if(!node->is_visible()) {
            continue;
        }

        auto renderable_lights = filter(lights_visible, [=](const LightPtr& light) -> bool {
            // Filter by whether or not the renderable bounds intersects the light bounds
            if(light->type() == LIGHT_TYPE_DIRECTIONAL) {
                return true;
            } else if(light->type() == LIGHT_TYPE_SPOT_LIGHT) {
                return node->transformed_aabb().intersects_aabb(light->transformed_aabb());
            } else {
                return node->transformed_aabb().intersects_sphere(light->absolute_position(), light->range() * 2);
            }
        });

        std::partial_sort(
            renderable_lights.begin(),
            renderable_lights.begin() + std::min(MAX_LIGHTS_PER_RENDERABLE, (uint32_t) renderable_lights.size()),
            renderable_lights.end(),
            [=](LightPtr lhs, LightPtr rhs) {
                /* FIXME: Sorting by the centre point is problematic. A renderable is made up
                 * of many polygons, by choosing the light closest to the center you may find that
                 * that polygons far away from the center aren't affected by lights when they should be.
                 * This needs more thought, probably. */
                if(lhs->type() == LIGHT_TYPE_DIRECTIONAL && rhs->type() != LIGHT_TYPE_DIRECTIONAL) {
                    return true;
                } else if(rhs->type() == LIGHT_TYPE_DIRECTIONAL && lhs->type() != LIGHT_TYPE_DIRECTIONAL) {
                    return false;
                }

                float lhs_dist = (node->centre() - lhs->position()).length_squared();
                float rhs_dist = (node->centre() - rhs->position()).length_squared();
                return lhs_dist < rhs_dist;
            }
        );

        for(auto& renderable: node->_get_renderables(camera->frustum())) {
            if(!renderable->index_element_count()) {
                // Don't render things with no indices
                continue;
            }

            renderable->update_last_visible_frame_id(frame_id);
            renderable->set_affected_by_lights(renderable_lights);
            ++renderables_rendered;
        }
    }

    profiler.checkpoint("lights");

    window->stats->set_geometry_visible(renderables_rendered);

    using namespace std::placeholders;

    auto visitor = renderer_->get_render_queue_visitor(camera);

    // Render the visible objects
    stage->render_queue->traverse(visitor.get(), frame_id);

    profiler.checkpoint("traversal");

    // Trigger a signal to indicate the stage has been rendered
    stage->signal_stage_post_render()(camera_id, viewport);

    signal_pipeline_finished_(*pipeline_stage);
    profiler.checkpoint("post_render");
}

}
