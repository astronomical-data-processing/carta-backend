#include "AnimationObject.h"
#include <cmath>

AnimationObject::AnimationObject(int file_id, CARTA::AnimationFrame& start_frame, CARTA::AnimationFrame& first_frame, CARTA::AnimationFrame& last_frame, CARTA::AnimationFrame& delta_frame, int frame_rate, bool looping, bool reverse_at_end, bool always_wait) : _file_id(file_id),
          _start_frame(start_frame),
          _first_frame(first_frame),
          _last_frame(last_frame),
          _delta_frame(delta_frame),
          _looping(looping),
          _reverse_at_end(reverse_at_end),
          _set_frame_rate(frame_rate),
          _always_wait(always_wait) {
        _current_frame = start_frame;
        _next_frame = start_frame;
        _frame_rate = frame_rate/2; // start at half requested rate.
        _frame_interval = std::chrono::microseconds(int64_t(1.0e6 / frame_rate));
        _going_forward = true;
        _wait_duration_ms = 100;
        _stop_called = false;
        _file_open = true;
        _waiting_flow_event = false;
        _last_flow_frame = start_frame;
        _stop_frame = start_frame;

        _s_start = 0;
        _s_end = 0;
        _smooth_sum = 0;
    }


void AnimationObject::UpdateFrameRate(int timestamp)
{
    int front_end_rate;
    int diff;

    diff = timestamp - _last_timestamp;
    _smoothing[_s_end] = diff;
    _smooth_sum+= diff;

    _s_end = ((_s_end == (ANI_SMOOTHING_SIZE - 1))? 0 : _s_end + 1);
    
    if (_s_size != ANI_SMOOTHING_SIZE) {
        ++_s_size;
    } else {
        _smooth_sum-= _smoothing[_s_start];
        _s_start= ((_s_start == (ANI_SMOOTHING_SIZE - 1))? 0 : _s_start + 1); 
    }

    if (_s_size == 1) {
        return;
    }

    front_end_rate = (int)std::ceil(_smooth_sum/_s_size);

    if ( front_end_rate < _frame_rate ) {
        // Need to reduce the actual rate.
        _frame_rate = front_end_rate;
        _frame_interval = std::chrono::microseconds(int64_t(1.0e6 / _frame_rate));
    } else if ( _frame_rate < _set_frame_rate ) {
        // If front end is keeping up but the rate used is less than the 
        // requested rate, increase the rate of the backend.
        ++_frame_rate;
        _frame_interval = std::chrono::microseconds(int64_t(1.0e6 / _frame_rate));
    }
}