#include <iostream>
#include <cstdlib>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

#include <InfluxDB.h>
#include <InfluxDBFactory.h>

constexpr char delimiter = ',';

std::vector<std::vector<double>> readCSV(std::ifstream& csvFile) {
    std::vector<std::vector<double>> lines{};
    std::string currentLine;

    while(std::getline(csvFile, currentLine)) {
        //std::cout << currentLine;

        std::vector<double> currentLineVec;
        std::stringstream currentLineSS (currentLine);
        
        std::string curNumStr;
        while(std::getline(currentLineSS, curNumStr, delimiter)) {
            currentLineVec.push_back(std::stod(curNumStr));
            //std::cout << curNumStr << " ";
        }
        lines.push_back(currentLineVec);
        //std::cout << '\n';

    }

    return lines;
    
}

int main(int argc, char * argv[]) {
    // Only continue running if INFLUX_HOST_URL can be assigned successfully; 
    const char * hostUrl;
    if(!(hostUrl = std::getenv("INFLUX_HOST_URL"))) {
        std::cout << "Please set INFLUX_HOST_URL to the database URL you want to connect to.\n";
        return EXIT_FAILURE;
    }

    // connect
    auto db = influxdb::InfluxDBFactory::Get(hostUrl);
    db->createDatabaseIfNotExists();

    for(auto query : db->query("SHOW DATABASES")) {
        std::cout << query.getTags() << '\n';
    }

    std::ifstream argFile {argv[1]};

    std::vector<std::vector<double>> csvData {readCSV(argFile)};

    for(auto line : csvData) {
        for(auto num: line) {
            std::cout << num << " , ";
        }
        std::cout << '\n';
    }

    return 0;
}
