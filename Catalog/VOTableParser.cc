#include "VOTableParser.h"

using namespace catalog;

VOTableParser::VOTableParser(std::string filename, VOTableCarrier* carrier, bool only_read_to_header, bool verbose)
    : _carrier(carrier), _only_read_to_header(only_read_to_header), _verbose(verbose) {
    if (!IsVOTable(filename)) {
        std::cerr << "File: " << filename << " is NOT a VOTable!" << std::endl;
        _continue_read = false;
        return;
    }
    _reader = xmlReaderForFile(filename.c_str(), NULL, 0);
    if (_reader == nullptr) {
        std::cerr << "Unable to open " << filename << std::endl;
        return;
    }
    if (_carrier) {
        _carrier->SetFileName(filename);
    }
    Scan(); // Scan the VOTable
}

VOTableParser::~VOTableParser() {
    xmlFreeTextReader(_reader);
    CleanupParser();
}

bool VOTableParser::IsVOTable(std::string filename) {
    xmlTextReaderPtr reader;
    xmlChar* name_xmlchar;
    std::string name;
    reader = xmlReaderForFile(filename.c_str(), NULL, 0);
    if (xmlTextReaderRead(reader)) {
        if (xmlTextReaderNodeType(reader) == XML_READER_TYPE_ELEMENT) {
            name_xmlchar = xmlTextReaderName(reader);
            name = (char*)name_xmlchar;
            if (GetElementName(name) == VOTABLE) {
                xmlFreeTextReader(reader);
                CleanupParser();
                return true;
            }
        }
    }
    xmlFreeTextReader(reader);
    CleanupParser();
    return false;
}

void VOTableParser::CleanupParser() {
    // Cleanup function for the XML library.
    xmlCleanupParser();

    // This is to debug memory for regression tests
    xmlMemoryDump();
}

void VOTableParser::Scan() {
    int result;
    do {
        result = xmlTextReaderRead(_reader);
        if (result && _continue_read) {
            Parse(); // Parse the VOTable and store data in the VOTableCarrier
        } else {
            if (_verbose) {
                std::cout << "End of the XML file." << std::endl;
            }
            break;
        }
    } while (true);
}

void VOTableParser::Parse() {
    int node_type = xmlTextReaderNodeType(_reader);
    xmlChar* name_xmlchar;
    xmlChar* value_xmlchar;
    name_xmlchar = xmlTextReaderName(_reader);
    value_xmlchar = xmlTextReaderValue(_reader);

    std::string name;
    std::string value;
    name = (char*)name_xmlchar;
    xmlFree(name_xmlchar); // Not sure if it is necessary, just for safety.
    if (value_xmlchar) {
        value = (char*)value_xmlchar;
        xmlFree(value_xmlchar); // Not sure if it is necessary, just for safety.
    }

    switch (node_type) {
        case XML_READER_TYPE_ELEMENT:
            Print("<" + name + ">", value);
            _pre_element_name = _element_name;
            _element_name = GetElementName(name);
            if (_only_read_to_header && _element_name == DATA) {
                _continue_read = false;
                break;
            }
            IncreaseElementCounts(_element_name);
            // Loop through the attributes
            while (xmlTextReaderMoveToNextAttribute(_reader)) {
                Parse();
            }
            break;
        case XML_READER_TYPE_END_ELEMENT:
            Print("</" + name + ">", value);
            if (_element_name == TD && !_td_filled && _carrier) {
                // Fill the TR element values as "" if there is an empty column, i.e. <TD></TD>.
                _carrier->FillTdValues(_td_counts, "");
                _td_filled = true; // Decrease the TD counter in order to mark such TR element has been filled
            }
            break;
        case XML_READER_TYPE_ATTRIBUTE:
            Print("    " + name, value);
            FillElementAttributes(_element_name, name, value);
            break;
        case XML_READER_TYPE_TEXT:
            Print("    " + name, value);
            FillElementValues(_element_name, value);
            break;

            // Regardless the following node types
        case XML_READER_TYPE_NONE:
            break;
        case XML_READER_TYPE_SIGNIFICANT_WHITESPACE:
            break;
        case XML_READER_TYPE_WHITESPACE:
            break;
        case XML_READER_TYPE_CDATA:
            break;
        case XML_READER_TYPE_ENTITY_REFERENCE:
            break;
        case XML_READER_TYPE_ENTITY:
            break;
        case XML_READER_TYPE_PROCESSING_INSTRUCTION:
            break;
        case XML_READER_TYPE_COMMENT:
            break;
        case XML_READER_TYPE_DOCUMENT:
            break;
        case XML_READER_TYPE_DOCUMENT_TYPE:
            break;
        case XML_READER_TYPE_DOCUMENT_FRAGMENT:
            break;
        case XML_READER_TYPE_NOTATION:
            break;

        default:
            std::cerr << "Fail to parse the XML text!" << std::endl;
    }
}

