#ifndef RENDERER_H
#define RENDERER_H

#include <set>
#include <vector>
#include <memory>

#include "../generic/protected_ptr.h"
#include "../types.h"
#include "../generic/auto_weakptr.h"
#include "../window_base.h"

#include "batching/renderable.h"
#include "batching/render_queue.h"

namespace kglt {

class SubActor;

class Renderer:
    public batcher::RenderGroupFactory {

public:
    typedef std::shared_ptr<Renderer> ptr;

    Renderer(WindowBase* window):
        window_(window) {}

    void set_current_stage(StageID stage) {
        current_stage_ = stage;
    }

    virtual void render(CameraPtr camera, StagePtr stage, bool, const batcher::RenderGroup*, Renderable*, MaterialPass*, Light*, batcher::Iteration) = 0;

    Property<Renderer, WindowBase> window = { this, &Renderer::window_ };

    virtual void init_context() = 0;

    // virtual void upload_texture(Texture* texture) = 0;
protected:
    StagePtr current_stage();

private:    
    WindowBase* window_ = nullptr;
    StageID current_stage_;

};

}

#endif // RENDERER_H
