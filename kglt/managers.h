#ifndef MANAGERS_H
#define MANAGERS_H

#include "generic/manager.h"
#include "types.h"
#include "interfaces.h"

namespace kglt {


class BackgroundManager:
    public generic::TemplatedManager<WindowBase, Background, BackgroundID>,
    public virtual Updateable {

public:
    BackgroundManager(WindowBase* window);

    BackgroundID new_background();
    BackgroundID new_background_from_file(const unicode& filename, float scroll_x=0.0, float scroll_y=0.0);
    BackgroundPtr background(BackgroundID bid);
    bool has_background(BackgroundID bid) const;
    void delete_background(BackgroundID bid);
    uint32_t background_count() const;

    void update(double dt) override;

private:
    WindowBase* window_;
};


}

#endif // MANAGERS_H
