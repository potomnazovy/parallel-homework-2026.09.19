#include <iostream>
#include <cstddef>
#include <random>
#include <vector>
#include <cstring>
#include <cmath>
#include <pthread.h>

namespace
{
  bool isInside(double x, double y, double r)
  {
    return x * x + y * y <= r * r;
  }

  size_t calc(double r, size_t tests, size_t seed)
  {
    size_t count = 0;
    std::default_random_engine engine(seed);
    std::uniform_real_distribution< double > distribution(-r, r);

    for (size_t i = 0; i < tests; ++i)
    {
      double x = distribution(engine);
      double y = distribution(engine);

      if (isInside(x, y, r))
      {
        ++count;
      }
    }

    return count;
  }

  struct Thread
  {
    double r;
    size_t tests;
    size_t seed;
  };

  void* pthreadFunc(void* data)
  {
    auto param = static_cast< Thread* >(data);
    size_t res = calc(param->r, param->tests, param->seed);
    return reinterpret_cast< void* >(res);
  }

  double area(double r, size_t threads, size_t tests)
  {
    if (threads == 0 || tests == 0)
    {
      std::cerr << "Error: threads and tests must be > 0" << '\n';
      return -1.0;
    }

    size_t chunk = tests / threads;
    size_t remainder = tests % threads;

    std::vector< pthread_t > th(threads);
    std::vector< Thread > pthread(threads);

    size_t count_th = 0;
    size_t sum = 0;

    bool has_error = false;

    for (size_t i = 0; i < threads; ++i)
    {
      pthread[i].seed = i + 1;
      pthread[i].r = r;
      pthread[i].tests = (i == threads - 1) ? chunk + remainder : chunk;

      int err = pthread_create(&th[i], nullptr, pthreadFunc, &pthread[i]);
      if (err)
      {
        std::cerr << strerror(err) << '\n';
        has_error = true;
        break;
      }

      ++count_th;
    }

    for (size_t i = 0; i < count_th; ++i)
    {
      void* res = nullptr;
      int err = pthread_join(th[i], &res);

      if (err)
      {
        std::cerr << strerror(err) << '\n';
      }
      else if (!has_error)
      {
        sum += reinterpret_cast< size_t >(res);
      }
    }

    if (has_error)
    {
      return -1.0;
    }

    return (static_cast< double >(sum) / tests) * 4 * r * r;
  }
}

int main()
{
  double r = 5.0;
  size_t threads = 16;
  size_t tests = 10000000;

  std::cout << "Calculating area of circle with radius = " << r << '\n';
  std::cout << "Using " << threads << " threads and " << tests << " tests" << '\n';

  double result = area(r, threads, tests);

  if (result < 0)
  {
    std::cerr << "Error: area calculation failed" << '\n';
    return 1;
  }

  std::cout << "Monte-Carlo area: " << result << '\n';
  std::cout << "Simple area: " << M_PI * r * r << '\n';

  std::cout << "\n--- Test zero cases ---\n";
  std::cout << "Testing with 0 threads: " << area(5.0, 0, 1000) << '\n';
  std::cout << "Testing with 0 tests: " << area(5.0, 4, 0) << '\n';

  return 0;
}
