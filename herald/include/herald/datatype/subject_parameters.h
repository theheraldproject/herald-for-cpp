//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef HERALD_SUBJECT_PARAMETERS_H
#define HERALD_SUBJECT_PARAMETERS_H

#include <cstdint>

namespace herald {
namespace datatype {

/**
 * @brief Age in completed years since birth.
 * 
 * \note I'll risk exceeding 255 years.
 */
using Age = std::uint8_t;

/**
 * @brief Mass in KG.
 */
using Mass = double;

/**
 * @brief Phenotypic Sex (not gender) as used in Clinical Medicine
 * 
 * 
 * See BMJ Advice on clinical medicine definition of sex (NOT gender): https://www.bmj.com/content/372/bmj.n735/rr-0
 * Also NHS Data Dictionary here: https://datadictionary.nhs.uk/classes/person_phenotypic_sex.html
 * \note that phenotypic sex is not the same as chromosonal sex either. Clinical medicine uses Phenotypic sex generally.
 */
using PhenotypicSex = std::uint8_t;

namespace phenotypic_sex {

static constexpr PhenotypicSex male{1};
static constexpr PhenotypicSex female{2};
static constexpr PhenotypicSex indeterminate{9};

}

}
}

#endif