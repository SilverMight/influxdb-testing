#include <chrono>
#include <iostream>
#include <cstdlib>
#include <ratio>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <thread>

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
    std::cout << hostUrl << "\n";
    auto db = influxdb::InfluxDBFactory::Get(hostUrl);
    db->createDatabaseIfNotExists();

    for(auto query : db->query("SHOW DATABASES")) {
        std::cout << query.getTags() << '\n';
    }

    std::ifstream argFile {argv[1]};

    std::vector<std::vector<double>> csvData {readCSV(argFile)};

    /* 
    db->batchOf(10000);
    int count = 0;
    for(const auto& line : csvData) {
        //influxdb::Point curPoint{"something"};
        //curPoint.addField("Field 1", line[1]).addField("Field 2", line[2]);
        std::cout << line[0] << " " << line[1] << " " << line[2] << '\n';

        //std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> now = std::chrono::system_clock::now();
        std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> now{std::chrono::nanoseconds(static_cast<int>(line[0]))};
        //const auto curTime = now + std::chrono::nanoseconds((int)line[0]);
        
        db->write(influxdb::Point{"Something"}.addField("Field 1", line[1]).addField("Field 2", line[2]).setTimestamp(now));
        count++;
        if(count == 10000 - 1) {
            db->flushBatch();
            count = 0;
        }         
        std::this_thread::sleep_for(std::chrono::nanoseconds(100));
    }
    */

    std::vector<influxdb::Point> points = db->query("SELECT * FROM Something");
    for(const auto& point : points) {
        std::cout << point.getFields() << '\n';
    }

    return 0;
}
