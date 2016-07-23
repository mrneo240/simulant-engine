#ifndef MANAGERS_H
#define MANAGERS_H

#include "generic/generic_tree.h"
#include "generic/manager.h"
#include "generic/property.h"
#include "types.h"
#include "interfaces.h"

namespace kglt {


class BackgroundManager:
    public generic::TemplatedManager<Background, BackgroundID>,
    public virtual Updateable {

public:
    BackgroundManager(WindowBase* window);
    ~BackgroundManager();

    BackgroundID new_background();
    BackgroundID new_background_from_file(const unicode& filename, float scroll_x=0.0, float scroll_y=0.0);
    BackgroundPtr background(BackgroundID bid);
    bool has_background(BackgroundID bid) const;
    void delete_background(BackgroundID bid);
    uint32_t background_count() const;

    void update(double dt) override;

    Property<BackgroundManager, WindowBase> window = { this, &BackgroundManager::window_ };
private:
    WindowBase* window_;
};

class CameraManager:
    public generic::TemplatedManager<Camera, CameraID> {

public:
    CameraManager(WindowBase* window);

    CameraID new_camera();
    CameraID new_camera_with_orthographic_projection(double left=0, double right=0, double bottom=0, double top=0, double near=-1.0, double far=1.0);
    CameraID new_camera_for_ui();
    CameraID new_camera_for_viewport(const Viewport& vp);
    CameraPtr camera(CameraID c);
    void delete_camera(CameraID cid);
    uint32_t camera_count() const;
    const bool has_camera(CameraID id) const;

private:
    WindowBase* window_;
};

typedef sig::signal<void (StageID)> StageAddedSignal;
typedef sig::signal<void (StageID)> StageRemovedSignal;

class StageManager:
    public BaseStageManager,
    public virtual Updateable {

    DEFINE_SIGNAL(StageAddedSignal, signal_stage_added);
    DEFINE_SIGNAL(StageRemovedSignal, signal_stage_removed);

public:
    StageManager(WindowBase* window);

    StageID new_stage(AvailablePartitioner partitioner=PARTITIONER_OCTREE);
    StagePtr stage(StageID s);
    void delete_stage(StageID s);
    uint32_t stage_count() const;
    bool has_stage(StageID stage_id) const;

    void print_tree();
    void update(double dt) override;

private:
    WindowBase* window_ = nullptr;
    void print_tree(GenericTreeNode* node, uint32_t& level);
};

class UIStageManager:
    public generic::TemplatedManager<UIStage, UIStageID> {

public:
    UIStageManager(WindowBase* window);

    UIStageID new_ui_stage();
    UIStageID new_ui_stage_from_file(const unicode& rml_file);

    UIStagePtr ui_stage(UIStageID s);
    void delete_ui_stage(UIStageID s);
    uint32_t ui_stage_count() const;

    bool has_ui_stage(UIStageID ui_stage) const;
private:
    WindowBase* window_;

};

}

#endif // MANAGERS_H
