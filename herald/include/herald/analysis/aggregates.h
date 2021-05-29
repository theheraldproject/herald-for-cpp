//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_AGGREGATES_H
#define HERALD_AGGREGATES_H

#include <map>
#include <variant>
#include <vector>
// #include <iostream>

#include "ranges.h"

namespace herald {
namespace analysis {
namespace aggregates {

struct Mean {
  static constexpr int runs = 1;

  Mean() : count(0), run(1), sum(0.0) {}
  ~Mean() = default;

  void beginRun(int thisRun) { // 1 indexed
    run = thisRun;
  }

  template <typename ValT>
  void map(ValT value) {
    if (run > 1) return; // performance enhancement

    sum += (double)value;
    ++count;
  }

  double reduce() {
    return sum / count;
  }

  void reset() {
    count = 0;
    sum = 0.0;
  }

private:
  int count;
  int run;
  double sum;
};

struct Mode {
  static constexpr int runs = 1;
  
  Mode() : run(1), counts() {}
  ~Mode() = default;

  void beginRun(int thisRun) { // 1 indexed
    run = thisRun;
  }

  template <typename ValT>
  void map(ValT value) {
    if (run > 1) return; // performance enhancement

    double dv = (double)value;
    auto ptr = counts.find(dv);
    if (counts.end() == ptr) {
      counts.emplace(dv,1);
    } else {
      ++(ptr->second);
    }
  }

  double reduce() {
    // loop through map once and find largest
    double largest = 0;
    int largestCount = 0;
    for (auto iter = counts.begin();iter != counts.end();++iter) {
      if (iter->second > largestCount) {
        largestCount = iter->second;
        largest = iter->first;
      }
    }
    return largest;
  }

  void reset() {
    counts.clear();
  }

private:
  int run;
  std::map<double,int> counts; // value converted to double, and int count for each
};

struct Variance {
  static constexpr int runs = 2;

  Variance() : count(0), run(1), sum(0.0), mean(0.0) {}
  ~Variance() = default;

  void beginRun(int thisRun) { // 1 indexed
    run = thisRun;
    if (2 == run) {
      // initialise mean
      mean = sum / count;
      // reinitialise counters
      sum = 0.0;
      count = 0;
    }
  }

  template <typename ValT>
  void map(ValT value) {
    double dv = (double)value;
    if (1 == run) {
      sum += dv;
    } else {
      // 2 === run
      sum += (dv - mean)*(dv - mean);
    }
    ++count;
  }

  double reduce() {
    if (count < 1) {
      return 0.0; // div by zero check
    }
    return sum / (count - 1); // Sample variance
  }

  void reset() {
    count = 0;
    run = 1;
    sum = 0.0;
    mean = 0.0;
  }

private:
  int count;
  int run;
  double sum;
  double mean;
};


struct Median {
  static constexpr int runs = 1;

  Median() : run(1), minNextPos(0), maxNextPos(0), minHeap(), maxHeap() {}
  ~Median() = default;

  void beginRun(int thisRun) {
    run = thisRun;
  }

  template <typename ValT>
  void map(ValT value) {
    if (run > 1) {
      return;
    }
    double dv = (double)value;
    if (minNextPos == maxNextPos) {
      if (10 == maxNextPos) {
        removeLeast(maxHeap,maxNextPos);
      }
      maxHeap[maxNextPos++] = dv;
      if (10 == minNextPos) {
        removeMost(minHeap,minNextPos);
      }
      minHeap[minNextPos++] = maxHeap[leastIndex(maxHeap,maxNextPos)];
    } else {
      if (10 == minNextPos) {
        removeMost(minHeap,minNextPos);
      }
      minHeap[minNextPos++] = dv;
      if (10 == maxNextPos) {
        removeLeast(maxHeap,maxNextPos);
      }
      maxHeap[maxNextPos++] = minHeap[mostIndex(minHeap,minNextPos)];
    }
  }

  double reduce() {
    if (0 == minNextPos && 0 == maxNextPos) {
      return 0.0; // empty data check
    }
    if (minNextPos > maxNextPos) {
      return minHeap[leastIndex(minHeap,minNextPos)];
    }
    return (minHeap[leastIndex(minHeap,minNextPos)] + maxHeap[mostIndex(maxHeap,maxNextPos)]) / 2.0;
  }

