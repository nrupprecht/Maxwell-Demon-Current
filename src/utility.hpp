#ifndef __UTILITY_HPP__
#define __UTILITY_HPP__

#include <fstream>
using std::ofstream;

#include "argparse.hpp"

#include <string>
using std::string;

#include <sstream>
using std::stringstream;

#include <random>
#include <chrono>
using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;

#include <iostream>
using std::cout;
using std::endl;

#include <map>
using std::map;
using std::pair;


template<typename T> inline string toString(T t) {
  stringstream stream;
  string str;
  stream << t;
  stream >> str;
  return str;
}

inline bool writeToFile(const string fileName, const map<int,int>& data, double time, int trials, double alpha=-1, double beta=0, double gamma=0, double delta=0, double kp=0, double km=0) {
  std::ofstream fout(fileName);
  if (fout.fail()) {
    cout << "Error occurred in opening \"" + fileName + "\".\n";
    return false;
  }
  else {
    // Print out parameters.
    fout << time << "," << trials << "," << alpha << "," << beta << "," << gamma << "," << delta << "," << kp << "," << km << endl;
    // Print out distribution.
    for (auto cn : data) fout << cn.first/time << "," << cn.second << "\n";
    fout.close();
    // Return success.
    return true;
  }
}

inline bool writeToFile(const string fileName, double** occupation, int occ_size) {
  // (Try to) open the file
  std::ofstream fout(fileName);
  // Check for success.
  if (fout.fail()) {
    cout << "File [" << fileName << "] failed to open.\n";
    return false;
  }
  else {
    // Print out data.
    for (int i=0; i<occ_size; ++i)
      for (int j=0; j<occ_size; ++j)
        fout << i << "," << j << "," << occupation[i][j] << endl;
    // Close file.
    fout.close();
    // Return success.
    return true;
  }
}

#endif 