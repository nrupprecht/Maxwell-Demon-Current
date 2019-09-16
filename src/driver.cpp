#include "current.hpp"
#include "large-current.hpp"
// For mkdir
#include <sys/stat.h>

int main(int argc, char **argv) {
  // Create a random seed.
  unsigned seed = static_cast<unsigned>(std::chrono::system_clock::now().time_since_epoch().count());

  // Get command line arguments.
  ArgParse parser(argc, argv);
  // Check if there is a specific seed.
  parser.get("seed", seed);

  // Seed random number generator.
  srand48(seed);

  // Parameters. Create after seeding random number generator.
  double alpha = 1.+2.*drand48(), beta = 1.+2.*drand48(), gamma = 1.+2.*drand48(), delta = 1.+2.*drand48();
  double kp = 1.+2.*drand48(), km=1+2.*drand48();
  int trials = 100000;
  int slices = 21;
  double time = 1000.;
  int numsys = 20;
  string save = "data.csv";
  string directory = "demon";

  // Get the rest of the parameters.
  parser.get("alpha", alpha);
  parser.get("beta", beta);
  parser.get("gamma", gamma);
  parser.get("delta", delta);
  parser.get("kp", kp);
  parser.get("km", km);
  parser.get("trials", trials);
  parser.get("slices", slices);
  parser.get("time", time);
  parser.get("numsys", numsys);
  parser.get("save", save);
  parser.get("directory", directory);

  // Print message to screen.
  // cout << "Seed: " << seed << "\n";
  // cout << "Params: " << alpha << ", " << beta << ", " << gamma << ", " << delta << "; " << kp << ", " << km << "\n";

  // A current system.
  CurrentSystem system;
  system.setAllParams(alpha, beta, gamma, delta, kp, km);

  // Start timing.
  auto start_time = high_resolution_clock::now();

  // Create directory
  mkdir(directory.c_str(), 0777);
  cout << "Created directory [" << directory << "].\n";
  
  // Slowly vary demon parameters.
  if (false) {
    // Parameters for advancing demon.
    double initialRate=1., finalRate=0.;
    double dr = (finalRate-initialRate)/static_cast<double>(slices-1);
    // Run a number of slices, adjusting the rates of the non-prefered transitions.
    for (int i=0; i<slices; ++i) {
      auto start_slice = high_resolution_clock::now();
      double rate = initialRate + i*dr;
      cout << "Setting demon to " << rate << ".\n";
      system.setDemon_GreaterThan(rate);
      cout << "Running... ";
      auto data = system.gatherCurrentStatistics(trials, time);
      string save_name = "incremental-data-" + toString(i) + ".csv";
      auto end_slice = high_resolution_clock::now();
      cout << "Done. Saving to [" << save_name << "]. Slice time was " << duration_cast<duration<double> >(end_slice-start_slice).count() << ".\n";
      writeToFile(save_name, data, time, trials, alpha, beta, gamma, delta, kp, km);
    }
  }


  // LargeCurrentSystem mysys(5, 1);
  // mysys.setHomogeneousRates(1.1, 0.9);
  // mysys.setDemon_Random(0.1);
  // auto data = mysys.runSystem(time);
  // cout << "Affinity: " << mysys.getAffinity() << endl;
  // cout << data.first << ", " << data.second << endl;

  if (true) {
    for (int I=0; I<numsys; ++I) {
      LargeCurrentSystem largeSystem(5, 5);
      double affinity = largeSystem.getAffinity();
      // Gather statistics.
      auto data1 = largeSystem.gatherCurrentStatistics(trials, time);
      // Randomize demon
      //largeSystem.setSystem_Random(0.5, 2.);
      largeSystem.setDemon_Random(0.1);
      // Gather statistics.
      auto data2 = largeSystem.gatherCurrentStatistics(trials, time);
      // Create directory
      string dir = directory + "/" + directory + "-" + toString(I) + "/";
      mkdir(dir.c_str(), 0777);
      // Write data to files.
      writeToFile(dir+"data1.csv", data1.first, time, trials, affinity, data2.second);
      writeToFile(dir+"data2.csv", data2.first, time, trials, affinity, data2.second);

      cout << "Done with run " << I << ".\n";
    }
  }
  
  // Small demon statistics.
  if (false) {
    // We want to record the state occupation.
    system.setRecordOccupation(true);

    // Run system without demon.
    auto data1 = system.gatherCurrentStatistics(trials, time);
    int occ_size = system.getOccSize();
    double **occupation = system.getOccupation();
    writeToFile(directory+"data1-occ.csv", occupation, occ_size);

    // Reset and set demon.
    //system.setDemon_Random(0.1);
    system.setSystem_Random(0.5,1.5);
    system.clearOccupation();

    // Run system with demon.
    auto data2 = system.gatherCurrentStatistics(trials, time);
    occupation = system.getOccupation();
    writeToFile(directory+"data2-occ.csv", occupation, occ_size);

    // Write demon function to file.
    int demon_size = system.getDemonSize();
    double **demon = system.getDemonFunction();
    writeToFile(directory+"demon-function.csv", demon, demon_size);
    
    // Write current data to files.
    writeToFile(directory+"data1.csv", data1, time, trials, alpha, beta, gamma, delta, kp, km);
    writeToFile(directory+"data2.csv", data2, time, trials, alpha, beta, gamma, delta, kp, km);
  }

  // End timing.
  auto end_time = high_resolution_clock::now();
  duration<double> span = duration_cast<duration<double> >(end_time-start_time);
  cout << "Finished running. Time: " << span.count() << ".\n";

  return 0;
}
