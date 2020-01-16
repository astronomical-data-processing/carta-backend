//# CartaGrpcService.cc: grpc server to receive messages from the python scripting client

#include "CartaGrpcService.h"

CartaGrpcService::CartaGrpcService(bool verbose) : _verbose(verbose) {}

void CartaGrpcService::AddSession(Session* session) {
    // Associate Session with its id
    uint32_t id = session->GetId();
    std::pair<Session*, bool> session_info(session, false);
    _sessions[id] = session_info;
}

void CartaGrpcService::RemoveSession(Session* session) {
    // Remove Session from map
    uint32_t id = session->GetId();
    if (_sessions.count(id)) {
        _sessions.erase(id);
    }
}

bool CartaGrpcService::CheckSessionId(uint32_t id, const std::string& command, std::string& message) {
    bool id_ok(true);
    if (!_sessions.count(id)) {
        message = fmt::format("{} failed: Session {} does not exist", command, id);
        id_ok = false;
    } else if (!_sessions[id].second) {
        message = fmt::format("{} failed: Session {} is not connected", command, id);
        id_ok = false;
    }
    return id_ok;
}

grpc::Status CartaGrpcService::connectToSession(
    grpc::ServerContext* context, const CARTAVIS::ConnectToSession* request, CARTAVIS::ConnectToSessionAck* reply) {
    uint32_t id = request->session_id();
    if (_verbose) {
        std::cout << "GRPC_DEBUG: received connectToSession " << id << std::endl;
    }
    if (_sessions.count(id)) {
        _sessions[id].second = true;
        reply->set_success(true);
    } else {
        reply->set_success(false);
        std::string message = fmt::format("Connection failed: Session {} does not exist", id);
        reply->set_message(message);
    }
    return grpc::Status::OK;
}

grpc::Status CartaGrpcService::disconnectFromSession(
    grpc::ServerContext* context, const CARTAVIS::DisconnectFromSession* request, google::protobuf::Empty*) {
    uint32_t id = request->session_id();
    if (_verbose) {
        std::cout << "GRPC_DEBUG: received disconnectFromSession " << id << std::endl;
    }
    if (_sessions.count(id)) {
        _sessions[id].second = false;
    }
    return grpc::Status::OK;
}

grpc::Status CartaGrpcService::openFile(grpc::ServerContext* context, const CARTAVIS::OpenFile* request, CARTAVIS::OpenFileAck* reply) {
    if (_verbose) {
        std::cout << "GRPC_DEBUG: received openFile" << std::endl;
    }

    uint32_t id = request->session_id();
    std::string message;
    if (CheckSessionId(id, "open_file", message)) {
        // call session method
    } else {
        reply->set_success(false);
        reply->set_message(message);
    }

    return grpc::Status::OK;
}

grpc::Status CartaGrpcService::closeFile(grpc::ServerContext* context, const CARTAVIS::CloseFile* request, google::protobuf::Empty*) {
    if (_verbose) {
        std::cout << "GRPC_DEBUG: received closeFile" << std::endl;
    }
    return grpc::Status::OK;
}

grpc::Status CartaGrpcService::setColorMap(grpc::ServerContext* context, const CARTAVIS::SetColorMap* request, google::protobuf::Empty*) {
    if (_verbose) {
        std::cout << "GRPC_DEBUG: received setColorMap" << std::endl;
    }
    return grpc::Status::OK;
}

grpc::Status CartaGrpcService::setCoordinateSystem(
    grpc::ServerContext* context, const CARTAVIS::SetCoordinateSystem* request, google::protobuf::Empty*) {
    if (_verbose) {
        std::cout << "GRPC_DEBUG: received setCooordinateSystem" << std::endl;
    }
    return grpc::Status::OK;
}

grpc::Status CartaGrpcService::setImageChannels(
    grpc::ServerContext* context, const CARTAVIS::SetImageChannels* request, google::protobuf::Empty*) {
    if (_verbose) {
        std::cout << "GRPC_DEBUG: received setImageChannels" << std::endl;
    }
    return grpc::Status::OK;
}

grpc::Status CartaGrpcService::setImageView(grpc::ServerContext* context, const CARTAVIS::SetImageView* request, google::protobuf::Empty*) {
    if (_verbose) {
        std::cout << "GRPC_DEBUG: received setImageView" << std::endl;
    }
    return grpc::Status::OK;
}

grpc::Status CartaGrpcService::showGrid(grpc::ServerContext* context, const CARTAVIS::ShowGrid* request, google::protobuf::Empty*) {
    if (_verbose) {
        std::cout << "GRPC_DEBUG: received showGrid" << std::endl;
    }
    return grpc::Status::OK;
}

grpc::Status CartaGrpcService::getRenderedImage(
    grpc::ServerContext* context, const CARTAVIS::GetRenderedImage* request, CARTAVIS::GetRenderedImageAck* reply) {
    if (_verbose) {
        std::cout << "GRPC_DEBUG: received getRenderedImage" << std::endl;
    }
    return grpc::Status::OK;
}

grpc::Status CartaGrpcService::savePlot(grpc::ServerContext* context, const CARTAVIS::SavePlot* request, CARTAVIS::SavePlotAck* reply) {
    if (_verbose) {
        std::cout << "GRPC_DEBUG: received savePlot" << std::endl;
    }
    return grpc::Status::OK;
}