  void reset() {
    run = 1;
    minNextPos = 0;
    maxNextPos = 0;
  }


private:
  int run;
  int minNextPos;
  int maxNextPos;
  std::array<double,10> minHeap;
  std::array<double,10> maxHeap;

  int leastIndex(const std::array<double,10>& from, const int fromNextPos) const {
    int leastIdx = 0;
    double least = from[0];
    for (int i = 1;i < fromNextPos;++i) {
      if (from[i] < least) {
        least = from[i];
        leastIdx = i;
      }
    }
    return leastIdx;
  }

  int mostIndex(const std::array<double,10>& from, const int fromNextPos) const {
    int mostIdx = 0;
    double most = from[0];
    for (int i = 1;i < fromNextPos;++i) {
      if (from[i] > most) {
        most = from[i];
        mostIdx = i;
      }
    }
    return mostIdx;
  }

  void removeMost(std::array<double,10>& from, int& fromNextPos) {
    int mostIdx = mostIndex(from,fromNextPos);
    for (int i = mostIdx; i < fromNextPos - 1;++i) {
      from[i] = from[i + 1];
    }
    --fromNextPos;
  }

  void removeLeast(std::array<double,10>& from, int& fromNextPos) {
    int leastIdx = leastIndex(from,fromNextPos);
    for (int i = leastIdx; i < fromNextPos - 1;++i) {
      from[i] = from[i + 1];
    }
    --fromNextPos;
  }
};




/// A Variadic aggregation function requiring aggregations to be prior initialised (i.e. configured)

template <typename... Aggs>
struct aggregate {
  aggregate(Aggs... configuredAggregates) : aggregates() {
    addAggregate<Aggs...>(configuredAggregates...);
  }
  ~aggregate() = default;
  
  template <typename SampleListT,
            typename SampleT = typename std::remove_cv<typename SampleListT::value_type>::type,
            typename ValT = typename std::remove_cv<typename SampleT::value_type>::type,
            std::size_t MaxSize = SampleListT::max_size
           >
  friend auto operator|(SampleListT& from, aggregate<Aggs...> me) -> aggregate<Aggs...> {
    // determine number of runs
    int maxRuns = 1;
    for (auto& agg : me.aggregates) {
      std::visit([&maxRuns](auto&& arg) {
        if (arg.runs > maxRuns) {
          maxRuns = arg.runs;
        }
      }, agg);
    }

    // loop over all incoming data, calling each aggregate each time
    for (int run = 1;run <= maxRuns;run++) {
      for (auto& agg : me.aggregates) {
        std::visit([&run](auto&& arg) {
          arg.beginRun(run);
          // std::cout << "Beggining run " << run << std::endl;
        }, agg);
      }

      for (auto& v : from) {
        for (auto& agg : me.aggregates) {
          std::visit([&v](auto&& arg) {
            // std::cout << "Sample taken: " << v.taken.secondsSinceUnixEpoch() << std::endl;
            arg.map(v);
          }, agg);
        }
      }
    }

    // return me so we can then do get<Agg>()
    return me;
  }
  
  template <typename Coll, typename Pred>
  friend auto operator|(herald::analysis::views::filtered_iterator_proxy<Coll,Pred> from, aggregate<Aggs...> me) -> aggregate<Aggs...> {
    // determine number of runs
    int maxRuns = 1;
    for (auto& agg : me.aggregates) {
      std::visit([&maxRuns](auto&& arg) {
        if (arg.runs > maxRuns) {
          maxRuns = arg.runs;
        }
      }, agg);
    }

    // loop over all incoming data, calling each aggregate each time
    for (int run = 1;run <= maxRuns;run++) {
      for (auto& agg : me.aggregates) {
        std::visit([&run](auto&& arg) {
          arg.beginRun(run);
          // std::cout << "Beggining run " << run << std::endl;
        }, agg);
      }

      while (!from.ended()) {
        auto& v = *from;
        for (auto& agg : me.aggregates) {
          std::visit([&v](auto&& arg) {
            // std::cout << "Sample taken: " << v.taken.secondsSinceUnixEpoch() << std::endl;
            arg.map(v);
          }, agg);
        }
        ++from;
      }
    }

    // return me so we can then do get<Agg>()
    return me;
  }

