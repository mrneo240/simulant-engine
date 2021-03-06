#include <cmath>

#include "widget.h"
#include "ui_manager.h"
#include "../actor.h"
#include "../../stage.h"
#include "../../material.h"

namespace smlt {
namespace ui {

Widget::Widget(WidgetID id, UIManager *owner, UIConfig *defaults):
    ContainerNode(owner->stage()),
    generic::Identifiable<WidgetID>(id),
    owner_(owner) {

}

Widget::~Widget() {
    if(focus_next_ && focus_next_->focus_previous_ == this) {
        focus_next_->focus_previous_ = nullptr;
    }

    if(focus_previous_ && focus_previous_->focus_next_ == this) {
        focus_previous_->focus_next_ = nullptr;
    }
}

bool Widget::init() {
    actor_ = stage->new_actor();
    actor_->set_parent(this);

    material_ = stage->assets->new_material_from_file(Material::BuiltIns::TEXTURE_ONLY).fetch();
    material_->first_pass()->set_blending(BLEND_ALPHA);

    // Assign the default font as default
    set_font(stage->assets->default_font_id());

    mesh_ = construct_widget(0, 0);

    initialized_ = true;
    return true;
}

void Widget::cleanup() {
    // Make sure we fire any outstanding events when the widget
    // is destroyed. If any buttons are held, then they should fire
    // released signals.

    auto fingers_down = fingers_down_; // Copy, fingerup will delete from fingers_down_
    for(auto& finger_id: fingers_down) {
        fingerup(finger_id);
    }

    StageNode::cleanup();
}

void Widget::set_font(FontID font_id) {
    if(font_ && font_->id() == font_id) {
        return;
    }

    font_ = stage->assets->font(font_id);
    line_height_ = ::round(float(font_->size()) * 1.1);

    on_size_changed();
}

void Widget::resize(float width, float height) {
    if(width_ == width && height_ == height) {
        return;
    }

    width_ = width;
    height_ = height;
    on_size_changed();
}

void Widget::rebuild() {
    // If we aren't initialized, don't do anything yet
    if(!is_initialized()) return;

    mesh_ = construct_widget(width_, height_);
    actor_->set_mesh(mesh_->id());
}

void Widget::set_border_width(float x) {
    if(border_width_ == x) {
        return;
    }

    border_width_ = x;
    rebuild();
}

void Widget::set_border_colour(const Colour &colour) {
    if(border_colour_ == colour) {
        return;
    }

    border_colour_ = colour;
    rebuild();
}

void Widget::set_width(float width) {
    if(width_ == width) {
        return;
    }

    width_ = width;
    on_size_changed();
}

void Widget::set_height(float height) {
    if(height_ == height) {
        return;
    }

    height_ = height;
    on_size_changed();
}

void Widget::set_text(const unicode &text) {
    if(text_ == text) {
        return;
    }

    text_ = text;
    on_size_changed();
}

void Widget::on_size_changed() {
    rebuild();
}

void Widget::set_property(const std::string &name, float value) {
    properties_[name] = value;
}

void Widget::ask_owner_for_destruction() {
    owner_->delete_widget(id());
}

const AABB &Widget::aabb() const {
    return actor_->aabb();
}

void Widget::set_background_image(TextureID texture) {
    if(background_image_ == texture) {
        return;
    }

    background_image_ = texture;

    // Triggers a rebuild
    set_background_image_source_rect(
        Vec2(),
        texture.fetch()->dimensions()
    );
}

void Widget::set_background_image_source_rect(const Vec2& bottom_left, const Vec2& size) {
    if(background_image_rect_.bottom_left == bottom_left && background_image_rect_.size == size) {
        // Nothing to do
        return;
    }

    background_image_rect_.bottom_left = bottom_left;
    background_image_rect_.size = size;
    rebuild();
}

void Widget::set_foreground_image(TextureID texture) {
    if(foreground_image_ == texture) {
        return;
    }

    foreground_image_ = texture;

    // Triggers a rebuild
    set_foreground_image_source_rect(
        Vec2(),
        texture.fetch()->dimensions()
    );
}

void Widget::set_foreground_image_source_rect(const Vec2& bottom_left, const Vec2& size) {
    if(foreground_image_rect_.bottom_left == bottom_left && foreground_image_rect_.size == size) {
        // Nothing to do
        return;
    }

    foreground_image_rect_.bottom_left = bottom_left;
    foreground_image_rect_.size = size;
    rebuild();
}

void Widget::set_background_colour(const Colour& colour) {
    if(background_colour_ == colour) {
        // Nothing to do
        return;
    }

    background_colour_ = colour;
    rebuild();
}

void Widget::set_foreground_colour(const Colour& colour) {
    if(foreground_colour_ == colour) {
        // Nothing to do
        return;
    }

    foreground_colour_ = colour;
    rebuild();
}

void Widget::set_text_colour(const Colour &colour) {
    if(text_colour_ == colour) {
        // Nothing to do
        return;
    }

    text_colour_ = colour;
    rebuild();
}

void Widget::render_text(MeshPtr mesh, const std::string& submesh_name, const unicode& text, float width, float left_margin, float top_margin) {
    if(mesh->has_submesh(submesh_name)) {
        // Save these vertices as free
        auto sm = mesh->submesh(submesh_name);
        sm->index_data->each([&](uint32_t idx) {
            available_indexes_.insert(idx);
        });

        sm->index_data->clear();

        // Make sure we maintain the correct material
        sm->set_material_id(font_->material_id());
    } else {
        // Create a new submesh for the text
        auto sm = mesh->new_submesh(submesh_name, MESH_ARRANGEMENT_TRIANGLES);
        sm->set_material_id(font_->material_id());
    }

    auto submesh = mesh->submesh(submesh_name);

    // We return here so there's always a text mesh even if it's got nothing in it
    // FIXME: although totally not thread or exception safe :/
    if(text.empty()) {
        return;
    }

    struct WordVertex {
        Vec3 position;
        Vec2 texcoord;
    };

    std::vector<WordVertex> word_vertices;
    float word_length = .0f;
    float longest_line = .0f;
    float line_length = .0f;

    // We use the font-size as the default height, but then add line-height for each subsequent
    // line. That's because the line height might be less than the font size (causing overlapping)
    // but we want the total visible height... I think this makes sense!
    float min_height = std::numeric_limits<float>::max();
    float max_height = std::numeric_limits<float>::lowest();

    float xoffset = 0;
    float yoffset = line_height_ / 2.0f;


    for(std::size_t i = 0; i < text.length(); ++i) {
        bool word_ended = text[i] == U' ' || i == text.length() - 1;
        bool word_wrap = false;

        if(text[i] == '\n') {
            xoffset = 0;
            yoffset -= line_height_;

            if(line_length > longest_line) {
                longest_line = line_length;
            }
            line_length = .0f;
        } else {
            auto ch = text[i];
            auto ch_width = font_->character_width(ch);
            auto ch_height = font_->character_height(ch);
            auto coords = font_->texture_coordinates_for_character(ch);
            auto min = coords.first;
            auto max = coords.second;

            auto off = font_->character_offset(ch);

            WordVertex v1, v2, v3, v4;

            v1.position.x = off.first + xoffset;
            v1.position.y = off.second + yoffset;
            v1.texcoord.x = min.x;
            v1.texcoord.y = min.y;
            word_vertices.push_back(v1);

            v2.position.x = off.first + xoffset;
            v2.position.y = off.second + yoffset - ch_height;
            v2.texcoord.x = min.x;
            v2.texcoord.y = max.y;
            word_vertices.push_back(v2);

            v3.position.x = off.first + xoffset + ch_width;
            v3.position.y = off.second + yoffset - ch_height;
            v3.texcoord.x = max.x;
            v3.texcoord.y = max.y;
            word_vertices.push_back(v3);

            v4.position.x = off.first + xoffset + ch_width;
            v4.position.y = off.second + yoffset;
            v4.texcoord.x = max.x;
            v4.texcoord.y = min.y;
            word_vertices.push_back(v4);

            if(i != text.length() - 1) {
                float advance = font_->character_advance(ch, text[i + 1]);
                xoffset += advance;
                line_length += advance + ch_width;
            } else {
                line_length += ch_width;
            }

            // If the xoffset is greater than the width then we should word wrap
            if(xoffset > width) {
                word_wrap = true;
            }
        }

        if(word_ended) {
            float xshift = 0;
            float yshift = 0;

            // If we need to wrap the word, then do so but only if the word
            // would actually fit. We don't even try otherwise (but it would be nice to
            // hyphenate)
            if(word_wrap && word_length < width) {
                xshift = -xoffset;
                yshift = line_height_;

                xoffset = word_length;
                yoffset -= line_height_;
            }

            std::vector<uint32_t> used_indexes;

            // We're at the end of the word, time to commit to the mesh
            for(auto& v: word_vertices) {
                auto pos = v.position + Vec3(xshift, yshift, text_depth_bias_);
                if(pos.y < min_height) min_height = pos.y;
                if(pos.y > max_height) max_height = pos.y;

                if(!available_indexes_.empty()) {
                    auto it = available_indexes_.begin();
                    auto idx = *it;
                    available_indexes_.erase(it);

                    // Use an existing slot
                    submesh->vertex_data->move_to(idx);
                    used_indexes.push_back(idx);
                } else {
                    // Move to the end to create a new vertex
                    used_indexes.push_back(submesh->vertex_data->count());
                    submesh->vertex_data->move_to_end();
                }

                submesh->vertex_data->position(pos);
                submesh->vertex_data->tex_coord0(v.texcoord);
                submesh->vertex_data->diffuse(text_colour_);
            }

            assert(used_indexes.size() == word_vertices.size());

            for(std::size_t k = 0; k < word_vertices.size(); k += 4) {
                submesh->index_data->index(used_indexes[k]);
                submesh->index_data->index(used_indexes[k + 1]);
                submesh->index_data->index(used_indexes[k + 2]);

                submesh->index_data->index(used_indexes[k]);
                submesh->index_data->index(used_indexes[k + 2]);
                submesh->index_data->index(used_indexes[k + 3]);
            }

            word_vertices.clear();
            used_indexes.clear();
            word_length = .0f;
            word_wrap = false;
            if(line_length > longest_line) {
                longest_line = line_length;
            }
            line_length = .0f;

        }
    }

    // Everything was positioned with the starting X being at the
    // the center of the widget, so now we move the text to the left by whatever its width was
    // or half of "width" if that was specified
    float shiftX = ::round(std::max(submesh->aabb().width(), width) / 2.0);
    float descent = font_->descent();

    // FIXME: I can't for the life of me figure out why this works - it's almost definitely wrong
    float shiftY = ::round(-descent);

    for(uint32_t i = 0; i < submesh->vertex_data->count(); ++i) {
        submesh->vertex_data->move_to(i);
        auto pos = submesh->vertex_data->position_at<Vec3>(i);
        submesh->vertex_data->position(
            pos + Vec3(-shiftX, -shiftY, 0)
        );
    }

    mesh->vertex_data->done();
    submesh->index_data->done();
}

MeshPtr Widget::construct_widget(float requested_width, float requested_height) {
    VertexSpecification spec;
    spec.position_attribute = VERTEX_ATTRIBUTE_3F;
    spec.texcoord0_attribute = VERTEX_ATTRIBUTE_2F;
    spec.diffuse_attribute = VERTEX_ATTRIBUTE_4F;

    float width = requested_width;
    float height = requested_height;

    auto mesh = (mesh_) ? mesh_ : stage->assets->new_mesh(spec).fetch();

    /* New mesh, so make sure we clear the available vertices */
    available_indexes_.clear();

    // Render the text to the specified width
    render_text(mesh, "text", text_, width - padding_.left - padding_.right, padding_.left, padding_.top);

    if(resize_mode_ == RESIZE_MODE_FIXED_WIDTH) {
        height = std::max(
            mesh->submesh("text")->aabb().height() + padding_.top + padding_.bottom,
            height
        );
    } else if(resize_mode_ == RESIZE_MODE_FIXED_HEIGHT) {
        width = std::max(
            mesh->submesh("text")->aabb().width() + padding_.left + padding_.right,
            width
        );

        // Default to the font size + padding if there is no requested height
        height = (height == 0.0f) ? font_->size() + padding_.top + padding_.bottom : height;

    } else if(resize_mode_ == RESIZE_MODE_FIT_CONTENT) {
        auto aabb = mesh->submesh("text")->aabb();
        width = aabb.width() + padding_.left + padding_.right;
        height = aabb.height() + padding_.top + padding_.bottom;
    } else {
        // Clip the content?
    }

    resize_or_generate_border(mesh, width + (border_width_ * 2), height + (border_width_ * 2), 0, 0);
    resize_or_generate_background(mesh, width, height, 0, 0);
    resize_or_generate_foreground(mesh, width, height, 0, 0);

    content_width_ = width;
    content_height_ = height;

    return mesh;
}

void Widget::set_resize_mode(ResizeMode resize_mode) {
    resize_mode_ = resize_mode;
    on_size_changed();
}

void Widget::set_padding(float left, float right, float bottom, float top) {
    padding_.left = left;
    padding_.right = right;
    padding_.bottom = bottom;
    padding_.top = top;
}

void generate_or_resize_rectangle(MeshPtr mesh, MaterialID material_id, const std::string& submesh_name, float width, float height, float xoffset, float yoffset, float zoffset) {
    if(!mesh->has_submesh(submesh_name)) {
        mesh->new_submesh_as_rectangle(
            submesh_name, material_id, width, height,
            Vec3(xoffset, yoffset, zoffset)
        );
    } else {
        auto submesh = mesh->submesh(submesh_name);
        mesh->vertex_data->move_to(submesh->index_data->at(0));
        mesh->vertex_data->position(xoffset + (-width / 2.0), yoffset + (-height / 2.0), zoffset);

        mesh->vertex_data->move_to(submesh->index_data->at(1));
        mesh->vertex_data->position(xoffset + (width / 2.0), yoffset + (-height / 2.0), zoffset);

        mesh->vertex_data->move_to(submesh->index_data->at(2));
        mesh->vertex_data->position(xoffset + (width / 2.0),  yoffset + (height / 2.0), zoffset);

        mesh->vertex_data->move_to(submesh->index_data->at(5));
        mesh->vertex_data->position(xoffset + (-width / 2.0),  yoffset + (height / 2.0), zoffset);
        mesh->vertex_data->done();
    }
}

void Widget::resize_or_generate_border(MeshPtr mesh, float width, float height, float xoffset, float yoffset) {
    generate_or_resize_rectangle(mesh, material_->id(), "border", width, height, xoffset, yoffset, 0);
    mesh->submesh("border")->set_diffuse(border_colour_);
}

void Widget::resize_or_generate_background(MeshPtr mesh, float width, float height, float xoffset, float yoffset) {
    generate_or_resize_rectangle(mesh, material_->id(), "background", width, height, xoffset, yoffset, background_depth_bias_);
    mesh->submesh("background")->set_diffuse(background_colour_);

    if(has_background_image()) {
        auto submesh = mesh->submesh("background");
        submesh->set_texture_on_material(0, background_image_);

        auto& vertices = mesh->vertex_data;
        auto& indices = submesh->index_data;
        auto dim = background_image_.fetch()->dimensions();

        Vec2 min = Vec2(
            background_image_rect_.bottom_left.x / dim.x,
            background_image_rect_.bottom_left.y / dim.y
        );

        Vec2 max = Vec2(
            (background_image_rect_.bottom_left.x + background_image_rect_.size.x) / dim.x,
            (background_image_rect_.bottom_left.y + background_image_rect_.size.y) / dim.y
        );

        auto first_idx = indices->at(0);

        vertices->move_to(first_idx);
        vertices->tex_coord0(min.x, min.y);
        vertices->move_to(first_idx + 1);
        vertices->tex_coord0(max.x, min.y);
        vertices->move_to(first_idx + 2);
        vertices->tex_coord0(max.x, max.y);
        vertices->move_to(first_idx + 3);
        vertices->tex_coord0(min.x, max.y);
        vertices->done();
    }
}

void Widget::resize_or_generate_foreground(MeshPtr mesh, float width, float height, float xoffset, float yoffset) {
    generate_or_resize_rectangle(mesh, material_->id(), "foreground", width, height, xoffset, yoffset, foreground_depth_bias_);
    mesh->submesh("foreground")->set_diffuse(foreground_colour_);

    if(has_foreground_image()) {
        auto submesh = mesh->submesh("foreground");
        submesh->set_texture_on_material(0, foreground_image_);

        auto& vertices = mesh->vertex_data;
        auto& indices = submesh->index_data;
        auto dim = foreground_image_.fetch()->dimensions();

        Vec2 min = Vec2(
            foreground_image_rect_.bottom_left.x / dim.x,
            foreground_image_rect_.bottom_left.y / dim.y
        );

        Vec2 max = Vec2(
            (foreground_image_rect_.bottom_left.x + foreground_image_rect_.size.x) / dim.x,
            (foreground_image_rect_.bottom_left.y + foreground_image_rect_.size.y) / dim.y
        );

        auto first_idx = indices->at(0);

        vertices->move_to(first_idx);
        vertices->tex_coord0(min.x, min.y);
        vertices->move_to(first_idx + 1);
        vertices->tex_coord0(max.x, min.y);
        vertices->move_to(first_idx + 2);
        vertices->tex_coord0(max.x, max.y);
        vertices->move_to(first_idx + 3);
        vertices->tex_coord0(min.x, max.y);
        vertices->done();
    }
}

bool Widget::is_pressed_by_finger(uint32_t finger_id) {
    return fingers_down_.find(finger_id) != fingers_down_.end();
}

void Widget::fingerdown(uint32_t finger_id) {
    // If we added, and it was the first finger down
    if(fingers_down_.insert(finger_id).second && fingers_down_.size() == 1) {
        // Emit the widget pressed signal
        signal_pressed_();
    }
}

void Widget::fingerup(uint32_t finger_id) {
    // If we just released the last finger, emit a widget released signal
    if(fingers_down_.erase(finger_id) && fingers_down_.empty()) {
        signal_released_();
        signal_clicked_();
    }
}

void Widget::fingerenter(uint32_t finger_id) {
    fingerdown(finger_id); // Same behaviour
}

void Widget::fingermove(uint32_t finger_id) {
    //FIXME: fire signal
}

void Widget::fingerleave(uint32_t finger_id) {
    // Same as fingerup, but we don't fire the click signal
    if(fingers_down_.erase(finger_id) && fingers_down_.empty()) {
        signal_released_();
    }
}

bool Widget::is_focused() const {
    return is_focused_;
}

void Widget::set_focus_previous(WidgetPtr previous_widget) {
    focus_previous_ = previous_widget;
    if(focus_previous_) {
        focus_previous_->focus_next_ = this;
    }
}

void Widget::set_focus_next(WidgetPtr next_widget) {
    focus_next_ = next_widget;
    if(focus_next_) {
        focus_next_->focus_previous_ = this;
    }
}

void Widget::focus() {
    auto focused = focused_in_chain_or_this();
    if(focused == this) {
        if(!is_focused_) {
            is_focused_ = true;
            signal_focused_();
        }
        return;
    } else {
        focused->blur();
        is_focused_ = true;
        signal_focused_();
    }
}

WidgetPtr Widget::focused_in_chain() {
    auto ret = focused_in_chain_or_this();

    // If we got back this element, but this element isn't focused
    // then return null
    if(ret == this && !is_focused_) {
        return nullptr;
    }

    return ret;
}

WidgetPtr Widget::focused_in_chain_or_this() {
    auto next = focus_next_;
    while(next && next != this) {
        if(next->is_focused()) {
            return next;
        }
        next = next->focus_next_;
    }

    auto previous = focus_previous_;
    while(previous && previous != this) {
        if(previous->is_focused()) {
            return previous;
        }
        previous = previous->focus_previous_;
    }

    return this;
}

void Widget::focus_next_in_chain(ChangeFocusBehaviour behaviour) {
    bool focus_none_if_none = (behaviour & FOCUS_NONE_IF_NONE_FOCUSED) == FOCUS_NONE_IF_NONE_FOCUSED;
    bool focus_this_if_none = (behaviour & FOCUS_THIS_IF_NONE_FOCUSED) == FOCUS_THIS_IF_NONE_FOCUSED;

    if(focus_none_if_none && focus_this_if_none) {
        throw std::logic_error(
            "You can only specify one 'none focused' behaviour"
        );
    }

    auto focused = focused_in_chain_or_this();
    auto to_focus = this;

    // If something is focused
    if(focused) {
        // Focus the next in the chain, otherwise focus the first in the chain
        if(focused->focus_next_) {
            to_focus = focused->focus_next_;
        } else {
            to_focus = first_in_focus_chain();
        }
    } else {
        // If nothing is focused, focus this if that's the desired behaviour
        // otherwise we don't focus anything
        if(focus_this_if_none) {
            to_focus = this;
        }
    }

    // Bail if we're already focusing the right thing
    if(focused && focused == to_focus) {
        return;
    }

    if(focused) {
        focused->blur();
    }

    if(to_focus) {
        to_focus->is_focused_ = true;
        to_focus->signal_focused_();
    }
}

void Widget::focus_previous_in_chain(ChangeFocusBehaviour behaviour) {
    bool focus_none_if_none = (behaviour & FOCUS_NONE_IF_NONE_FOCUSED) == FOCUS_NONE_IF_NONE_FOCUSED;
    bool focus_this_if_none = (behaviour & FOCUS_THIS_IF_NONE_FOCUSED) == FOCUS_THIS_IF_NONE_FOCUSED;

    if(focus_none_if_none && focus_this_if_none) {
        throw std::logic_error(
            "You can only specify one 'none focused' behaviour"
        );
    }

    auto focused = focused_in_chain_or_this();
    auto to_focus = this;

    // If something is focused
    if(focused) {
        // Focus the previous in the chain, otherwise focus the last in the chain
        if(focused->focus_previous_) {
            to_focus = focused->focus_previous_;
        } else {
            to_focus = last_in_focus_chain();
        }
    } else {
        // If nothing is focused, focus this if that's the desired behaviour
        // otherwise we don't focus anything
        if(focus_this_if_none) {
            to_focus = this;
        }
    }

    // Bail if we're already focusing the right thing
    if(focused && focused == to_focus) {
        return;
    }

    if(focused) {
        focused->blur();
    }

    if(to_focus) {
        to_focus->is_focused_ = true;
        to_focus->signal_focused_();
    }
}

WidgetPtr Widget::first_in_focus_chain() {
    auto search = this;
    while(search->focus_previous_) {
        search = search->focus_previous_;
    }

    return search;
}

WidgetPtr Widget::last_in_focus_chain() {
    auto search = this;
    while(search->focus_next_) {
        search = search->focus_next_;
    }

    return search;
}

void Widget::blur() {
    is_focused_ = false;
    signal_blurred_();
}

void Widget::click() {
    signal_clicked_();
}


}
}
