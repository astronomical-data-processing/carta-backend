//# CartaGrpcService.cc: grpc server to receive messages from the python scripting client

#include "CartaGrpcService.h"

#include <chrono>

#include <carta-protobuf/defs.pb.h>
#include <carta-protobuf/enums.pb.h>

#include "../InterfaceConstants.h"
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
    // Use Log() in Util.h for error messages
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
        // send request to session
        auto session = _sessions[session_id].first;
        session->ScriptClientGetRenderedImage(file_id);

        // get result (plot data) or timeout
        auto t_start = std::chrono::system_clock::now();
        while (!GetPlotData(session, file_id)) {
            auto t_end = std::chrono::system_clock::now();
            std::chrono::duration<double> elapsed_sec = t_end - t_start;
            if (elapsed_sec.count() > RENDERED_DATA_TIMEOUT) {
                // plot data timed out
                message = fmt::format("getRenderedImage for file ID {} timed out", file_id);
                Log(session_id, message);
                return grpc::Status(grpc::StatusCode::DEADLINE_EXCEEDED, message);
            }
        }

        // set reply
        reply->set_base64(_plot_data.c_str(), _plot_data.size());
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
    auto hdu = request->hdu();
    LogMessage(fmt::format("openFile for session {} file ID {} directory {} file {} hdu {}", session_id, file_id, directory, file, hdu));

    grpc::Status status(grpc::Status::OK);
    reply->set_file_id(file_id);
    std::string message;
    if (CheckSessionId(session_id, "openFile", message)) {
        // check if file id already exists
        auto session = _sessions[session_id].first;
        if (session->HasFileId(file_id)) {
            std::string message = fmt::format("openFile failed: file ID {} already exists", file_id);
            Log(session_id, message);
            reply->set_success(false);
            reply->set_message(message);
            return grpc::Status(grpc::StatusCode::ALREADY_EXISTS, message);
        }

        // Check render mode
        int render_mode = request->render_mode();
        if (CARTA::RenderMode_IsValid(render_mode)) {
            CARTA::RenderMode mode = (CARTA::RenderMode)render_mode;

            // send request to session
            session->ScriptClientOpenFile(directory, file, hdu, file_id, mode);

            // get result (new file id or error message), or timeout
            auto t_start = std::chrono::system_clock::now();
            while (!session->HasFileId(file_id)) {
                // check for error message for open file
                message = session->GetFileOpenError(file_id);
                if (!message.empty()) {
                    Log(session_id, message);
                    reply->set_success(false);
                    reply->set_message(message);
                    return grpc::Status(grpc::StatusCode::UNKNOWN, message);
                }

                // check for timeout
                auto t_end = std::chrono::system_clock::now();
                std::chrono::duration<double> elapsed_sec = t_end - t_start;
                if (elapsed_sec.count() > OPEN_FILE_TIMEOUT) {
                    message = fmt::format("openFile for file ID {} timed out", file_id);
                    Log(session_id, message);
                    reply->set_success(false);
                    reply->set_message(message);
                    return grpc::Status(grpc::StatusCode::DEADLINE_EXCEEDED, message);
                }
            }

            // set reply
            reply->set_success(true);
        } else {
            message = fmt::format("openFile file ID {} failed: invalid render mode {}.", file_id, render_mode);
            Log(session_id, message);
            reply->set_success(false);
            reply->set_message(message);
            status = grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, message);
        }
    } else {
        Log(session_id, message);
        reply->set_success(false);
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
        auto session = _sessions[session_id].first;
        session->ScriptClientSavePlot(file_id);

        // get result (plot filename) or timeout
        auto t_start = std::chrono::system_clock::now();
        while (!GetPlotFilename(session, file_id)) {
            auto t_end = std::chrono::system_clock::now();
            std::chrono::duration<double> elapsed_sec = t_end - t_start;
            if (elapsed_sec.count() > SAVE_PLOT_TIMEOUT) {
                // plotname timed out
                message = fmt::format("savePlot for file ID {} timed out", file_id);
                Log(session_id, message);
                reply->set_success(false);
                reply->set_message(message);
                return grpc::Status(grpc::StatusCode::DEADLINE_EXCEEDED, message);
            }
        }

        // set reply
        reply->set_success(true);
        reply->set_file_name(_plot_filename);
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

// get results

bool CartaGrpcService::GetPlotFilename(Session* session, int file_id) {
    _plot_filename = session->GetPlotFilename(file_id);
    return !_plot_filename.empty();
}

bool CartaGrpcService::GetPlotData(Session* session, int file_id) {
    _plot_data = session->GetRenderedImage(file_id);
    return !_plot_data.empty();
}
