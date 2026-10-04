

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

static std::string sendCommand(const std::string& command, const std::string& ip = "127.0.0.1", uint16_t port = 8080) {
    int sockFd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockFd < 0) {
        return "Error creating socket\n";
    }

    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    setsockopt(sockFd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
    setsockopt(sockFd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof(tv));

    sockaddr_in serverAddr{};
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr) <= 0) {
        close(sockFd);
        return "Invalid IP address\n";
    }

    if (connect(sockFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        close(sockFd);
        return "Error: Storage daemon not running on " + ip + ":" + std::to_string(port) + ". Start daemon first.\n";
    }

    ssize_t bytesSent = send(sockFd, command.c_str(), command.size(), 0);
    if (bytesSent <= 0) {
        close(sockFd);
        return "Error sending command\n";
    }

    char buffer[4096] = {0};
    ssize_t bytesRead = read(sockFd, buffer, sizeof(buffer) - 1);
    close(sockFd);

    if (bytesRead > 0) {
        return std::string(buffer, bytesRead);
    }

    return "No response received\n";
}

void printHeader() {
    std::cout << "============================================================" << std::endl;
    std::cout << " Storage Orchestrator CLI" << std::endl;
    std::cout << " Student: Debashis Tripathy (2341020048)" << std::endl;
    std::cout << "============================================================" << std::endl;
}

void printHelp() {
    std::cout << "Commands:" << std::endl;
    std::cout << "  STATS       - Display storage and driver statistics" << std::endl;
    std::cout << "  PROC        - Display system process CPU and RAM metrics" << std::endl;
    std::cout << "  RATE <ms>   - Set sampling rate in milliseconds" << std::endl;
    std::cout << "  FAULT_ON    - Enable stress test mode" << std::endl;
    std::cout << "  FAULT_OFF   - Disable stress test mode" << std::endl;
    std::cout << "  PING        - Check server health" << std::endl;
    std::cout << "  CLEAR       - Clear screen" << std::endl;
    std::cout << "  HELP        - Print command list" << std::endl;
    std::cout << "  QUIT / EXIT - Exit CLI" << std::endl;
    std::cout << "------------------------------------------------------------" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        std::string fullCmd;
        for (int i = 1; i < argc; ++i) {
            if (i > 1) fullCmd += " ";
            fullCmd += argv[i];
        }
        std::cout << sendCommand(fullCmd);
        return 0;
    }

    printHeader();
    printHelp();

    while (true) {
        std::cout << "storage> ";
        std::string input;
        if (!std::getline(std::cin, input)) {
            break;
        }

        size_t start = input.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        size_t end = input.find_last_not_of(" \t\r\n");
        input = input.substr(start, (end - start + 1));

        if (input == "QUIT" || input == "EXIT" || input == "quit" || input == "exit") {
            break;
        } else if (input == "HELP" || input == "help" || input == "?") {
            printHelp();
            continue;
        } else if (input == "CLEAR" || input == "clear") {
            std::cout << "\033[2J\033[1;1H";
            printHeader();
            continue;
        }

        std::cout << sendCommand(input) << std::endl;
    }

    std::cout << "Exited CLI." << std::endl;
    return 0;
}
