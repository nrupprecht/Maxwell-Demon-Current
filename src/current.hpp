#ifndef __CURRENT_HPP__
#define __CURRENT_HPP__

#include "utility.hpp"

class CurrentSystem {
public:
  CurrentSystem();
  ~CurrentSystem();

  //! \brief Run the simulation for a fixed amount of time, return the current.
  int getCurrent(double);

  //! \brief Run many trials of the same system, return a map of (integrated current value, number of occurences).
  map<int, int> gatherCurrentStatistics(int, double);

  void set_alpha(double a)  { alpha = a; }
  void set_beta(double b)   { beta = b; }
  void set_gamma(double g)  { gamma = g; }
  void set_delta(double  d) { delta = d; }
  void set_kp(double k)     { kp = k; }
  void set_km(double k)     { km = k; }
  void setAllParams(double, double, double, double, double, double);

  int getOccSize() const            { return occ_size; }
  int getDemonSize() const          { return demon_size; }
  double** getOccupation() const    { return occupation; }
  double** getDemonFunction() const { return demon_function; }

  //! \brief Clear the occupation array.
  void clearOccupation();

  void setRecordOccupation(bool);

  //! \brief Set all the entries of the demon function where nr>nl to be the given (inverse) rate.
  void setDemon_GreaterThan(double=0);

  //! \brief Set all the entries of the demon functions where nr>nl to be a random number between min and 1.
  void setDemon_Random(double=0.5);

  //! \brief Set all entries of the demon functions to be random numbers between min and max.
  void setSystem_Random(double=0.5, double=1.0);

  //! \brief Set a single entry of the demon function.
  void setDemonFunctionEntry(int, int, double);
  
private:
  double alpha = 1., beta = 1., gamma = 1., delta = 1.;
  double kp = 1., km = 2.;

  //! \brief The highest occupation number to track.
  int occ_size = 10;
  double **occupation = nullptr;
  bool record_occupation = false;

  //! \brief The highest entry subject to a demon function.
  int demon_size = 10;
  //! \brief The demon function is the INVERSE RATES.
  double **demon_function = nullptr;

  std::default_random_engine generator;
  std::exponential_distribution<float> distribution;
  
};

#endif // __CURRENT_HPP__
