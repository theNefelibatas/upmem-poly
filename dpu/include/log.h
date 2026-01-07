#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

class Log {
public:
    static std::ostream& logfile() {
        static std::ofstream log_file = open_new_file();
        if (log_file.is_open()) { 
            return log_file;
        }

        std::cerr << "[ERROR] Unable to open log file\n";
        return std::cout;
    }

private:
    static std::ofstream open_new_file() {
        std::filesystem::create_directories("./log");

        std::time_t t = std::time(nullptr);
        std::tm* time_info = std::localtime(&t);

        std::ostringstream filename;
        filename << "./log/" << std::put_time(time_info, "%Y.%m.%d-%H.%M.%S") << ".log";

        std::ofstream ofs(filename.str(), std::ios::app);
        return ofs;
    }
};
