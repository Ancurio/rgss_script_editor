#ifndef RGSS_RUBY_DATA_HXX
#define RGSS_RUBY_DATA_HXX

#include <QString>
#include <QIODevice>

bool parseScript(std::string const& src);

struct Script {
  enum Format {
    XP,   /* Ruby 1.8 */
    VXAce /* Ruby 1.9 */
  };

  int magic;
  QString name;
  QString data;
};

typedef QList<Script> ScriptList;

ScriptList readScripts(QIODevice &dev);
void writeScripts(const ScriptList &scripts,
                  QIODevice &dev, Script::Format format);

#endif
