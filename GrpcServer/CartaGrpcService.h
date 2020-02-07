//# CartaGrpcService.h: service for a grpc client

#ifndef CARTA_BACKEND_GRPCSERVER_CARTAGRPCSERVICE_H_
#define CARTA_BACKEND_GRPCSERVER_CARTAGRPCSERVICE_H_

#include <condition_variable>

#include <grpc++/grpc++.h>

#include <cartavis/carta_service.grpc.pb.h>

#include "../Session.h"

class CartaGrpcService : public CARTAVIS::CartaBackend::Service {
public:
    CartaGrpcService(bool verbose);
    void AddSession(Session* session);
    void RemoveSession(Session* session);

    // CARTAVIS proto requests (alpha order)

    grpc::Status closeFile(grpc::ServerContext* context, const CARTAVIS::CloseFile* request, google::protobuf::Empty*);

    grpc::Status connectToSession(
        grpc::ServerContext* context, const CARTAVIS::ConnectToSession* request, CARTAVIS::ConnectToSessionAck* reply);

    grpc::Status disconnectFromSession(
        grpc::ServerContext* context, const CARTAVIS::DisconnectFromSession* request, google::protobuf::Empty*);

    grpc::Status getRenderedImage(
        grpc::ServerContext* context, const CARTAVIS::GetRenderedImage* request, CARTAVIS::GetRenderedImageAck* reply);

    grpc::Status openFile(grpc::ServerContext* context, const CARTAVIS::OpenFile* request, CARTAVIS::OpenFileAck* reply);

    grpc::Status savePlot(grpc::ServerContext* context, const CARTAVIS::SavePlot* request, CARTAVIS::SavePlotAck* reply);

    grpc::Status setColorMap(grpc::ServerContext* context, const CARTAVIS::SetColorMap* request, google::protobuf::Empty*);

    grpc::Status setCoordinateSystem(grpc::ServerContext* context, const CARTAVIS::SetCoordinateSystem* request, google::protobuf::Empty*);

    grpc::Status setImageChannels(grpc::ServerContext* context, const CARTAVIS::SetImageChannels* request, google::protobuf::Empty*);

    grpc::Status setImageView(grpc::ServerContext* context, const CARTAVIS::SetImageView* request, google::protobuf::Empty*);

    grpc::Status showGrid(grpc::ServerContext* context, const CARTAVIS::ShowGrid* request, google::protobuf::Empty*);

private:
    // Utility functions
    bool CheckSessionId(uint32_t session_id, const std::string& command, std::string& message);
    bool CheckFileId(uint32_t session_id, uint32_t file_id, const std::string& command, std::string& message);
    void LogMessage(const std::string& message);

    // Get results for replies
    bool GetPlotFilename(Session* session, int file_id);
    bool GetPlotData(Session* session, int file_id);

    bool _verbose;

    // Map session_id to <Session*, connected>
    std::unordered_map<int, std::pair<Session*, bool>> _sessions;

    // results for grpc replies
    std::string _plot_filename;
    std::string _plot_data;
};

#endif // CARTA_BACKEND_GRPCSERVER_CARTAGRPCSERVICE_H_
