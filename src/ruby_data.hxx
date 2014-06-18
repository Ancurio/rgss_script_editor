#ifndef RGSS_RUBY_DATA_HXX
#define RGSS_RUBY_DATA_HXX

#include <string>

#include <QVector>
#include <QHash>
#include <QString>
#include <QIODevice>

bool parseScript(std::string const& src);

struct ScriptArchive
{
  enum Format
  {
    XP,   /* Ruby 1.8 */
    VXAce /* Ruby 1.9 */
  };

  struct Script {
    int magic;
    QString name;
    QString data;

    /* Only needed in editor: an id that
     * uniquely identifies a script instance */
    int id;
  };

  /* These both throw QByteStrings containing
   * possible error messages */
  void read(QIODevice &dev);
  void write(QIODevice &dev, Format format);

  void insertScript(int idx);
  void deleteScript(int idx);

  QVector<Script> scripts;

  Script *getScriptForID(int id) { return id_hash.value(id, 0); }

  void rehashIDs();

private:
  /* Maps: script id, To: script struct */
  QHash<int, Script*> id_hash;

  int id_counter;
};

typedef QVector<ScriptArchive::Script> ScriptList;

#endif
