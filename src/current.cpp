#include "current.hpp"

CurrentSystem::CurrentSystem() {
  unsigned seed = static_cast<unsigned>(std::chrono::system_clock::now().time_since_epoch().count());
  generator = std::default_random_engine(seed);
  distribution = std::exponential_distribution<float>(1.0);

  double *occ = new double[occ_size*occ_size];
  occupation = new double*[occ_size];
  for (int i=0; i<occ_size; ++i) occupation[i] = &occ[i*occ_size];
  // Allocate demon function
  double *dem = new double[demon_size*demon_size];
  demon_function = new double*[demon_size];
  for (int i=0; i<demon_size; ++i) demon_function[i] = &dem[i*demon_size];
  // Initialize to 1.
  for (int i=0; i<demon_size*demon_size; ++i) dem[i] = 1.;
}

CurrentSystem::~CurrentSystem() {
  if (occupation) {
    double *occ = occupation[0];
    delete [] occ;
    delete [] occupation;
  }
  if (demon_function) {
    double *dem = demon_function[0];
    delete [] demon_function;
  }
}

int CurrentSystem::getCurrent(double runtime) {
  int nl = 0, nr = 0, J = 0;
  double time = 0;
  auto expnum = std::bind(distribution, generator);
  
  // Run until time is done.
  while (time<runtime) {
    int type = 0;

    // Get the demon rate factor.
    double demon_rate = (nl<demon_size && nr<demon_size) ? demon_function[nl][nr] : 1.;

    // Inverse rate. We use 100000 as "effectively infinite."
    double demon_scale = (demon_rate<=0) ? 100000 : 1./demon_rate;

    // System -> left site, System -> right site
    double dt = expnum()/alpha, ndt = distribution(generator)/delta;
    if (ndt<dt) {
      type = 1;
      dt = ndt;
    }
    if (nl>0) {
      // Left site -> system.
      ndt = distribution(generator)/(nl*gamma);
      if (ndt<dt) {
        type = 2;
        dt = ndt;
      }
      // Left site -> right site
      ndt = distribution(generator)/(nl*kp) * demon_scale;
      if (ndt<dt) {
        type = 4;
        dt = ndt;
      }
    }
    if (nr>0) {
      // Right site -> system (destruction).
      ndt = distribution(generator)/(nr*beta);
      if (ndt<dt) {
        type = 3;
        dt = ndt;
      }
      // Right site -> left site.
      ndt = distribution(generator)/(nr*km) * demon_scale;
      if (ndt<dt) {
        type = 5;
        dt = ndt;
      }
    }

    // Increment occupation.
    //dt = dt<time - runtime ? dt : time - runtime; 
    if (record_occupation && nl<occ_size && nr<occ_size) occupation[nl][nr] += dt;

    // Increment time.
    time += dt;

    // If the next event happens after the simulation is done, just return.
    if (runtime <= time);
    else {
      // Enact transition.
      switch (type) {
        case 0: {
          ++nl;
          break;
        }
        case 1: {
          ++nr;
          break;
        }
        case 2: {
          --nl;
          break;
        }
        case 3: {
          --nr;
          break;
        }
        case 4: {
          --nl;
          ++nr;
          ++J;
          break;
        }
        case 5: {
          ++nl;
          --nr;
          --J;
          break;
        } 
      }
    }
  }

  // Return the current.
  return J;
}

map<int, int> CurrentSystem::gatherCurrentStatistics(int trials, double time) {
  // Count currents.
  map<int, int> counts;

  for (int i=0; i<trials; ++i) {
    // Run for the time and see what (integrated) current we get.
    int J = getCurrent(time);
    // Record the current.
    auto it = counts.find(J);
    if (it==counts.end()) counts.insert(pair<int, int>(J, 1));
    else ++it->second;
  }

  // Return the map
  return counts;
}

void CurrentSystem::setAllParams(double a, double b, double g, double d, double k, double K) {
  set_alpha(a);
  set_beta(b);
  set_gamma(g);
  set_delta(d);
  set_kp(k);
  set_km(K);
}

void CurrentSystem::clearOccupation() {
  for (int i=0; i<occ_size; ++i)
    for (int j=0; j<occ_size; ++j)
      occupation[i][j] = 0;
}

void CurrentSystem::setRecordOccupation(bool r) {
  record_occupation = r;
}

void CurrentSystem::setDemon_GreaterThan(double slow_rate) {
  // Rate cannot be zero.
  if (slow_rate==0) return;
  // Set rates.
  for (int nl=0; nl<demon_size; ++nl)
    for (int nr=nl+1; nr<demon_size; ++nr)
      demon_function[nl][nr] = slow_rate; 
}

void CurrentSystem::setDemon_Random(double min) {
  // Rate cannot be zero.
  if (min==0) return;
  // Set rates.
  for (int nl=0; nl<demon_size; ++nl)
    for (int nr=nl+1; nr<demon_size; ++nr)
      demon_function[nl][nr] = drand48()*(1-min) + min;
}

void CurrentSystem::setSystem_Random(double min, double max) {
  if (min==0) return;
  // Set rates.
  for (int nl=0; nl<demon_size; ++nl)
    for (int nr=0; nr<demon_size; ++nr)
      demon_function[nl][nr] = drand48()*(max-min) + min;
}

void CurrentSystem::setDemonFunctionEntry(int l, int r, double d) {
  if (l<demon_size && r<demon_size) demon_function[l][r] = d;
}
