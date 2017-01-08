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

#ifndef ELEMENT_H
#define ELEMENT_H

#include <string>
#include <memory>

#include "../colour.h"
#include "../idle_task_manager.h"
#include "../utils/unicode.h"
#include "../generic/data_carrier.h"

namespace smlt {
namespace ui {

class ElementImpl;

struct TouchEvent {
    int finger_id;
};

enum EventType {
    EVENT_TYPE_NONE,
    EVENT_TYPE_TOUCH_DOWN,
    EVENT_TYPE_TOUCH_UP,
    EVENT_TYPE_TOUCH_OVER,
    EVENT_TYPE_TOUCH_OUT
};

enum TextAlignment {
    TEXT_ALIGNMENT_LEFT,
    TEXT_ALIGNMENT_CENTRE,
    TEXT_ALIGNMENT_RIGHT
};

union Event {
    EventType type = EVENT_TYPE_NONE;
    TouchEvent touch;
};


typedef std::function<void (Event)> EventCallback;

class Element :
    public generic::DataCarrier {

public:
    Element(std::shared_ptr<ElementImpl> impl);

    std::string name() const;

    void set_text(const unicode& content);
    const unicode text() const;

    void add_class(const unicode& cl);
    void remove_class(const unicode &cl);

    std::string css(const std::string& property) const;
    void add_css(const std::string& property, const std::string& value);

    void set_attr(const std::string& property, const std::string& value);
    std::string attr(const std::string& property);

    void show(const std::string& transition="");
    void hide() {
        add_css("visibility", "hidden");
    }

    void set_id(const std::string& id);
    std::string id() const;

    void scroll_to_bottom();

    bool is_visible() const;

    Element append_row();
    Element append_label(const unicode& text);
    Element append_button(const unicode& text, EventCallback on_click=EventCallback());
    Element append_progress_bar();

    void set_event_callback(EventType event_type, EventCallback func);

    void remove_children();
    void inner_rml(const unicode& rml);

    bool is_dead() const;

    void set_background_colour(const smlt::Colour& colour);
    void set_border_colour(const smlt::Colour& colour);
    void set_text_colour(const smlt::Colour& colour);
    void set_border_width(const float width);
    void set_border_radius(const float radius);
    void set_text_alignment(TextAlignment alignment);
    void set_padding(float padding);

    Colour background_colour() const;
    Colour text_colour() const;
private:
    friend class Interface;

    std::shared_ptr<ElementImpl> impl_;
};

}
}

#endif // ELEMENT_H
