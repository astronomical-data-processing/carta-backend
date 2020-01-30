#ifndef CARTA_BACKEND__VOTABLECARRIER_H_
#define CARTA_BACKEND__VOTABLECARRIER_H_

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "VOTableController.h"

namespace catalog {

class VOTableCarrier {
    // For the element <COOSYS> and its attributes
    struct Coosys {
        std::string id;
        std::string equinox;
        std::string epoch;
        std::string system;
        void Print() {
            std::cout << "    id          = " << id << std::endl;
            std::cout << "    equinox     = " << equinox << std::endl;
            std::cout << "    epoch       = " << epoch << std::endl;
            std::cout << "    system      = " << system << std::endl;
        }
    };

    // For the element <FIELD> and its attributes
    struct Field {
        std::string name;
        std::string id;
        std::string datatype;
        std::string arraysize;
        std::string width;
        std::string precision;
        std::string xtype;
        std::string unit;
        std::string ucd;
        std::string utype;
        std::string ref;
        std::string type;
        std::string description;
        void Print() {
            std::cout << "    name        = " << name << std::endl;
            std::cout << "    id          = " << id << std::endl;
            std::cout << "    datatype    = " << datatype << std::endl;
            std::cout << "    arraysize   = " << arraysize << std::endl;
            std::cout << "    width       = " << width << std::endl;
            std::cout << "    precision   = " << precision << std::endl;
            std::cout << "    xtype       = " << xtype << std::endl;
            std::cout << "    unit        = " << unit << std::endl;
            std::cout << "    ucd         = " << ucd << std::endl;
            std::cout << "    utype       = " << utype << std::endl;
            std::cout << "    ref         = " << ref << std::endl;
            std::cout << "    type        = " << type << std::endl;
            std::cout << "    description = " << description << std::endl;
        }
    };

    const float _check_filter_progress_interval = 0.1;

public:
    VOTableCarrier(){};
    ~VOTableCarrier(){};

    void SetFileName(std::string file_path_name);
    void FillVOTableAttributes(std::string name, std::string version);
    void FillFileDescription(std::string description);
    void FillCoosysAttributes(int count, std::string name, std::string value);
    void FillFieldAttributes(int count, std::string name, std::string value);
    void FillFieldDescriptions(int count, std::string value);
    void FillTdValues(int column_index, std::string value);
    void UpdateNumOfTableRows();
    std::string GetFileDescription();
    void GetHeaders(CARTA::CatalogFileInfoResponse& file_info_response);
    void GetHeadersAndData(CARTA::OpenCatalogFileAck& open_file_response, int preview_data_size);
    void GetFilteredData(
        CARTA::CatalogFilterRequest filter_request, std::function<void(CARTA::CatalogFilterResponse)> partial_results_callback);
    size_t GetTableRowNumber();
    static void GetDataType(std::string data_type, CARTA::EntryType& catalog_data_type);
    bool IsValid();

    void PrintTableElement(int row, int column);
    void PrintData();

private:
    bool BoolFilter(CARTA::FilterConfig filter, bool value);
    bool StringFilter(CARTA::FilterConfig filter, std::string value);
    template <typename T>
    bool NumericFilter(CARTA::FilterConfig filter, T value);

    std::string _filename;
    std::string _directory;
    std::string _file_description = "";
    std::string _votable_version = "";       // VOTable version, "" means this is not the VOTable XML file.
    std::unordered_map<int, Coosys> _coosys; // Unordered map for the element <COOSYS>: <COOSYS count (Column Index), COOSYS attributes>
    std::unordered_map<int, Field> _fields;  // Unordered map for the element <FIELD>: <FIELD count, FIELD attributes>
    size_t _num_of_rows = 0;                 // Number of table rows

    // Unordered map for table columns: <Column Index, Column Vector>
    std::unordered_map<int, std::vector<bool>> _bool_vectors;          // For the column with datatype = "boolean"
    std::unordered_map<int, std::vector<std::string>> _string_vectors; // For the column with datatype = "char"
    std::unordered_map<int, std::vector<int>> _int_vectors;            // For the column with datdtype = "int"
    std::unordered_map<int, std::vector<long long>> _ll_vectors;       // For the column with datdtype = "long long"
    std::unordered_map<int, std::vector<float>> _float_vectors;        // For the column with datdtype = "float"
    std::unordered_map<int, std::vector<double>> _double_vectors;      // For the column with datdtype = "double"

    // PS: do not consider the datatypes: "bit", "unsignedByte", "unicodeChar", "floatComplex" and "doubleComplex"
};

} // namespace catalog

#endif // CARTA_BACKEND__VOTABLECARRIER_H_