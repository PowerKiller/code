#ifndef INEXOR_RPC_SHARED_VAR_HEADER
#define INEXOR_RPC_SHARED_VAR_HEADER

#include "inexor/util/Observe.hpp"

template<typename T> using SharedVar = inexor::util::Observe<T>;

#endif
