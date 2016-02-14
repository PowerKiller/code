#include "Logging.hpp"

namespace inexor {
namespace util {

InexorConsoleHandler::InexorConsoleHandler()
{
    el::Loggers::getLogger("console");
}

void InexorConsoleHandler::handle(const el::LogDispatchData* handlePtr)
{
    // NEVER LOG ANYTHING HERE!
    // 1<<0 == CON_INFO
    conline(1<<0, handlePtr->logMessage()->message().c_str());
}

}
}
