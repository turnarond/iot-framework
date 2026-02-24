#include "servermgrctl.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    try {
        Servermgrctl ctl;
        
        if (argc < 2) {
            ctl.showHelp();
            return 1;
        }
        
        std::string command = argv[1];
        
        if (command == "help") {
            ctl.showHelp();
        } else if (command == "list") {
            ctl.listServices();
        } else if (command == "start") {
            if (argc < 3) {
                std::cerr << "Usage: servermgrctl start <service_name>" << std::endl;
                return 1;
            }
            ctl.startService(argv[2]);
        } else if (command == "stop") {
            if (argc < 3) {
                std::cerr << "Usage: servermgrctl stop <service_name>" << std::endl;
                return 1;
            }
            ctl.stopService(argv[2]);
        } else if (command == "restart") {
            if (argc < 3) {
                std::cerr << "Usage: servermgrctl restart <service_name>" << std::endl;
                return 1;
            }
            ctl.restartService(argv[2]);
        } else if (command == "reload") {
            ctl.reloadConfig();
        } else {
            std::cerr << "Unknown command: " << command << std::endl;
            ctl.showHelp();
            return 1;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
