#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <vector>

#include <boost/program_options.hpp>
#include <boost/range.hpp>
#include <boost/algorithm/string/regex.hpp>

#include "inexor/gluegen/tree.hpp"
#include "inexor/gluegen/parser_cpp_shared_var.hpp"
#include "inexor/gluegen/generator_protoc.hpp"
#include "inexor/gluegen/generator_cpp_tree_adapter.hpp"

using namespace inexor::rpc::gluegen;
namespace po = boost::program_options;
using std::string;

template<typename SinglePassRange, typename Delim>
std::string join(SinglePassRange r, Delim d) {
    std::stringstream s;
    bool first = true;
    for (auto &e : r) {
        if (first)
            first = false;
        else
            s << d;
        s << e;
    }
    return s.str();
}

void usage(const std::string &ex, const po::options_description &params) {
    std::cerr
        << "NAME"
        << "\n inexor gluegen – Generate the glue code for the tree API."
        << "\n\nSYNPOSIS"
        << "\n  (1) " << ex << " -h|--help"
        << "\n  (2) " << ex << " --protoc|-P FILE --tree-adapter-source|-S FILE --tree-adapter-header|-H FILE -N|--namespace NAMESPACE -- CLANG_OPTIONS... -- SOURCE_FILES..."
        << "\n\nDESCRIPTION"
        << "\n  (1) Show this help page"
        << "\n  (2) Generate the glue code. If no options are passed to clang, you must still specify two double dashes: -- --"
        << "\n\n" << params << "\n";
}

// TODO: Boost::find should also provide the std::find
// interface
// TODO: Move this to our own algorithm library?
using std::find;
using boost::find;

int main(int argc, const char **argv) {

    // Declare our variables

    po::variables_map cli_config;
    po::options_description params("PARAMETERS");
    params.add_options()
        ("help,h", "Print this help message")
        ("protoc,P", po::value<string>(), "The .proto file to write the protocol description to.")
        ("tree-adapter-header,H", po::value<string>(), "The header `.hpp` filethe c++ tree adapter code should be generated in")
        ("tree-adapter-source,S", po::value<string>(), "The source `.cpp` file the c++ tree adapter code should be generated in")
        ("namespace,N", po::value<string>(), "The namespace to use in the generated protocol file and c++ source files. (use C++ :: notation)");

    std::string exec{argv[0]};

    // Split the parameters into three basic -- delimited
    // blocks

    typedef std::vector<string> args_t;
    typedef typename args_t::iterator args_itr;

    // TODO: Use a generalized variant of tokenized for this?
    // Argc = number of relevant args + 1, hence
    // (argv+1)+(argc-1) = argv+argv
    args_t args(argv+1, argv+argc-1);
    auto div1 = find(args, "--");
    auto div2 = find(div1+1, args.end(), "--");

    if (div1 == args.end() || div2 == args.end()) {
        usage(exec, params);
        return 1;
    }

    typedef boost::iterator_range<args_itr> argsub_t;
    auto our_opts       = argsub_t{ args.begin(), div1 };
    auto compiler_opts  = argsub_t{ div1 + 1, div2 };
    auto compiler_files = argsub_t{ div2 + 1, args.end() };

    // Parse our own parameters

    try {
      po::store(
          // Note: Using the original char** here because
          // program options won't take iterators or ranges.
          // Those would be better though
          po::command_line_parser(our_opts.size()+1, argv)
            .options(params)
            .run()
        , cli_config);
      po::notify(cli_config);
    } catch(po::error &e) {
      std::cerr << "Failed to parse the options: "
        << e.what() << "\n\n";
      usage(exec, params);
      return 1;
    }

    auto c = [&cli_config](const std::string &s) {
        return cli_config.count(s);
    };

    if ( c("help") || !c("protoc") || !c("namespace") || !c("tree-adapter-header") || !c("tree-adapter-source")) {
      usage(exec, params);
      return 0;
    }

    using boost::regex;
    using boost::split_regex;

    const auto &protoc_file = cli_config["protoc"].as<std::string>();
    const auto &hpp_file = cli_config["tree-adapter-header"].as<std::string>();
    const auto &cpp_file = cli_config["tree-adapter-source"].as<std::string>();
    const auto &ns_str = cli_config["namespace"].as<std::string>();

    std::vector<string> ns;
    split_regex(ns, ns_str, regex("::"));
    const std::string &protoc_pkg = join(ns, '.');

    // Read the list of variables

    ShTree tree;

    struct {
        ShTree &tree;
        void shared_var(string cpp_type, string cpp_var, string tree_path) {
            tree.emplace_back(cpp_type, std::move(cpp_var), std::move(tree_path));
        }
    } visitor{tree};

    find_shared_decls(compiler_opts, compiler_files, visitor);

    // Write the protoc file
    update_protoc_file(protoc_file, tree, protoc_pkg);

    // Write cpp files
    update_cpp_tree_server_impl(hpp_file, cpp_file, tree, ns);

    return 0;
}
