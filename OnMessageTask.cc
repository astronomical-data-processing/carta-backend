#include "OnMessageTask.h"

#include <algorithm>
#include <cstring>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include "EventHeader.h"
#include "Util.h"

tbb::task* SetImageChannelsTask::execute() {
    std::pair<CARTA::SetImageChannels, uint32_t> request_pair;
    bool tester;

    _session->ImageChannelLock();
    tester = _session->_set_channel_queue.try_pop(request_pair);
    _session->ImageChannelTaskSetIdle();
    _session->ImageChannelUnlock();

    if (tester) {
        _session->ExecuteSetChannelEvt(request_pair);
    }

    return nullptr;
}

tbb::task* SetImageViewTask::execute() {
    _session->_file_settings.ExecuteOne(CARTA::EventType::SET_IMAGE_VIEW, _file_id);
    return nullptr;
}

tbb::task* SetCursorTask::execute() {
    _session->_file_settings.ExecuteOne(CARTA::EventType::SET_CURSOR, _file_id);
    return nullptr;
}

tbb::task* SetHistogramRequirementsTask::execute() {
    CARTA::SetHistogramRequirements message;
    if (message.ParseFromArray(_event_buffer, _event_length)) {
        _session->OnSetHistogramRequirements(message, _header.request_id);
    }

    return nullptr;
}

tbb::task* AnimationTask::execute() {
    if (_session->ExecuteAnimationFrame()) {
        if (_session->CalculateAnimationFlowWindow() > _session->CurrentFlowWindowSize()) {
            _session->SetWaitingTask(true);
        } else {
            increment_ref_count();
            recycle_as_safe_continuation();
        }
    } else {
        if (!_session->WaitingFlowEvent()) {
            _session->CancelAnimation();
        }
    }

    return nullptr;
}

tbb::task* OnAddRequiredTilesTask::execute() {
    _session->OnAddRequiredTiles(_message);
    return nullptr;
}
