#include "stage.h"
#include "sound.h"

namespace kglt {

static ALCdevice* dev = nullptr;
static ALCcontext* ctx = nullptr;

void Sound::init_openal() {
    if(!dev) {
        dev = alcOpenDevice(NULL);
        ctx = alcCreateContext(dev, NULL);
        alcMakeContextCurrent(ctx);
    }
}

void Sound::shutdown_openal() {
    alcDestroyContext(ctx);
    alcCloseDevice(dev);
}

Sound::Sound(ResourceManager *resource_manager, SoundID id):
    generic::Identifiable<SoundID>(id),
    Resource(resource_manager) {


}

Source::Source(Stage *stage):
    stage_(stage),
    al_source_(0),
    buffers_{0, 0} {


}

Source::~Source() {
    alDeleteSources(1, &al_source_);
    alDeleteBuffers(2, buffers_);
}

void Source::attach_sound(SoundID sound) {
    if(!can_attach_sound_by_id()) {
        throw LogicError("Attaching a sound by ID is not supported here");
    }

    assert(stage_);
    sound_ = sound;
}

void Source::play_sound(bool loop) {
    if(!sound_) {
        throw LogicError("No sound attached");
    }

    auto sound = stage_->sound(sound_);
    sound->init_source_(*this);

    if(!al_source_) {
        alGenSources(1, &al_source_);
    }

    if(!buffers_[0]) {
        alGenBuffers(2, buffers_);
    }

    //Fill up two buffers to begin with
    auto bs1 = stream_func_(buffers_[0]);
    auto bs2 = stream_func_(buffers_[1]);

    assert(alGetError() == AL_NO_ERROR);

    int to_queue = (bs1 && bs2) ? 2 : (bs1 || bs2)? 1 : 0;

    alSourceQueueBuffers(al_source_, to_queue, buffers_);
    assert(alGetError() == AL_NO_ERROR);

    alSourcePlay(al_source_);
    assert(alGetError() == AL_NO_ERROR);

    playing_ = true;
    loop_stream_ = loop;
}

void Source::update_source(float dt) {
    if(!playing_) {
        return;
    }

    ALint processed = 0;

    assert(alGetError() == AL_NO_ERROR);

    alGetSourcei(al_source_, AL_BUFFERS_PROCESSED, &processed);

    assert(alGetError() == AL_NO_ERROR);

    while(processed--) {
        ALuint buffer = 0;
        alSourceUnqueueBuffers(al_source_, 1, &buffer);

        uint32_t bytes = stream_func_(buffer);

        if(!bytes) {
            signal_stream_finished_();

            if(loop_stream_) {
                play_sound(loop_stream_);
            } else {
                //Reset everything
                stream_func_ = std::function<int32_t (ALuint)>();
                playing_ = false;
            }
        } else {
            alSourceQueueBuffers(al_source_, 1, &buffer);
        }
    }
}

bool Source::is_playing_sound() const {
    return playing_;
}

}
