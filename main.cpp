#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <chrono>
#include <fstream>
#include <sstream>

#include <InfluxDB.h>
#include <InfluxDBFactory.h>

// Read CSV into a 2D array
// Each line is vector of doubles
std::vector<std::vector<double>> readCSV(std::ifstream& csvFile) {
    // delimiter for CSV
    constexpr char delimiter = ',';
    
    // array of doubles for each line
    std::vector<std::vector<double>> lines{};
    std::string currentLine;

    // Get current line
    while(std::getline(csvFile, currentLine)) {
        // vector to hold current lines numbers
        std::vector<double> currentLineVec;

        // We need to use a stringstream to work with getline
        std::istringstream currentLineSS (currentLine);
        
        std::string valueString;
        // From our stringstream, read number seperated by comma into valueString
        while(std::getline(currentLineSS, valueString, delimiter)) {
            // convert it to a double and push it back
            currentLineVec.push_back(std::stod(valueString));
        }

        // push back vector of doubles into our line array
        lines.push_back(currentLineVec);

    }

    return lines;
    
}

int main(int argc, char * argv[]) {
    // Only continue running if INFLUX_HOST_URL can be assigned successfully 
    const char * hostUrl;
    if(!(hostUrl = std::getenv("INFLUX_HOST_URL"))) {
        std::cout << "Please set INFLUX_HOST_URL to the database URL you want to connect to.\n";
        return EXIT_FAILURE;
    }

    // connect to our db
    std::cout << hostUrl << "\n";
    auto db = influxdb::InfluxDBFactory::Get(hostUrl);
    db->createDatabaseIfNotExists();

    // Get input file from arguments
    std::ifstream argFile {argv[1]};

    // Read from CSV
    std::vector<std::vector<double>> csvData {readCSV(argFile)};

    // Set how many writes we want to batch together
    constexpr int batchSize = 10000;
    db->batchOf(batchSize);

    // This will store how many writes we made
    // Will help us flush the batch when necessary
    int writeCount = 0;
    
    // Iterate by line over our data 
    for(const auto& line : csvData) {
        using namespace std::chrono;

        // This takes the first field of the data (which is a timestamp) and puts it into a timepoint
        // (Since we read it in as a double, we must cast to an int)
        time_point<system_clock, nanoseconds> time {nanoseconds(static_cast<int>(line[0]))};
        
        // Point of measurement, stores fields and timestamp
        auto point = influxdb::Point("Data");

        // Loop over the rest of the current line and add it as a field to our point
        for(int i = 1; i < line.size(); ++i) {
            // Our field name would be more significant than "Field #", but just for an example
            point.addField("Field" + std::to_string(i), line[i]);
        }

        point.setTimestamp(time);

        // Write our point
        // (we need to use std::move because db->write only accepts r-value references)
        db->write(std::move(point));

        // Keep track of our writes 
        // Flush when batchcount reached, reset count
        writeCount++;
        if(writeCount >= (batchSize - 1)) {
            db->flushBatch();
            writeCount = 0;
        }         
    }
    
     
    //Probably wouldn't be too necessary for the backend, but here's how we could
    //query every point into a vector
    std::vector<influxdb::Point> points = db->query("SELECT * FROM Data");
    for(const auto& point : points) {
        std::cout << point.getFields() << '\n';
    }

    return 0;
}
