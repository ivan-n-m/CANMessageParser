#include <iostream>
#include <fstream>
#include <string>
#include "CANMessageParser.h"


int main() {
  std::string filename = "transcript.txt";

  std::ifstream file(filename); 
  if (!file.is_open()) {
      std::cerr << "ERROR: Can't open file: " << filename << std::endl;
      return -1;
  }

  std::string line;
  CANMessageParser canParser;
  size_t lineNumber = 0;
  while (std::getline(file, line)) {
      lineNumber++;
      if (line.empty()) continue; // skip empty lines

      try {
         canParser.proceedFrame(line);
      } 
      catch (const std::exception &e) {
         std::cerr << "Exception while processing line " << lineNumber
                   << ": " << e.what() << std::endl;
      }
  }

  file.close();
  std::cout << "Finished processing file: " << filename << std::endl;
  return 0;
}