
#include "config.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>
namespace prsi {

Config::Config() {}

const std::unordered_map<std::string, Config::Setter> Config::setters_ = {
    {"IP", &Config::ip}, {"PORT", &Config::port}, {"EME", &Config::eme},
    {"ET", &Config::et}, {"MC", &Config::mc},     {"PT", &Config::pt},
    {"ST", &Config::st}, {"DT", &Config::dt},     {"MR", &Config::mr}};

Config::Config(const std::string &filename) {
  // open file
  std::ifstream file(filename);
  if (!file)
    throw std::runtime_error("Failed to open config file");

  // for each line
  std::string line;
  while (std::getline(file, line)) {
    std::istringstream iss(line);
    std::string key, value;
    // split string into key & value
    if (!(iss >> key >> value))
      continue;

    key = to_upper(key);

    // find what to set by this
    auto it = setters_.find(key);
    if (it != setters_.end()) {
      (this->*(it->second))(value); // set the value

    } else {
      std::cerr << "Config loading: Unknown key in line: '" << line << "'."
                << std::endl;
    }
  }
}

} // namespace prsi
