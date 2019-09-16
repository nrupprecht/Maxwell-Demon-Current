#include "large-current.hpp"

LargeCurrentSystem::LargeCurrentSystem(int s, int p) : nstates(s), nparticles(p) {
  // Set up random number generators.
  unsigned seed = static_cast<unsigned>(std::chrono::system_clock::now().time_since_epoch().count());
  generator = std::default_random_engine(seed);
  distribution = std::exponential_distribution<float>(1.0);

  // Create vector of demon functions.
  demon_functions = vector<double**>(nstates, nullptr);
  // Calculate demon size.
  demon_size = std::min(max_demon_function_size, nparticles+1);
  // Create initial demon function - set all rate modifiers to 1.
  for (auto &df : demon_functions) {
    df = new double*[max_demon_function_size];
    for (int i=0; i<max_demon_function_size; ++i) {
      df[i] = new double[max_demon_function_size];
      for (int j=0; j<max_demon_function_size; ++j)
        df[i][j] = 1.;
    }
  }

  // Initialize rates
  const double maxRp = 1.5, minRp = 0.5, maxRm = 1.0, minRm = 0.1;
  for (int i=0; i<nstates; ++i) {
    double kp = drand48()*(maxRp - minRp) + minRp;
    double km = drand48()*(maxRm - minRm) + minRm;
    Kpos.push_back(kp);
    Kneg.push_back(km);
  }

  // Initialize particle positions.
  occupation = vector<int>(nstates);
  for (int i=0; i<nparticles; ++i) {
    int s = drand48()*nstates;
    ++occupation[s];
  }
}

LargeCurrentSystem::~LargeCurrentSystem() {
  for (auto df : demon_functions) 
    if (df) delete df;
}

pair<int, double> LargeCurrentSystem::runSystem(double runtime) {

  double time = 0;
  auto expnum = std::bind(distribution, generator);
  int J = 0;
  double current_demon_rate = 1., last_demon_rate = 1.;
  double demon_entropy = 0;

  // Run for as long as requested.
  while (time<runtime) {

    int state = -1, dir = 0;
    double rate = 1., demon_rate = 1., event = 1., minevent = 1000000.;

    // Look through all potential rates
    for (int i=0; i<nstates; ++i) {
      // Occupation of this and the other site.
      int occ1 = occupation[i], occ2;

      // If there are no particles here, no transition from here can occur.
      if (occ1==0) continue;

      // Calculate next forward event.
      int ip1 = (i+1) % nstates;
      occ2 = occupation[ip1];
      demon_rate = (occ1<demon_size && occ2 < demon_size) ? demon_functions[i][occ1][occ2] : 1.;
      rate = Kpos[i]*demon_rate;
      // Check if the rate is the minimal rate so far.
      if (rate>0) {
        event = distribution(generator)/(occ1*rate);
        if (event < minevent) {
          minevent = event;
          current_demon_rate = demon_rate;
          state = i;
          dir = 1;
        }
      }

      // Calculate next backwards event.
      int ip2 = i==0 ? nstates-1 : i-1;
      occ2 = occupation[ip2];
      demon_rate = (occ1<demon_size && occ2 < demon_size) ? demon_functions[ip2][occ2][occ1] : 1.;

      rate = Kneg[ip2]*demon_rate;
      // Check if the rate is the minimal rate so far.
      if (rate>0) {
        event = distribution(generator)/(occ1*rate);
        if (event < minevent) {
          minevent = event;
          current_demon_rate = demon_rate;
          state = i;
          dir = -1;
        }
      }
    }

    // We have checked all rates. Check if there are any good ones.
    if (state==-1) {
      cout << "Error: no transitions available. Exiting.";
      return std::make_pair(-1, 0.);
    }

    --occupation[state];
    if (dir==1) {
      ++occupation[(state+1) % nstates];
      ++J;
    }
    else if (dir==-1) {
      int s2 = state>0 ? state-1 : nstates-1;
      ++occupation[s2];
      --J;
    }

    // Count change in entropy.
    demon_entropy += dir*log(current_demon_rate/last_demon_rate);

    // Set last demon rate.
    last_demon_rate = current_demon_rate;

    // Increment time
    time += event;
  }

  // Return the current.
  return std::make_pair(J, demon_entropy/runtime);
}

pair<map<int, int>, double> LargeCurrentSystem::gatherCurrentStatistics(int trials, double time) {
  // Count currents.
  map<int, int> counts;
  double entropy_production = 0;

  for (int i=0; i<trials; ++i) {
    // Run for the time and see what (integrated) current we get.
    auto data = runSystem(time);
    int J = data.first;
    entropy_production += data.second;
    // Record the current.
    auto it = counts.find(J);
    if (it==counts.end()) counts.insert(pair<int, int>(J, 1));
    else ++it->second;
  }

  // Return the map
  return std::make_pair(counts, entropy_production/trials);
}

double LargeCurrentSystem::getAffinity() {
  double forward = 1., reverse = 1.;
  for (int i=0; i<nstates; ++i) {
    forward *= Kpos[i];
    reverse *= Kneg[i];
  }
  return log(forward/reverse);
}

void LargeCurrentSystem::setHomogeneousRates(double kp, double km) {
  for (auto &k : Kpos) k = kp;
  for (auto &k : Kneg) k = km;
}

void LargeCurrentSystem::setDemon_Random(double min) {
  if (min==0) return;
  // Set rates.
  for (auto &df : demon_functions)
    for (int nl=0; nl<demon_size; ++nl)
      for (int nr=nl+1; nr<demon_size; ++nr)
        df[nl][nr] = drand48()*(1.-min) + min;
}

void LargeCurrentSystem::setSystem_Random(double min, double max) {
  if (min==0) return;
  if (max>min) std::swap(min, max);
  // Set rates.
  for (auto &df : demon_functions)
    for (int nl=0; nl<demon_size; ++nl)
      for (int nr=0; nr<demon_size; ++nr)
        df[nl][nr] = drand48()*(max-min) + min;
}