  template <typename ValT>
  friend auto operator|(herald::analysis::views::view<ValT> from, aggregate<Aggs...> me) -> aggregate<Aggs...> {
    // determine number of runs
    int maxRuns = 1;
    for (auto& agg : me.aggregates) {
      std::visit([&maxRuns](auto&& arg) {
        if (arg.runs > maxRuns) {
          maxRuns = arg.runs;
        }
      }, agg);
    }

    // loop over all incoming data, calling each aggregate each time
    for (int run = 1;run <= maxRuns;run++) {
      for (auto& agg : me.aggregates) {
        std::visit([&run](auto&& arg) {
          arg.beginRun(run);
          // std::cout << "Beggining run " << run << std::endl;
        }, agg);
      }

      for (auto& v : from) {
        for (auto& agg : me.aggregates) {
          std::visit([&v](auto&& arg) {
            // std::cout << "Sample taken: " << v.taken.secondsSinceUnixEpoch() << std::endl;
            arg.map(v);
          }, agg);
        }
      }
    }

    // return me so we can then do get<Agg>()
    return me;
  }

  template <typename Agg>
  Agg& get() {
    Agg& retval = std::get<0>(aggregates.front());
    for (auto& agg : aggregates) {
      // See https://en.cppreference.com/w/cpp/utility/variant/visit
      std::visit([&retval](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<Agg,T>) {
          retval = arg;
        }
      }, agg);
    }
    return retval;
  }

private:
  std::vector<std::variant<Aggs...>> aggregates;

  template <typename Last>
  void addAggregate(Last last) {
    aggregates.emplace_back(std::move(last));
  }

  template <typename First, typename Second, typename... Remaining>
  void addAggregate(First first, Second second, Remaining... remaining) {
    aggregates.emplace_back(std::move(first));
    addAggregate<Second, Remaining...>(second, remaining...);
  }
};








/// A specialised aggregation that doesn't require pre-defining or
/// parameterising the aggregation classes being used.
/// This is a convenience version of aggregate
template <typename... Aggs>
struct summarise {
  summarise() : aggregates() {
    // Initialise members of aggregates, one per aggregate type requested
    addAggregate<Aggs...>();
  }
  ~summarise() = default;

  template <typename ValT>
  friend auto operator|(herald::analysis::views::view<ValT>& from, summarise<Aggs...> me) -> summarise<Aggs...> {
    // determine number of runs
    int maxRuns = 1;
    for (auto& agg : me.aggregates) {
      std::visit([&maxRuns](auto&& arg) {
        if (arg.runs > maxRuns) {
          maxRuns = arg.runs;
        }
      }, agg);
    }

    // loop over all incoming data, calling each aggregate each time
    for (int run = 1;run <= maxRuns;run++) {
      for (auto& agg : me.aggregates) {
        std::visit([&run](auto&& arg) {
          arg.beginRun(run);
        }, agg);
      }

      for (auto& v : from) {
        for (auto& agg : me.aggregates) {
          std::visit([&v](auto&& arg) {
            arg.map(v);
          }, agg);
        }
      }
    }

    // return me so we can then do get<Agg>()
    return me;
  }

  template <typename Agg>
  double get() {
    double result = 0.0;
    for (auto& agg : aggregates) {
      // See https://en.cppreference.com/w/cpp/utility/variant/visit
      std::visit([&result](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<Agg,T>) {
          result = arg.reduce();
        }
      }, agg);
    }
    return result;
  }

private:
  std::vector<std::variant<Aggs...>> aggregates;

  template <typename Last>
  void addAggregate() {
    aggregates.emplace_back(Last());
  }

  template <typename First, typename Second, typename... Remaining>
  void addAggregate() {
    aggregates.emplace_back(First());
    addAggregate<Second, Remaining...>();
  }
};


} // end namespace aggregates
}
}

#endif
