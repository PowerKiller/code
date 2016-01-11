#include <algorithm>
#include <fstream>
#include <functional>
#include <tuple>
#include <iostream>

#include <boost/spirit/include/karma.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/fusion/include/std_tuple.hpp>

#include "inexor/gluegen/protoc_generator.hpp"

namespace inexor {
namespace rpc {
namespace gluegen {

void update_protoc_file(
      const std::string &path
    , std::vector<ShTreeNode> &tree
    , const std::string &package) {

    /// Assign numbers to each of the fields

    using boost::adaptors::transformed;
    using std::function; // Finds the next index to use for not indexed fields: // The biggest used index plus one; we specifically do // not include unused index inbetween, because the could
    // be packets that have been deprecated
    // TODO: Use
    // https://github.com/ericniebler/range-v3/issues/255
    function<uint64_t(ShTreeNode&)> get_pidx = [](ShTreeNode&n) {
        return n.protoc_idx; };
    uint64_t idx_cnt = 1 + *boost::max_element( tree | transformed(get_pidx) );

    // Assign new indexes to each Node with unset index
    // TODO: Argh. Using ranges would be nicer
    for (auto &n : tree)
        if (n.protoc_idx == 0)
            n.protoc_idx = idx_cnt++;

    /// Declare the generator

    using boost::spirit::eol;
    using boost::spirit::ascii::string;
    using boost::spirit::lit;
    using boost::spirit::long_long;

    // Indentation helpers (replace with special eol,
    // indend_, outdend_ and indend[...] directives)
    auto t0 = eol;
    auto t1 = eol << "    ";

    // tuple(string type, mangled_name, index)
    auto var_gen =
        t1 << "optional " << string << " " << string
          << " = " << long_long << ";"
          << "  // " << string;

    // range[tuple]
    auto protoc_gen =
               "package " << lit(package) << ";"
      << t0
      << t0 << "message Global {"
          << *var_gen
      << t0 << "}";

    // Format the data properly
    
    using boost::adaptors::filtered;
    using std::tuple;
  
  // TODO: seriously, this syntax is terrible
    typedef tuple<std::string&, std::string&, int64_t, std::string&> ptup;
    auto mktup = [](ShTreeNode &n) {
        return ptup{
            n.protoc_lit
          , n.mangled_path
          , n.protoc_idx
          , n.cpp_var }; };
    auto data = tree | transformed(mktup);

    /// Generate and write the code
    
    using boost::spirit::karma::format;
    
    std::ofstream sink{path, std::ofstream::trunc};
    sink << format(protoc_gen, data);
}


}
}
}
