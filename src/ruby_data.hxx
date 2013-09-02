#ifndef RGSS_RUBY_DATA_HXX
#define RGSS_RUBY_DATA_HXX

#include <string>
#include <QVector>

struct Script {
  unsigned magic;
  std::string name;
  std::string data;

  Script() {}

  Script(unsigned const m, std::string const& n, std::string const& d);
};

typedef QVector<Script> ScriptList;

bool loadScripts(std::string const& file, ScriptList& data);
bool dumpScripts(std::string const& file, ScriptList const& data);

bool parseScript(std::string const& src);

struct RubyInstance {
  RubyInstance();
  ~RubyInstance();
};

#endif
