//  Copyright 2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/context.h"
#include "herald/engine/activities.h"
#include "herald/engine/coordinator.h"

#include <memory>

namespace herald {
namespace engine {

using namespace herald::datatype;

class Coordinator::Impl {
public:
  Impl(std::shared_ptr<Context> ctx);
  ~Impl();

  std::shared_ptr<Context> context;
};

Coordinator::Impl::Impl(std::shared_ptr<Context> ctx)
  : context(ctx)
{
  ;
}

Coordinator::Impl::~Impl()
{
  ;
}




Coordinator::Coordinator(std::shared_ptr<Context> ctx)
  : mImpl(std::make_unique<Impl>(ctx))
{
  ;
}

Coordinator::~Coordinator()
{
  ;
}


/** Introspect and include in iteration planning **/
void
Coordinator::add(std::shared_ptr<Sensor> sensor)
{

}

/** Remove from iteration planning **/
void
Coordinator::remove(std::shared_ptr<Sensor> sensor)
{

}

/** Prepares for iterations to be called (may pre-emptively make calls) **/
void
Coordinator::start()
{

}

/** Execute an iteration of activity, according to settings **/
void
Coordinator::iteration()
{

}

/** Closes out any existing connections/activities **/
void
Coordinator::stop()
{

}


}
}