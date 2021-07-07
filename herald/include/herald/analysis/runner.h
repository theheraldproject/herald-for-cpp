//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_ANALYSIS_RUNNER_H
#define HERALD_ANALYSIS_RUNNER_H

#include "sampling.h"

#include <variant>
#include <array>

// debug only
// #include <iostream>

namespace herald {
namespace analysis {

using namespace sampling;

/// \brief Manages a set of lists for a particular Sample Value Type
template <typename ValT, std::size_t Size>
struct ListManager {
  using value_type = ValT;
  static constexpr std::size_t max_size = Size;

  ListManager() = default;
  ~ListManager() = default;

  SampleList<Sample<ValT>,Size>& list(const SampledID sampled) {
    auto iter = lists.try_emplace(sampled).first;
    return lists.at(sampled);
  }

  void remove(const SampledID listFor) {
    lists.erase(listFor);
  }

  const std::size_t size() const {
    return lists.size();
  }

  decltype(auto) begin() {
    return lists.begin();
  }

  decltype(auto) end() {
    return lists.end();
  }

private:
  std::map<SampledID,SampleList<Sample<ValT>,Size>> lists;
};

/// \brief A fixed size set that holds exactly one instance of the std::variant for each
/// of the specified ValTs value types.
template <typename... ValTs>
struct VariantSet {
  static constexpr std::size_t Size = sizeof...(ValTs);

  VariantSet() : variants() {
    createInstances<ValTs...>(0);
  }; // Instantiate each type instance in the array
  ~VariantSet() = default;

  /// CAN THROW std::bad_variant_access
  template <typename ValT>
  ValT& get() {
    for (auto& v : variants) {
      if (auto pval = std::get_if<ValT>(&v)) {
        return *pval;
      }
    }
    throw std::bad_variant_access();
  }

  const std::size_t size() const {
    return variants.size();
  }

  decltype(auto) begin() {
    return variants.begin();
  }

  decltype(auto) end() {
    return variants.end();
  }

private:
  std::array<std::variant<ValTs...>,Size> variants;
  template <typename LastT>
  void createInstances(int pos) {
    variants[pos].template emplace<LastT>();
  }

  template <typename FirstT, typename SecondT, typename... RestT>
  void createInstances(int pos) {
    variants[pos].template emplace<FirstT>();
    createInstances<SecondT,RestT...>(pos + 1);
  }
};

/// \brief Convenience wrapper for all AnalysisDelegate types used by the analysis API
template <typename... DelegateTypes>
struct AnalysisDelegateManager {
  AnalysisDelegateManager(DelegateTypes... dts) : delegates() {
    addDelegates(0,dts...);
  }
  ~AnalysisDelegateManager() = default;

  template <typename ValT>
  void notify(SampledID sampled, Sample<ValT> sample) {
    for (auto& delegateV : delegates) {
      std::visit([sampled,sample](auto&& arg) {
        using noref = typename std::remove_reference<decltype(arg)>::type;
        if constexpr (std::is_same_v<ValT,typename noref::value_type>) {
          ((decltype(arg))arg).newSample(sampled,sample); // cast to call derived class function
        }
      }, delegateV);
    }
  }

  /// CAN THROW std::bad_variant_access
  template <typename DelegateT>
  DelegateT& get() {
    for (auto& v : delegates) {
      if (auto pval = std::get_if<DelegateT>(&v)) {
        return *pval;
      }
    }
    throw std::bad_variant_access();
  }

private:
  std::array<std::variant<DelegateTypes...>,sizeof...(DelegateTypes)> delegates;

  template <typename LastT>
  constexpr void addDelegates(int nextPos,LastT&& last) {
    delegates[nextPos] = (std::move(last));
  }

  template <typename FirstT, typename SecondT, typename... RestT>
  constexpr void addDelegates(int nextPos,FirstT&& first, SecondT&& second, RestT&&... rest) {
    delegates[nextPos] = std::move(first);
    ++nextPos;
    addDelegates(nextPos,second,rest...);
  }
};

/// \brief Convenience wrapper for all AnalysisProvider types used by the analysis API
template <typename... ProviderTypes>
struct AnalysisProviderManager {
  AnalysisProviderManager(ProviderTypes... prvs) : providers() {
    addProviders(0, prvs...);
  }
  ~AnalysisProviderManager() = default;

  template <typename InputValT, std::size_t SrcSz, typename SourceType, std::size_t ListSize, typename CallableForNewSample>
  bool analyse(Date timeNow, SampledID sampled, SampleList<Sample<InputValT>,SrcSz>& src, ListManager<SourceType,ListSize>& lists, CallableForNewSample& callable) {
    bool generated = false;
    for (auto& providerV : providers) {
      std::visit([&timeNow,&sampled,&src,&lists,&generated,&callable](auto&& arg) {
        using noref = typename std::remove_reference<decltype(arg)>::type;
        // Ensure our calee supports the types we have
        if constexpr (std::is_same_v<InputValT, typename noref::input_value_type>) {
          auto& listRef = lists.list(sampled);
          generated = generated | ((decltype(arg))arg).analyse(timeNow,sampled,src,listRef,callable);
        }
      }, providerV);
    }
    return generated;
  }

  /// CAN THROW std::bad_variant_access
  template <typename ProviderT>
  ProviderT& get() {
    for (auto& v : providers) {
      if (auto pval = std::get_if<ProviderT>(&v)) {
        return *pval;
      }
    }
    throw std::bad_variant_access();
  }

