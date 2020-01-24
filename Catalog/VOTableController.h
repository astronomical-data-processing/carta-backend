#ifndef CARTA_BACKEND__VOTABLECONTROLLER_H_
#define CARTA_BACKEND__VOTABLECONTROLLER_H_

#include <carta-protobuf/catalog_file_info.pb.h>
#include <carta-protobuf/catalog_filter.pb.h>
#include <carta-protobuf/catalog_list.pb.h>
#include <carta-protobuf/open_catalog_file.pb.h>

#include <cmath>
#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex> 

namespace catalog {

class VOTableCarrier;

class Controller {
    const int _default_preview_row_numbers = 50;

public:
    Controller(){};
    ~Controller();

    static void OnFileListRequest(CARTA::CatalogListRequest file_list_request, CARTA::CatalogListResponse& file_list_response);
    static void OnFileInfoRequest(CARTA::CatalogFileInfoRequest file_info_request, CARTA::CatalogFileInfoResponse& file_info_response);
    void OnOpenFileRequest(CARTA::OpenCatalogFile open_file_request, CARTA::OpenCatalogFileAck& open_file_response);
    void OnCloseFileRequest(CARTA::CloseCatalogFile close_file_request);
    void OnFilterRequest(
        CARTA::CatalogFilterRequest filter_request, std::function<void(CARTA::CatalogFilterResponse)> partial_results_callback);

private:
    static std::string GetCurrentWorkingPath();
    static std::string GetFileSize(std::string file_path_name);
    static int GetFileKBSize(std::string file_path_name);
    static void ParseBasePath(std::string& file_path_name);
    static std::string Concatenate(std::string directory, std::string filename);
    void CloseFile(int file_id);

    std::unordered_map<int, VOTableCarrier*> _carriers; // The unordered map for <File Id, VOTableCarrier Ptr>
    std::mutex _carriers_mutex;
};

} // namespace catalog

#endif // CARTA_BACKEND__VOTABLECONTROLLER_H_