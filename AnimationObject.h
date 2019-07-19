#ifndef CARTA_BACKEND__ANIMATIONOBJECT_H_
#define CARTA_BACKEND__ANIMATIONOBJECT_H_

#include <tbb/task.h>
#include <chrono>
#include <iostream>

#include <carta-protobuf/animation.pb.h>
#include <carta-protobuf/set_image_channels.pb.h>

namespace CARTA {
const int AnimationFlowWindowConstant = 5;
const int AnimationFlowWindowScaler = 2;
} // namespace CARTA

const int ANI_SMOOTHING_SIZE = 50;

class AnimationObject {
    friend class Session;

    int _file_id;
    CARTA::AnimationFrame _start_frame;
    CARTA::AnimationFrame _first_frame;
    CARTA::AnimationFrame _last_frame;
    CARTA::AnimationFrame _delta_frame;
    CARTA::AnimationFrame _current_frame;
    CARTA::AnimationFrame _next_frame;
    CARTA::AnimationFrame _stop_frame;
    CARTA::AnimationFrame _last_flow_frame;
    int _frame_rate;
    int _set_frame_rate;
    std::chrono::microseconds _frame_interval;
    std::chrono::time_point<std::chrono::high_resolution_clock> _t_start;
    std::chrono::time_point<std::chrono::high_resolution_clock> _t_last;
    bool _looping;
    bool _reverse_at_end;
    bool _going_forward;
    bool _always_wait;
    volatile bool _stop_called;
    int _wait_duration_ms;
    volatile int _file_open;
    volatile bool _waiting_flow_event;
    tbb::task_group_context _tbb_context;

    int _s_start;
    int _s_end;
    int _s_size;
    int _smooth_sum;
    int _last_timestamp;
    int _smoothing[ANI_SMOOTHING_SIZE];
public:
    AnimationObject(int file_id, CARTA::AnimationFrame& start_frame, CARTA::AnimationFrame& first_frame, CARTA::AnimationFrame& last_frame,
        CARTA::AnimationFrame& delta_frame, int frame_rate, bool looping, bool reverse_at_end, bool always_wait);
    int CurrentFlowWindowSize() {
        return (CARTA::AnimationFlowWindowConstant * CARTA::AnimationFlowWindowScaler * _frame_rate);
    }
    void CancelExecution() {
        _tbb_context.cancel_group_execution();
    }
    void UpdateFrameRate(int timestamp);
};

#endif // CARTA_BACKEND__ANIMATIONOBJECT_H_
