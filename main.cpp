#include <iostream>
#include <InfluxDB.h>
#include <InfluxDBFactory.h>
#include <cstdlib>


int main() {
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

    return 0;
}
