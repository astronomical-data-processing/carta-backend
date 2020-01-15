//# CartaBackendService.h: server for a grpc client

#ifndef CARTA_BACKEND_GRPCSERVER_CARTABACKENDSERVICE_H_
#define CARTA_BACKEND_GRPCSERVER_CARTABACKENDSERVICE_H_

#include <grpc++/grpc++.h>

#include <cartavis/carta_service.grpc.pb.h>

class CartaBackendService : public CARTAVIS::CartaBackend::Service {

public:
    grpc::Status connectToSession(grpc::ServerContext* context, const CARTAVIS::ConnectToSession* request, CARTAVIS::ConnectToSessionAck* reply);

    grpc::Status disconnectFromSession(grpc::ServerContext* context, const CARTAVIS::DisconnectFromSession* request, google::protobuf::Empty*);

    grpc::Status openFile(grpc::ServerContext* context, const CARTAVIS::OpenFile* request, CARTAVIS::OpenFileAck* reply);

    grpc::Status closeFile(grpc::ServerContext* context, const CARTAVIS::CloseFile* request, google::protobuf::Empty*);

    grpc::Status setColorMap(grpc::ServerContext* context, const CARTAVIS::SetColorMap* request, google::protobuf::Empty*);

    grpc::Status setCoordinateSystem(grpc::ServerContext* context, const CARTAVIS::SetCoordinateSystem* request, google::protobuf::Empty*);

    grpc::Status setImageChannels(grpc::ServerContext* context, const CARTAVIS::SetImageChannels* request, google::protobuf::Empty*);

    grpc::Status setImageView(grpc::ServerContext* context, const CARTAVIS::SetImageView* request, google::protobuf::Empty*);

    grpc::Status showGrid(grpc::ServerContext* context, const CARTAVIS::ShowGrid* request, google::protobuf::Empty*);

    grpc::Status getRenderedImage(grpc::ServerContext* context, const CARTAVIS::GetRenderedImage* request, CARTAVIS::GetRenderedImageAck* reply);

    grpc::Status savePlot(grpc::ServerContext* context, const CARTAVIS::SavePlot* request, CARTAVIS::SavePlotAck* reply);
};

#endif //CARTA_BACKEND_GRPCSERVER_CARTABACKENDSERVICE_H_
