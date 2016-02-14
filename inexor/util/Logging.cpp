namespace inexor {
namespace util {

    // wrapper/callback für das logging-zeug der jedesmal conline( bei bestimmten verbositätsstufen aufruft

    // refactoring: conoutf existiert 2x: in server.cpp und in main.cpp -> nur noch 1x in logger.cpp

    // colors: conoutf


//    class LogHandler : public el::LogDispatchCallback {
//    public:
//        void handle(const el::LogDispatchData* data) {
//            // NEVER LOG ANYTHING HERE! NOT HAPPY WITH MULTI_THREADING
//            ELPP_COUT << "Test this " << data << std::endl;
//        }
//    };
//
//    class HtmlHandler : public el::LogDispatchCallback {
//    public:
//        HtmlHandler() {
//            el::Loggers::getLogger("html");
//        }
//        void handle(const el::LogDispatchData* data) {
//            // NEVER LOG ANYTHING HERE! NOT HAPPY WITH MULTI_THREADING
//            ELPP_COUT << "<b>" << data->logMessage()->message() << "</b>" << std::endl;
//        }
//    };

}
}
