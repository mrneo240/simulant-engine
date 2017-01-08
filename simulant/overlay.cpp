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

#include "overlay.h"
#include "window_base.h"
#include "loader.h"
#include "ui/ui_private.h"
#include "resource_manager.h"

namespace smlt {

Overlay::Overlay(OverlayID id, WindowBase *parent):
    generic::Identifiable<OverlayID>(id),
    Resource(parent->shared_assets.get()),
    window_(parent) {

    resource_manager_ = ResourceManager::create(parent, parent->shared_assets.get());
    interface_ = ui::Interface::create(*parent, this);

    update_conn_ = window_->signal_step().connect(
        std::bind(&Overlay::update, this, std::placeholders::_1)
    );
}

Overlay::~Overlay() {
    try {
        update_conn_.disconnect();
    } catch(...) {}
}


void Overlay::add_css(const std::string& property, const std::string& value) {
    interface_->add_css(property, value);
}

ui::ElementList Overlay::append_row() {
    return interface_->append_row();
}

ui::ElementList Overlay::find(const std::string &selector) {
    return interface_->find(selector);
}

void Overlay::set_styles(const std::string& styles) {
    return interface_->set_styles(styles);
}

void Overlay::load_rml(const unicode& path) {
    window_->loader_for("rml_loader", path)->into(interface_);
}

void Overlay::register_font_globally(const unicode& ttf_file) {
    this->interface_->load_font(ttf_file);
}

void Overlay::load_rml_from_string(const unicode& data) {

}

void Overlay::render(CameraPtr camera, Viewport viewport) {
    interface_->render(camera, viewport);
}

void Overlay::update(double dt) {
    interface_->update(dt);
}

void Overlay::__handle_mouse_move(int x, int y) {
    if(!is_being_rendered()) return;

}

void Overlay::__handle_mouse_down(int button) {
    if(!is_being_rendered()) return;

    mouse_buttons_down_.insert(button);
}

void Overlay::__handle_mouse_up(int button, bool check_rendered) {
    if(check_rendered && !is_being_rendered()) return;

    /* We have this check because we forcibly call mouse up when we stop rendering
     * and without it we might end up triggering a click twice!
     */
    if(mouse_buttons_down_.find(button) != mouse_buttons_down_.end()) {
        //FIXME: Again, pass down modifiers
        mouse_buttons_down_.erase(button);
    }
}

void Overlay::__handle_touch_up(int finger_id, int x, int y, bool check_rendered) {
    if(check_rendered && !is_being_rendered()) return;

    /* We have this check because we forcibly call touch up when we stop rendering
     * and without it we might end up triggering a click twice!
     */
    if(fingers_down_.find(finger_id) != fingers_down_.end()) {
        fingers_down_.erase(finger_id);
    }
}

void Overlay::__handle_touch_motion(int finger_id, int x, int y) {
    if(!is_being_rendered()) return;

    /*
     * We don't want to pass down motion events if they weren't preceded by a touch down. This can
     * happen if we stop rendering a Overlay, and we forcibly send touch up messages. This if statement
     * prevents actions being triggered if the Overlay starts rendering when the user has their finger on
     * the screen
     */
    if(fingers_down_.find(finger_id) == fingers_down_.end()) return;

}

void Overlay::__handle_touch_down(int finger_id, int x, int y) {
    if(!is_being_rendered()) return;

    fingers_down_.insert(finger_id);
}

void Overlay::on_render_stopped() {
    // When rendering stops, we need to release all of the touches and buttons (as if the document
    // had been destroyed)

    auto mtmp = mouse_buttons_down_;
    for(auto btn: mtmp) {
        __handle_mouse_up(btn, false);
    }

    auto ftmp = fingers_down_;
    for(auto finger: ftmp) {
        __handle_touch_up(finger, 0, 0, false); //FIXME: Perhaps we should maintain the position?
    }
}

}
