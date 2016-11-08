#ifndef ELEMENT_H
#define ELEMENT_H

#include <string>
#include <memory>

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

    float left() const;
    float top() const;
    float width() const;
    float height() const;

    void remove_children();
    void inner_rml(const unicode& rml);

    bool is_dead() const;
private:
    friend class Interface;

    std::shared_ptr<ElementImpl> impl_;
};

}
}

#endif // ELEMENT_H