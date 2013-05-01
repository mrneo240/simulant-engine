#ifndef CONSOLE_H
#define CONSOLE_H

#include <tr1/memory>
#include "generic/managed.h"
#include "kazbase/unicode.h"
#include "interpreter.h"

namespace kglt {

class WindowBase;
class Interpreter;

class Console:
    public Managed<Console> {

public:
    Console(WindowBase& window);

    Interpreter& lua();

    void entry(kglt::KeyEvent code);

private:
    WindowBase& window_;
    Interpreter::ptr interpreter_;

    std::vector<unicode> history_;
    unicode current_command_;

    std::tr1::shared_ptr<Interpreter> lua_;

    bool active_;

    void update_output();
    LuaResult execute(const unicode& command, unicode& output);
};

}

#endif // CONSOLE_H