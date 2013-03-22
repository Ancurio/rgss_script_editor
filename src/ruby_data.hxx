#ifndef RGSS_RUBY_DATA_HXX
#define RGSS_RUBY_DATA_HXX

#include <string>
#include <boost/container/vector.hpp>

struct Script {
  unsigned magic;
  std::string name;
  std::string data;

  Script(unsigned const m, std::string const& n, std::string const& d);
};

typedef boost::container::vector<Script> ScriptList;

bool loadScripts(std::string const& file, ScriptList& data);
bool dumpScripts(std::string const& file, ScriptList const& data);

bool parseScript(std::string const& src);

struct RubyInstance {
  RubyInstance();
  ~RubyInstance();
};

#endif
