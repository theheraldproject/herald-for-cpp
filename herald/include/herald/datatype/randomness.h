//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef RANDOMNESS_H
#define RANDOMNESS_H

#include "data.h"

#include <string>
#include <random>
#include <climits>

namespace herald {
namespace datatype {

/**
 * A Randomness Source provides random data. It could be a simple
 * uniform integer distribution, it could be a predictable sequence,
 * it could be derived from a chipsets secure enclave, if present.
 * 
 * This high level class abstracts away the underlying implementation
 * mechanics. On native platforms in C++ the implementations available may
 * vary dramatically.
 */
class RandomnessSource {
public:
  RandomnessSource() = default;
  virtual ~RandomnessSource() = default;

  virtual std::string methodName() const = 0;

  virtual void nextBytes(std::size_t count, Data& into) = 0;
  virtual int nextInt() = 0;
  virtual double nextDouble() = 0;

};

/**
 * A decidedly non random source!!! Used to test the v4 UUID format generation function only.
 * DO NOT USE IN PRODUCTION.
 */
class AllZerosNotRandom : public RandomnessSource {
public:
  AllZerosNotRandom() = default;
  ~AllZerosNotRandom() = default;

  std::string methodName() const override {
    return "allzeros";
  }

  void nextBytes(std::size_t count, Data& into) override {
    std::vector<std::byte> result;
    for (std::size_t i = 0;i < count;i++) {
      result.push_back(std::byte(0));
    }
    Data final(result);
    into.append(final);
  }

  int nextInt() override {
    return 0;
  }

  double nextDouble() override {
    return 0.0;
  }
};

class IntegerDistributedRandomSource : public RandomnessSource {
public:
  IntegerDistributedRandomSource() 
    : rd(), gen(rd()), distrib(LONG_MIN,LONG_MAX)
  {}

  ~IntegerDistributedRandomSource() = default;

  std::string methodName() const override {
    return "integerdistributed";
  }

  void nextBytes(std::size_t count, Data& into) override {
    std::vector<std::byte> result;
    for (std::size_t i = 0;i < count;i++) {
      result.push_back(std::byte(distrib(gen))); // a little wasteful...
    }
    Data final(result);
    into.append(final);
  }

  int nextInt() override {
    return (int)distrib(gen);
  }

  double nextDouble() override {
    return (double)distrib(gen);
  }

private:
  std::random_device rd;  // Will be used to obtain a seed for the random number engine
  std::mt19937 gen; // Standard mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<int64_t> distrib;
};

/**
 * The Randomness Generator IS A source of randomness, but may also
 * be used by the application to inject a secondary source of entropy
 * in to the result of the underlying RandomnessSource implementation.
 * 
 * This is the class used by Herald. This allows application developers
 * to use the most appropriate randomness source for their target
 * platform and application needs.
 * 
 * A secondary source of entropy may be something else going on in the
 * app. In Herald, for example, it could be the time it actually takes
 * to complete a scan-and-interact loop. This combines outside effects
 * of communication, internal timer/interrupt timing changes, and
 * internal state computation times to make the entropy unpredictable.
 */
class RandomnessGenerator : public RandomnessSource {
public:
  RandomnessGenerator(std::unique_ptr<RandomnessSource>&& toOwn)
    : m_source(std::move(toOwn)),
      m_entropy(0)
  {
    ;
  }

  ~RandomnessGenerator() = default;

  template <typename T>
  void addEntropy(T entropy) {
    // Get size of amount of entropy we have
    constexpr std::size_t size = sizeof(T);
    // TODO consider whether there's benefit to detecting most significant set bits.
    //      This may provide a benefit if the method of combination isn't XOR.
    
    // See if we are at multiples of std::size_t
    constexpr std::size_t multiple = sizeof(std::size_t) / size; // integer division, rounds down
    // Note the above cannot ever be 0 because std::size_t is always the largest size on a given architecture

    std::size_t toXor = 0;
    for (std::size_t i = 0;i < multiple;i++) {
      if (0 != i) {
        toXor << size;
      }
      toXor += entropy;
    }

    // XOR will ensure the same distribution of 0 and 1 over multiple applications
    m_entropy = m_entropy ^ toXor;
  }

  
  std::string methodName() const {
    return m_source->methodName();
  }

  void nextBytes(std::size_t count, Data& into) {
    constexpr std::size_t byteSize = sizeof(std::byte);
    constexpr std::size_t sizeTSize = sizeof(std::size_t);
    constexpr std::size_t shifts = sizeTSize / byteSize;
    Data sourcedInto;
    m_source->nextBytes(count,sourcedInto);

    std::vector<std::byte> result;

    // now add in entropy
    for (std::size_t byteIndex = 0;byteIndex < count;byteIndex++) {
      result.push_back((std::byte)(
        std::size_t(sourcedInto.at(byteIndex))
        ^ 
        (m_entropy >> 8 * (byteIndex % shifts))
      ));
    }

    Data final(result);
    into.append(final);
  }

  int nextInt() {
    return (int)(m_source->nextInt() ^ m_entropy);
  }

  double nextDouble() {
    return (double)(((std::size_t)m_source->nextDouble()) ^ m_entropy);
  }

private:
  std::unique_ptr<RandomnessSource> m_source;
  std::size_t m_entropy;
};

} // end namespace
} // end namespace

#endif