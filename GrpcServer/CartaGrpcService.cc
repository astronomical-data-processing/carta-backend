//# CartaGrpcService.cc: grpc server to receive messages from the python scripting client

#include "CartaGrpcService.h"

#include <carta-protobuf/defs.pb.h>
#include <carta-protobuf/enums.pb.h>

#include "../Util.h"

CartaGrpcService::CartaGrpcService(bool verbose) : _verbose(verbose) {}

void CartaGrpcService::AddSession(Session* session) {
    // Map session to its ID, set connected to false
    auto session_id = session->GetId();
    std::pair<Session*, bool> session_info(session, false);
    _sessions[session_id] = session_info;
}

void CartaGrpcService::RemoveSession(Session* session) {
    // Remove Session from map
    auto session_id = session->GetId();
    if (_sessions.count(session_id)) {
        _sessions.erase(session_id);
    }
}

bool CartaGrpcService::CheckSessionId(uint32_t session_id, const std::string& command, std::string& message) {
    // Check if session ID exists and is connected
    bool id_ok(true);
    if (!_sessions.count(session_id)) {
        message = fmt::format("{} failed: Session {} does not exist", command, session_id);
        id_ok = false;
    } else if (!_sessions[session_id].second) {
        message = fmt::format("{} failed: Session {} is not connected", command, session_id);
        id_ok = false;
    }
    return id_ok;
}

bool CartaGrpcService::CheckFileId(uint32_t session_id, uint32_t file_id, const std::string& command, std::string& message) {
    // Check if session ID exists/connected and file ID exists
    bool id_ok(false);
    if (CheckSessionId(session_id, command, message)) {
        if (_sessions[session_id].first->HasFileId(file_id)) {
            id_ok = true;
        } else {
            message = fmt::format("{} failed: file id {} does not exist", command, file_id);
        }
    }

    return id_ok;
}

void CartaGrpcService::LogMessage(const std::string& message) {
    if (_verbose) {
        std::cout << "Scripting client request: " << message << std::endl;
    }
}

// ---------------------------------------------
// CARTAVIS proto support

grpc::Status CartaGrpcService::closeFile(grpc::ServerContext* context, const CARTAVIS::CloseFile* request, google::protobuf::Empty*) {
    auto session_id = request->session_id();
    auto file_id = request->file_id();
    LogMessage(fmt::format("closeFile for session {} file {}", session_id, file_id));

    grpc::Status status(grpc::Status::OK);
    std::string message;
    if (CheckFileId(session_id, file_id, "close", message)) {
        _sessions[session_id].first->ScriptClientCloseFile(file_id);
    } else {
        Log(session_id, message);
        status = grpc::Status(grpc::StatusCode::OUT_OF_RANGE, message);
    }

    return status;
}

grpc::Status CartaGrpcService::connectToSession(
    grpc::ServerContext* context, const CARTAVIS::ConnectToSession* request, CARTAVIS::ConnectToSessionAck* reply) {
    auto session_id = request->session_id();
    LogMessage(fmt::format("connectToSession {}", session_id));

    grpc::Status status(grpc::Status::OK);
    if (_sessions.count(session_id)) {
        auto session = _sessions[session_id];
        if (session.second == true) {
            std::string message = fmt::format("Connection failed: Session {} is already connected", session_id);
            Log(session_id, message);
            reply->set_success(false);
            reply->set_message(message);
            status = grpc::Status(grpc::StatusCode::ALREADY_EXISTS, message);
        } else {
            _sessions[session_id].second = true;
            reply->set_success(true);
        }
    } else {
        std::string message = fmt::format("Connection failed: Session {} does not exist", session_id);
        Log(session_id, message);
        status = grpc::Status(grpc::StatusCode::OUT_OF_RANGE, message);
    }

    return status;
}

grpc::Status CartaGrpcService::disconnectFromSession(
    grpc::ServerContext* context, const CARTAVIS::DisconnectFromSession* request, google::protobuf::Empty*) {
    auto session_id = request->session_id();
    LogMessage(fmt::format("disconnectFromSession {}", session_id));

    if (_sessions.count(session_id)) {
        _sessions[session_id].second = false;
    }

    return grpc::Status::OK;
}

