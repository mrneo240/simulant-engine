#include <kazbase/logging.h>
#include <kazbase/json/json.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <unistd.h>

#include "./debug_service.h"

#include "../application.h"
#include "../window_base.h"
#include "../screens/screen.h"
#include "../stage.h"
#include "../actor.h"
#include "../light.h"

namespace kglt {

DebugService::~DebugService() {
    stop();
}

void DebugService::start() {
    if(running_) {
        return;
    }

    if(!start_server()) {
        return;
    }

    handle_ = std::async(std::launch::async, std::bind(&DebugService::run, this));
}

void DebugService::stop() {
    running_ = false;
    if(handle_.valid()) {
        handle_.get();
    }
}

struct Always {
    Always(std::function<void ()> func):
        func_(func) {}

    ~Always() {
        func_();
    }
    std::function<void ()> func_;
};

void DebugService::cleanup_threads() {
    for(auto it = client_threads_.begin(); it != client_threads_.end();) {
        if(clients_[it->first] == -1) {
            it->second.get(); // Trigger any exceptions
            client_threads_.erase(it++);
        } else {
            ++it;
        }
    }
}

void DebugService::run() {
    auto ready = [](int socket) -> bool {
        fd_set rfds;
        struct timeval tv;

        FD_ZERO(&rfds);
        FD_SET(socket, &rfds);

        tv.tv_sec = 0;
        tv.tv_usec = 0;

        return select(socket + 1, &rfds, NULL, NULL, &tv);
    };

    running_ = true;

    struct sockaddr_in clientaddr;
    socklen_t addrlen;

    int slot = 0;

    // Signify there is no connection in each slot
    {
        std::lock_guard<std::mutex> lock(client_lock_);
        for(uint32_t i = 0; i < MAX_CLIENTS; ++i) {
            clients_[i] = -1;
        }
    }

    while(running_) {
        cleanup_threads();
        addrlen = sizeof(clientaddr);

        int new_socket = 0;
        if(ready(listen_fd_)) {
            std::lock_guard<std::mutex> lock(client_lock_);
            new_socket = clients_[slot] = accept(
                listen_fd_, (struct sockaddr*) &clientaddr, &addrlen
            );
            L_DEBUG("DebugService: Accepted client");
        } else {
            continue;
        }

        if(new_socket < 0) {
            L_WARN("DebugService: error with connection");
        } else {
            L_DEBUG("DebugService: Client connected");
            // Start a thread to respond to the client
            client_threads_[slot] = std::async(std::launch::async, [=]() {
                //(ab)use RAII to ensure no matter what happens we release the client
                Always __([=]() {
                    L_DEBUG("DebugService: Freeing client");

                    shutdown(new_socket, SHUT_RDWR);
                    close(new_socket);

                    std::lock_guard<std::mutex> lock(client_lock_);
                    this->clients_[slot] = -1;
                });

                respond(new_socket);
            });
        }
    }

    L_DEBUG("DebugService: Thread finished");
}

bool DebugService::start_server() {
    struct addrinfo hints = {0};
    struct addrinfo *res = nullptr;
    struct addrinfo *p = nullptr;

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if(getaddrinfo(nullptr, std::to_string(PORT).c_str(), &hints, &res) != 0) {
        L_ERROR("DebugService: Unable to start the debug service");
        return false;
    }

    for(p = res; p != nullptr; p = p->ai_next) {
        listen_fd_ = socket(p->ai_family, p->ai_socktype, IPPROTO_TCP);
        if(listen_fd_ == -1) continue;
        if(bind(listen_fd_, p->ai_addr, p->ai_addrlen) == 0) break;
    }

    if(!p) {
        L_ERROR("DebugService: Error binding to socket");
        return false;
    }

    freeaddrinfo(res);

    if(listen(listen_fd_, 10) != 0) {
        L_ERROR("DebugService: listen() error");
        return false;
    }

    return true;
}

std::string make_response(unicode content, int status=200) {
    unicode templ = "HTTP/1.1 {0} OK\r\nContent-Type: application/json\r\nContent-Length: {2}\r\n\r\n{1}";

    return templ.format(
        status,
        content,
        content.length()
    ).encode();
}


void DebugService::respond(int client_socket) {
    L_DEBUG("DebugService: Handling request");

    const int MAX_LENGTH = 1024 * 1024;
    char msg[MAX_LENGTH] = {0};

    int rcvd = recv(client_socket, msg, MAX_LENGTH, 0);
    if(rcvd < 0) {
        L_WARN("DebugService: Error receiving from client");
    } else if(rcvd == 0) {
        L_WARN("DebugService: Client disconnected");
    } else {
        // Handle message
        auto request = _u(msg);

        auto lines = request.split("\n");
        auto first_line = lines[0].strip().split(" ");
        if(first_line.size() != 3) {
            L_WARN(_u("Unable to deal with {0}").format(lines[0]).encode());
            return;
        }

        auto method = first_line[0];
        auto path = first_line[1];
        auto protocol = first_line[2];

        std::string response;
        if(path == "/screens/") {
            response = make_response(command_screens());
        } else if(path == "/stages/") {
            response = make_response(command_stages());
        } else if(path == "/pipelines/") {
            response = make_response("{ \"pipelines\": [] }");
        } else if(path == "/rendertree/") {
            response = make_response("{ \"rendertree\": [] }");
        } else {
            response = make_response("NOT FOUND", 404);
        }

        L_DEBUG(_u("DebugService: response was - {0}").format(response).encode());
        if(send(client_socket, response.c_str(), response.size(), 0) == 0) {
            L_WARN("DebugService: No bytes were sent while sending response to the client");
        }
    }
}

std::string DebugService::command_screens() {
    json::JSON result;

    auto& screens = result.insert_array("screens");
    for(auto& p: window_->application->routes()) {
        auto& screen = screens.append_dict();
        screen.insert_value("route").set(p.first);
        screen.insert_value("name").set(p.second->name());
    }

    return json::dumps(result).encode();
}

std::string DebugService::command_stages() {
    json::JSON result;

    auto& stages = result.insert_array("stages");
    auto stage_processor = [&](Stage* stage) {
        auto& entry = stages.append_dict();
        entry.insert_value("id").set((int32_t) stage->id().value());

        auto& actors = entry.insert_array("actors");
        auto actor_processor = [&](Actor* actor) {
            auto& actor_entry = actors.append_dict();
            actor_entry.insert_value("id").set((int32_t)actor->id().value());
            actor_entry.insert_value("name").set(actor->name());

            auto abs_pos = actor->absolute_position();
            auto actor_pos = _u("({0}, {1}, {2})").format(abs_pos.x, abs_pos.y, abs_pos.z);
            actor_entry.insert_value("position").set(actor_pos);

            auto abs_rot = actor->absolute_rotation();
            auto actor_rot = _u("({0}, {1}, {2}, {3})").format(abs_rot.x, abs_rot.y, abs_rot.z, abs_rot.w);
            actor_entry.insert_value("rotation").set(actor_rot);
        };

        auto& lights = entry.insert_array("lights");
        auto light_processor = [&](Light* light) {
            auto& light_entry = lights.append_dict();
            light_entry.insert_value("id").set((int32_t) light->id().value());
            auto abs_pos = light->absolute_position();
            auto light_pos = _u("({0}, {1}, {2})").format(abs_pos.x, abs_pos.y, abs_pos.z);
            light_entry.insert_value("position").set(light_pos);
        };

        stage->ActorManager::each(actor_processor);
        stage->LightManager::each(light_processor);
    };

    window_->StageManager::each(stage_processor);
    return json::dumps(result).encode();
}

}