void VOTableParser::Print(std::string name, std::string value) {
    if (_verbose) {
        if (name.empty() && !value.empty()) {
            std::cout << value << std::endl;
        } else if (!name.empty() && value.empty()) {
            std::cout << name << std::endl;
        } else {
            std::cout << name << " : " << value << std::endl;
        }
    }
}

VOTableParser::ElementName VOTableParser::GetElementName(std::string name) {
    VOTableParser::ElementName result;
    if (strcmp(name.c_str(), "VOTABLE") == 0) {
        result = VOTABLE;
    } else if (strcmp(name.c_str(), "RESOURCE") == 0) {
        result = RESOURCE;
    } else if (strcmp(name.c_str(), "DESCRIPTION") == 0) {
        result = DESCRIPTION;
    } else if (strcmp(name.c_str(), "DEFINITIONS") == 0) {
        result = DEFINITIONS;
    } else if (strcmp(name.c_str(), "INFO") == 0) {
        result = INFO;
    } else if (strcmp(name.c_str(), "PARAM") == 0) {
        result = PARAM;
    } else if (strcmp(name.c_str(), "TABLE") == 0) {
        result = TABLE;
    } else if (strcmp(name.c_str(), "FIELD") == 0) {
        result = FIELD;
    } else if (strcmp(name.c_str(), "GROUP") == 0) {
        result = GROUP;
    } else if (strcmp(name.c_str(), "FIELDref") == 0) {
        result = FIELDref;
    } else if (strcmp(name.c_str(), "PARAMref") == 0) {
        result = PARAMref;
    } else if (strcmp(name.c_str(), "VALUES") == 0) {
        result = VALUES;
    } else if (strcmp(name.c_str(), "MIN") == 0) {
        result = MIN;
    } else if (strcmp(name.c_str(), "MAX") == 0) {
        result = MAX;
    } else if (strcmp(name.c_str(), "OPTION") == 0) {
        result = OPTION;
    } else if (strcmp(name.c_str(), "LINK") == 0) {
        result = LINK;
    } else if (strcmp(name.c_str(), "DATA") == 0) {
        result = DATA;
    } else if (strcmp(name.c_str(), "TABLEDATA") == 0) {
        result = TABLEDATA;
    } else if (strcmp(name.c_str(), "TD") == 0) {
        result = TD;
    } else if (strcmp(name.c_str(), "TR") == 0) {
        result = TR;
    } else if (strcmp(name.c_str(), "FITS") == 0) {
        result = FITS;
    } else if (strcmp(name.c_str(), "BINARY") == 0) {
        result = BINARY;
    } else if (strcmp(name.c_str(), "STREAM") == 0) {
        result = STREAM;
    } else if (strcmp(name.c_str(), "COOSYS") == 0) {
        result = COOSYS;
    } else {
        result = NONE;
    }
    return result;
}

void VOTableParser::IncreaseElementCounts(ElementName element_name) {
    switch (element_name) {
        case COOSYS:
            ++_coosys_counts;
            break;
        case FIELD:
            ++_field_counts;
            break;
        case TR:
            ++_tr_counts;
            break;
        case TD:
            if (_field_counts > 0) {
                _td_counts = (((_td_counts + 1) % _field_counts) == 0) ? _field_counts : ((_td_counts + 1) % _field_counts);
            } else {
                std::cerr << "There is no column header!" << std::endl;
            }
            _td_filled = false;
            break;
        default:; // Do not count any elements
    }
}

void VOTableParser::FillElementAttributes(ElementName element_name, std::string name, std::string value) {
    if (!_carrier) {
        std::cerr << "The VOTableCarrier pointer is null!" << std::endl;
        return;
    }
    switch (element_name) {
        case VOTABLE:
            _carrier->FillVOTableAttributes(name, value);
            break;
        case COOSYS:
            _carrier->FillCoosysAttributes(_coosys_counts, name, value);
            break;
        case FIELD:
            _carrier->FillFieldAttributes(_field_counts, name, value);
            break;
        default:; // Do not fill any attributes
    }
}

void VOTableParser::FillElementValues(ElementName element_name, std::string value) {
    if (!_carrier) {
        std::cerr << "The VOTableCarrier pointer is null!" << std::endl;
        return;
    }
    switch (element_name) {
        case DESCRIPTION:
            if (_pre_element_name == FIELD) {
                _carrier->FillFieldDescriptions(_field_counts, value);
            } else {
                _carrier->FillFileDescription(value);
            }
            break;
        case TD:
            if (!_td_filled) {
                _carrier->FillTdValues(_td_counts, value);
                _td_filled = true;
            }
            break;
        default:; // Do not fill any values
    }
}