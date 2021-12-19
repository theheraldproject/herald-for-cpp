//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_SUBJECT_PARAMETERS_H
#define HERALD_SUBJECT_PARAMETERS_H

#include "model.h"
#include "uuid.h"

#include <cstdint>

namespace herald {
namespace datatype {

/**
 * @brief Convenience template to allow trivial but type safe wrapping of intrinsic values
 * 
 * @tparam UnderlyingT The underlying type to wrap (E.g. std::uint8_t)
 * @tparam ShortModelClassId The unique (for the compiler) modelClassId - this prevents accidental misusing of types with the same UnderlyingT
 */
template <typename UnderlyingT, std::uint8_t ShortModelClassId>
struct Wrapped {
  /**
   * @brief Reference to the underlying type of this struct's value
   */
  using wrapped_type = UnderlyingT;
  /**
   * @brief The unique ModelClass for this data type (for the compiler)
   */
  static constexpr ModelClass modelClassId{ShortModelClassId};

  /**
   * @brief The actual underlying intrinsic type value
   */
  UnderlyingT value;

  /**
   * @brief Allows the compiler to generate a copy or move assignment operator
   * 
   * @param from The source data
   * @return auto& A reference to this wrapped instance
   */
  auto& operator=(UnderlyingT from) noexcept {
    value = from;
    return *this;
  }

  /**
   * @brief Conversion operator, allowing this struct to be used as an intrinsic type in mathematical and other operations
   * 
   * @return UnderlyingT A copy of the underlying intrinsic value
   */
  operator UnderlyingT() const {
    return value;
  }
};

/**
 * @brief Age in completed years since birth.
 * 
 * \note I'll risk exceeding 255 years.
 */
using Age = Wrapped<std::uint8_t, 5>;

/**
 * @brief Mass in KG.
 */
using Mass = Wrapped<double, 6>;

/**
 * @brief Phenotypic Sex (not gender) as used in Clinical Medicine
 * 
 * 
 * See BMJ Advice on clinical medicine definition of sex (NOT gender): https://www.bmj.com/content/372/bmj.n735/rr-0
 * Also NHS Data Dictionary here: https://datadictionary.nhs.uk/classes/person_phenotypic_sex.html
 * \note that phenotypic sex is not the same as chromosonal sex either. Clinical medicine uses Phenotypic sex generally.
 */
using PhenotypicSex = Wrapped<std::uint8_t, 7>;

namespace phenotypic_sex {

static constexpr PhenotypicSex male{1};
static constexpr PhenotypicSex female{2};
static constexpr PhenotypicSex indeterminate{9};

}

}
}

#endif