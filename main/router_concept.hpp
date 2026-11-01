// What the Ui hands its controllers' requests to: a callable taking a `const Request&` (the application's: it sends
// each into the queue of the service it is for).
#pragma once

#include <concepts>

#include "avela_messaging/messages/request.hpp"

namespace avela {

template <typename T>
concept RouterConcept = std::invocable<T&, const Request&>;

}  // namespace avela
