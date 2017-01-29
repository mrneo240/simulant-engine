
/*
 * Widget Input Explanation
 *
 * Widget input is hard. Widgets can be in any stage, be transformed in any way and can be rendered with any camera to any viewport. That
 * makes processing input events tricky as the coordinates for touches/clicks are in window coordinates and we won't know how to transform them
 * until we process the render queue.
 *
 * So, we tackle this by making the UIManager an event listener. When we receive events we care about, we store them in a queue.
 *
 * The queue is processed each time the owning stage is rendered, but the queue is not cleared! This is because you can render the same stage
 * multiple times. The queue is cleared at the end of each frame.
 */

#include "ui_manager.h"
#include "button.h"
#include "label.h"
#include "progress_bar.h"
#include "../../stage.h"
#include "../../camera.h"

namespace smlt {
namespace ui {

using namespace std::placeholders;

UIManager::UIManager(Stage *stage):
    stage_(stage),
    window_(stage->window.get()){

    manager_.reset(new WidgetManager());

    window_->register_event_listener(this);

    /* Each time the stage is rendered with a camera and viewport, we need to process any queued events
     * so that (for example) we can interact with the same widget rendered to different viewports */
    stage_->signal_stage_pre_render().connect([this](CameraID cam_id, Viewport viewport) {
        this->process_event_queue(cam_id.fetch(), viewport);
    });

    /* We clear queued events at the end of each frame */
    window_->signal_frame_finished().connect([this]() {
        this->clear_event_queue();
    });
}

UIManager::~UIManager() {
    window_->unregister_event_listener(this);
}

WidgetID UIManager::new_widget_as_button(const unicode &text, float width, float height) {
    auto button = manager_->make_as<Button>(this, &config_).fetch();
    button->set_resize_mode(config_.button_resize_mode_);
    button->set_padding(
        config_.button_padding_.left,
        config_.button_padding_.right,
        config_.button_padding_.bottom,
        config_.button_padding_.top
    );
    button->set_text(text);
    button->set_background_colour(config_.button_background_color_);
    button->set_foreground_colour(config_.button_foreground_color_);
    button->resize(
        (width == .0f) ? config_.button_width_ : width,
        (height == .0f) ? config_.button_height_ : height
    );

    stage_->add_child(button);

    return button->id();
}

WidgetID UIManager::new_widget_as_label(const unicode &text, float width, float height) {
    auto label = manager_->make_as<Label>(this, &config_).fetch();
    label->set_text(text);
    label->resize(width, height);

    stage_->add_child(label);

    return label->id();
}

WidgetID UIManager::new_widget_as_progress_bar(float min, float max, float value) {
    auto pg = manager_->make_as<ProgressBar>(this, &config_).fetch();
    pg->set_property("min", min);
    pg->set_property("max", max);
    pg->set_property("value", value);

    stage_->add_child(pg);

    return pg->id();
}

void UIManager::delete_widget(WidgetID widget) {
    if(!widget) {
        return;
    }

    manager_->destroy(widget);
}

void UIManager::on_touch_begin(const TouchEvent &evt) {
    queue_event(evt);
}

void UIManager::on_touch_end(const TouchEvent &evt) {
    queue_event(evt);
}

void UIManager::queue_event(const TouchEvent& e) {
    UIEvent evt(e);
    queued_events_.push(evt);
}

void UIManager::process_event_queue(const Camera* camera, const Viewport &viewport) const {
    auto queued_events = queued_events_; // Copy the queue

    while(!queued_events.empty()) {
        auto evt = queued_events.front();
        queued_events.pop();

        switch(evt.type) {
            case UI_EVENT_TYPE_TOUCH: {
                auto widget = find_widget_at_window_coordinate(camera, viewport, Vec2(evt.touch.coord.x, evt.touch.coord.y));
                if(widget) {
                    if(evt.touch.type == TOUCH_EVENT_TYPE_FINGER_DOWN) {
                        widget->fingerdown(evt.touch.touch_id);
                    } else if(evt.touch.type == TOUCH_EVENT_TYPE_FINGER_UP){
                        widget->fingerup(evt.touch.touch_id);
                    }
                }
            } break;
        default:
            break;
        }
    }
}

void UIManager::clear_event_queue() {
    queued_events_ = std::queue<UIEvent>();
}

WidgetPtr UIManager::find_widget_at_window_coordinate(const Camera *camera, const Viewport &viewport, const Vec2 &window_coord) const {
    WidgetPtr result;

    manager_->each([&](uint32_t, WidgetPtr widget) {
        auto aabb = widget->transformed_aabb();
        std::vector<Vec3> ss_points;
        ss_points.push_back(camera->project_point(*window_, viewport, aabb.min).value());
        ss_points.push_back(camera->project_point(*window_, viewport, aabb.max).value());

        AABB ss_aabb(&ss_points[0], ss_points.size());

        // FIXME: Return the nearest if overlapping!
        if(ss_aabb.contains_point(Vec3(window_coord, 0.5))) {
            result = widget;
        }
    });

    return result;
}

}
}