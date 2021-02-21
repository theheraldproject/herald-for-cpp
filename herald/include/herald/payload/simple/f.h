//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef SIMPLE_F_H
#define SIMPLE_F_H

#include "../../datatype/data.h"

namespace herald {
namespace payload {
namespace simple {

using namespace herald::datatype;

namespace F {

static Data h(const Data& data) noexcept;

static Data t(const Data& data) noexcept;

static Data t(const Data& data, int n) noexcept;

// the name xor is reserved somehow
static Data xorData(const Data& left, const Data& right) noexcept;

}

}
}
}

#endif
