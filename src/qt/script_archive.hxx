#ifndef SCRIPT_ARCHIVE_HXX
#define SCRIPT_ARCHIVE_HXX

#include <QAbstractListModel>
#include "ruby_data.hxx"

class ScriptArchive : public QAbstractListModel
{
  Q_OBJECT

public:
  ScriptArchive()
    : scriptNums(false),
      scriptNumDigits(0)
  {}

  /* These both throw QByteStrings containing
   * possible error messages */
  void read(QIODevice &dev);
  void write(QIODevice &dev, Script::Format format);

  void clear();

  ScriptList scriptList() { return scripts; }
  void setScriptList(ScriptList &list);

  /* QAbstractListModel */
  QModelIndex index(int row, int column, const QModelIndex &parent) const;
  QModelIndex index(int row) { return index(row, 0, QModelIndex()); }
  int rowCount(const QModelIndex &parent) const;
  QVariant data(const QModelIndex &index, int role) const;
  bool setData(const QModelIndex &index, const QVariant &value, int role);
  bool insertRows(int row, int count, const QModelIndex &parent);
  bool removeRows(int row, int count, const QModelIndex &parent);

  Script *indexToScript(const QModelIndex &index) const;
  int scriptCount() const { return scripts.count(); }

  bool scriptNumsVisible() { return scriptNums; }

public slots:
  void setScriptNumsVisible(bool mode);

signals:
  void scriptCountChanged(int newCount);

private:
  ScriptList scripts;
  bool scriptNums;
  int scriptNumDigits;

  void emitScriptCountChanged();
};

#endif