  template <typename InputT,typename OutputT>
  constexpr bool hasMatchingAnalyser() noexcept {
    bool match = false;
    for (auto& providerV : providers) {
      std::visit([&match] (auto&& provider) {
        using InputValT = typename InputT::value_type;
        using InT = typename std::remove_reference_t<decltype(provider)>::input_value_type;
        using OutT = typename std::remove_reference_t<decltype(provider)>::output_value_type;
        // std::cout << "  Provider being checked " << typeid(provider).name() << std::endl;
        // InT inInstance;
        // OutT outInstance;
        // std::cout << "    In type " << typeid(inInstance).name() << ", out type " << typeid(outInstance).name() << std::endl;
        // InputValT inputInstance;
        // OutputT outputInstance;
        // std::cout << "    Input type " << typeid(inputInstance).name() << ", output type " << typeid(outputInstance).name() << std::endl;
        if constexpr (std::is_same_v<InputValT,InT> && std::is_same_v<OutputT,OutT>) {
          match = true;
          // std::cout << "  MATCHED!" << std::endl;
        }
      }, providerV);
    }
    return match;
  }

private:
  std::array<std::variant<ProviderTypes...>,sizeof...(ProviderTypes)> providers;

  template <typename LastT>
  constexpr void addProviders(int nextPos, LastT&& last) {
    providers[nextPos] = std::move(last);
  }

  template <typename FirstT, typename SecondT, typename... RestT>
  constexpr void addProviders(int nextPos, FirstT&& first, SecondT&& second, RestT&&... rest) {
    providers[nextPos] = std::move(first);
    ++nextPos;
    addProviders(nextPos,second,rest...);
  }
};

/// \brief Manages all sample lists, sources, sinks, and analysis instances for all data generated within a system
///
/// This class can be used 'live' against real sensors, or statically with reference data. 
/// This is achieved by ensuring the run(Date) method takes in the Date for the time of evaluation rather
/// than using the current Date.
template <typename AnalysisDelegateManagerT, typename AnalysisProviderManagerT, typename... SourceTypes> // TODO derive SourceTypes from providers and delegates // TODO parameterise type lengths somehow (traits template?)
struct AnalysisRunner {
  static constexpr std::size_t ListSize = 25; // TODO make this external somehow for each type (trait?)
  // using valueTypes = (typename SourceTypes::value_type)...;

  AnalysisRunner(AnalysisDelegateManagerT& adm, AnalysisProviderManagerT& provds) : lists(), delegates(adm), runners(provds) /*, hasNewData(false)*/ {}
  ~AnalysisRunner() = default;

  /// We are an analysis delegate ourselves - this is used by Source types, and by producers (analysis runners)
  template <typename ValT>
  void newSample(SampledID sampled, sampling::Sample<ValT> sample) {
    // incoming sample. Pass to correct list
    lists.template get<ListManager<ValT,ListSize>>().list(sampled).push(sample); // TODO get ListSize dynamically
    // inform delegates
    delegates.notify(sampled,sample);
  }

  template <typename ValT>
  void operator()(SampledID sampled,sampling::Sample<ValT> sample) {
    newSample(sampled,sample);
  }

  /// Run the relevant analyses given the current time point
  void run(Date timeNow) {
    // call analyse(dateNow,srcList,dstDelegate) for all delegates with the correct list each, for each sampled
    
    // DO NOT USE Performance enhancement - 'dirty' sample lists only (ones with new data)
    // The reason this is commented out is because for some analysers producing a new value based on no new data may be valid.
    // if (!hasNewData) {
    //   // This also prevents 'new' conversions even if no new data has arrived, skewing analysis results
    //   return;
    // }
    for (auto& listManager : lists) { // For each input list
      std::visit([timeNow,this] (auto&& arg) { // Visit each of our list managers (that may be used as an output list)
        for (auto& mgrPair : arg) { // For each output list instance // arg = ListManager<SampleList<InputValueT>,SrcSz>
          // Derived Input type and size from 'list' input list
          
          auto& sampled = mgrPair.first;
          auto& list = mgrPair.second;

          for (auto& outputListManagerV : lists) {
            std::visit([timeNow, &list, &sampled, this] (auto&& outputListManager) { // Visit each of our list managers (that may be used as an output list)
              using InputValT = typename std::remove_reference_t<decltype(list)>::value_type;
              using LMValT = typename std::remove_reference_t<decltype(outputListManager)>::value_type;
              // std::cout << "Trying to call analysers for source type " << typeid(list).name() << " to output type " << typeid(outputListManager).name() << std::endl;

              // Check for presence of an analyser that converts from InputValT to LMValT
              if (runners.template hasMatchingAnalyser<InputValT,LMValT>()) {
                // std::cout << "Found matching analyser!" << std::endl;
                /*bool newDataGenerated =*/ runners.template analyse(timeNow,sampled,list,outputListManager, *this); // <InputValT,InputSz,LMValT,LMSz>
              }
            }, outputListManagerV);
          }
        }
      }, listManager);
    }
    // hasNewData = false;
  }

private:
  // TODO make sizes a parameterised list derived from template parameters
  VariantSet<ListManager<SourceTypes,ListSize>...> lists; // exactly one list manager per value type
  AnalysisDelegateManagerT& delegates;
  AnalysisProviderManagerT& runners;
  // bool hasNewData;
};

}
}

#endif