grpc::Status CartaGrpcService::getRenderedImage(
    grpc::ServerContext* context, const CARTAVIS::GetRenderedImage* request, CARTAVIS::GetRenderedImageAck* reply) {
    auto session_id = request->session_id();
    auto file_id = request->file_id();
    LogMessage(fmt::format("getRenderedImage for session {} file {}", session_id, file_id));

    grpc::Status status(grpc::Status::OK);
    std::string message;
    if (CheckFileId(session_id, file_id, "getRenderedImage", message)) {
        _sessions[session_id].first->ScriptClientGetRenderedImage(file_id);
        // TODO: get session result here
        status = grpc::Status(grpc::StatusCode::UNIMPLEMENTED, message);
    } else {
        Log(session_id, message);
        status = grpc::Status(grpc::StatusCode::OUT_OF_RANGE, message);
    }

    return status;
}

grpc::Status CartaGrpcService::openFile(grpc::ServerContext* context, const CARTAVIS::OpenFile* request, CARTAVIS::OpenFileAck* reply) {
    auto session_id = request->session_id();
    auto file_id = request->file_id();
    auto directory = request->directory();
    auto file = request->file();
    LogMessage(fmt::format("openFile for session {} file {} directory {} file {}", session_id, file_id, directory, file));

    grpc::Status status(grpc::Status::OK);
    std::string message;
    if (CheckFileId(session_id, file_id, "openFile", message)) {
        // Open file in session if valid render mode
        auto session = _sessions[session_id].first;
        auto hdu = request->hdu();
        int render_mode = request->render_mode();
        if (CARTA::RenderMode_IsValid(render_mode)) {
            CARTA::RenderMode mode = (CARTA::RenderMode)render_mode;
            session->ScriptClientOpenFile(directory, file, hdu, file_id, mode);
            // TODO: get session result here
            status = grpc::Status(grpc::StatusCode::UNIMPLEMENTED, message);
        } else {
            message = fmt::format("openFile {} failed: invalid render mode.", file_id);
            Log(session_id, message);
            reply->set_success(false);
            reply->set_file_id(file_id);
            reply->set_message(message);
            status = grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, message);
        }
    } else {
        Log(session_id, message);
        reply->set_success(false);
        reply->set_file_id(file_id);
        reply->set_message(message);
        status = grpc::Status(grpc::StatusCode::OUT_OF_RANGE, message);
    }

    return status;
}

grpc::Status CartaGrpcService::savePlot(grpc::ServerContext* context, const CARTAVIS::SavePlot* request, CARTAVIS::SavePlotAck* reply) {
    auto session_id = request->session_id();
    auto file_id = request->file_id();
    LogMessage(fmt::format("savePlot for session {} file {}", session_id, file_id));

    grpc::Status status(grpc::Status::OK);
    std::string message;
    if (CheckFileId(session_id, file_id, "savePlot", message)) {
        _sessions[session_id].first->ScriptClientSavePlot(file_id);
        // TODO: get session result here
        status = grpc::Status(grpc::StatusCode::UNIMPLEMENTED, message);
    } else {
        Log(session_id, message);
        reply->set_success(false);
        reply->set_message(message);
        status = grpc::Status(grpc::StatusCode::OUT_OF_RANGE, message);
    }

    return status;
}

grpc::Status CartaGrpcService::setColorMap(grpc::ServerContext* context, const CARTAVIS::SetColorMap* request, google::protobuf::Empty*) {
    auto session_id = request->session_id();
    auto file_id = request->file_id();
    LogMessage(fmt::format("setColorMap for session {} file {}", session_id, file_id));

    grpc::Status status(grpc::Status::OK);
    std::string message;
    if (CheckFileId(session_id, file_id, "setColorMap", message)) {
        int colormap = request->colormap();
        if (CARTA::ColorMap_IsValid(colormap)) {
            CARTA::ColorMap carta_colormap = (CARTA::ColorMap)colormap;
            _sessions[session_id].first->ScriptClientSetColormap(file_id, carta_colormap);
        } else {
            message = fmt::format("setColorMap failed: invalid colormap.");
            Log(session_id, message);
            status = grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, message);
        }
    } else {
        Log(session_id, message);
        status = grpc::Status(grpc::StatusCode::OUT_OF_RANGE, message);
    }

    return status;
}

