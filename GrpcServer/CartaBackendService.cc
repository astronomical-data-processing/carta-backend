//# CartaBackendService.cc: grpc server to receive messages from the python scripting client

#include "CartaBackendService.h"

grpc::Status CartaBackendService::connectToSession(grpc::ServerContext* context, const CARTAVIS::ConnectToSession* request, CARTAVIS::ConnectToSessionAck* reply) {
    return grpc::Status::OK;
}

grpc::Status CartaBackendService::disconnectFromSession(grpc::ServerContext* context, const CARTAVIS::DisconnectFromSession* request, google::protobuf::Empty*) {
    return grpc::Status::OK;
}

grpc::Status CartaBackendService::openFile(grpc::ServerContext* context, const CARTAVIS::OpenFile* request, CARTAVIS::OpenFileAck* reply) {
    return grpc::Status::OK;
}
 
grpc::Status CartaBackendService::closeFile(grpc::ServerContext* context, const CARTAVIS::CloseFile* request, google::protobuf::Empty*) {
    return grpc::Status::OK;
}

grpc::Status CartaBackendService::setColorMap(grpc::ServerContext* context, const CARTAVIS::SetColorMap* request, google::protobuf::Empty*) {
    return grpc::Status::OK;
}

grpc::Status CartaBackendService::setCoordinateSystem(grpc::ServerContext* context, const CARTAVIS::SetCoordinateSystem* request, google::protobuf::Empty*) {
    return grpc::Status::OK;
}

grpc::Status CartaBackendService::setImageChannels(grpc::ServerContext* context, const CARTAVIS::SetImageChannels* request, google::protobuf::Empty*) {
    return grpc::Status::OK;
}

grpc::Status CartaBackendService::setImageView(grpc::ServerContext* context, const CARTAVIS::SetImageView* request, google::protobuf::Empty*) {
    return grpc::Status::OK;
}

grpc::Status CartaBackendService::showGrid(grpc::ServerContext* context, const CARTAVIS::ShowGrid* request, google::protobuf::Empty*) {
    return grpc::Status::OK;
}

grpc::Status CartaBackendService::getRenderedImage(grpc::ServerContext* context, const CARTAVIS::GetRenderedImage* request, CARTAVIS::GetRenderedImageAck* reply) {
    return grpc::Status::OK;
}

grpc::Status CartaBackendService::savePlot(grpc::ServerContext* context, const CARTAVIS::SavePlot* request, CARTAVIS::SavePlotAck* reply) {
    return grpc::Status::OK;
}
