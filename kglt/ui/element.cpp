#include <Rocket/Core/Element.h>
#include <Rocket/Core/ElementText.h>
#include <Rocket/Core/ElementDocument.h>

#include "interface.h"
#include "ui_private.h"

namespace kglt {
namespace ui {


Element::Element(std::tr1::shared_ptr<ElementImpl> impl):
    impl_(impl) {

}

void Element::text(const unicode& text) {
    impl_->set_text(text);
}

const unicode Element::text() const {
    return impl_->text();
}

void Element::add_class(const std::string& cl) {
    impl_->add_class(cl);
}

}

}