grpc::Status CartaGrpcService::setCoordinateSystem(
    grpc::ServerContext* context, const CARTAVIS::SetCoordinateSystem* request, google::protobuf::Empty*) {
    auto session_id = request->session_id();
    auto file_id = request->file_id();
    LogMessage(fmt::format("setCoordinateSystem for session {} file {}", session_id, file_id));

    grpc::Status status(grpc::Status::OK);
    std::string message;
    if (CheckFileId(session_id, file_id, "setCoordinateSystem", message)) {
        int ref_frame = request->direction_ref_frame();
        if (CARTA::CoordinateSystem_IsValid(ref_frame)) {
            CARTA::CoordinateSystem csys = (CARTA::CoordinateSystem)ref_frame;
            _sessions[session_id].first->ScriptClientSetCoordinateSystem(file_id, csys);
        } else {
            message = fmt::format("setCoordinateSystem failed: invalid reference frame.");
            Log(session_id, message);
            status = grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, message);
        }
    } else {
        Log(session_id, message);
        status = grpc::Status(grpc::StatusCode::OUT_OF_RANGE, message);
    }

    return status;
}

grpc::Status CartaGrpcService::setImageChannels(
    grpc::ServerContext* context, const CARTAVIS::SetImageChannels* request, google::protobuf::Empty*) {
    auto session_id = request->session_id();
    auto file_id = request->file_id();
    LogMessage(fmt::format("setImageChannels for session {} file {}", session_id, file_id));

    grpc::Status status(grpc::Status::OK);
    std::string message;
    if (CheckFileId(session_id, file_id, "setImageChannels", message)) {
        _sessions[session_id].first->ScriptClientSetImageChannels(file_id, request->channel(), request->stokes());
    } else {
        Log(session_id, message);
        status = grpc::Status(grpc::StatusCode::OUT_OF_RANGE, message);
    }

    return grpc::Status::OK;
}

grpc::Status CartaGrpcService::setImageView(grpc::ServerContext* context, const CARTAVIS::SetImageView* request, google::protobuf::Empty*) {
    auto session_id = request->session_id();
    auto file_id = request->file_id();
    LogMessage(fmt::format("setImageView for session {} file {}", session_id, file_id));

    grpc::Status status(grpc::Status::OK);
    std::string message;
    if (CheckFileId(session_id, file_id, "setImageView", message)) {
        CARTAVIS::ImageBounds req_bounds(request->image_bounds());
        CARTA::ImageBounds new_bounds;
        new_bounds.set_x_min(req_bounds.x_min());
        new_bounds.set_x_max(req_bounds.x_max());
        new_bounds.set_y_min(req_bounds.y_min());
        new_bounds.set_y_max(req_bounds.y_max());
        _sessions[session_id].first->ScriptClientSetImageView(file_id, new_bounds);
        status = grpc::Status(grpc::StatusCode::UNIMPLEMENTED, message);
    } else {
        Log(session_id, message);
        status = grpc::Status(grpc::StatusCode::OUT_OF_RANGE, message);
    }

    return status;
}

grpc::Status CartaGrpcService::showGrid(grpc::ServerContext* context, const CARTAVIS::ShowGrid* request, google::protobuf::Empty*) {
    auto session_id = request->session_id();
    auto file_id = request->file_id();
    LogMessage(fmt::format("showGrid for session {} file {}", session_id, file_id));

    grpc::Status status(grpc::Status::OK);
    std::string message;
    if (CheckFileId(session_id, file_id, "showGrid", message)) {
        _sessions[session_id].first->ScriptClientShowGrid(file_id, request->show_grid());
        status = grpc::Status(grpc::StatusCode::UNIMPLEMENTED, message);
    } else {
        Log(session_id, message);
        status = grpc::Status(grpc::StatusCode::OUT_OF_RANGE, message);
    }

    return status;
}
