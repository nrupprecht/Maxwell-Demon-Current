#ifndef __LARGE_CURRENT_HPP__
#define __LARGE_CURRENT_HPP__

#include "utility.hpp"

class LargeCurrentSystem {
public:
  //! \brief Constructor, takes the number of states, and the number of particles.
  LargeCurrentSystem(int, int);

  //! \brief Destructor - we have to clean up the demon functions.
  ~LargeCurrentSystem();

  //! \brief Run the simulation for some amount of time, and record the current.
  pair<int, double> runSystem(double);

  //! \brief Run many trials of the same system, return a map of (integrated current value, number of occurences).
  pair<map<int, int>, double> gatherCurrentStatistics(int, double);

  //! \brief Compute and return the affinity of the loop.
  double getAffinity();

  void setHomogeneousRates(double=1., double=1.);

  //! \brief 
  void setDemon_Random(double=0.5);

  //! \brief 
  void setSystem_Random(double=0.5, double=1.0);

private:

  //! \brief The number of states.
  int nstates = 5;

  //! \brief The number of particles in the system.
  int nparticles = 3;

  //! \brief The maximum grid size of each demon function.
  int max_demon_function_size = 5;

  //! \brief The actual demon size.
  int demon_size;

  //! \brief A vector of the demon functions.
  vector<double**> demon_functions;

  //! \brief Positive and negative transition rates.
  vector<double> Kpos, Kneg;

  //! \brief Site occupation.
  vector<int> occupation;

  std::default_random_engine generator;
  std::exponential_distribution<float> distribution;
};

#endif // __LARGE_CURRENT_HPP__