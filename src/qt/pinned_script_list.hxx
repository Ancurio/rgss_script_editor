#ifndef PINNED_SCRIPT_LIST_HXX
#define PINNED_SCRIPT_LIST_HXX

#include <QSortFilterProxyModel>
#include <QSet>

#include "script_archive.hxx"

class PinnedScriptList : public QSortFilterProxyModel
{
  Q_OBJECT

public:
  PinnedScriptList(QObject *parent = 0);

  void addScript(const Script &script);
  void removeScript(const Script &script);
  void clear();
  bool isEmpty();

private:
  QSet<const Script*> scripts;

  bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
};

#endif // PINNED_SCRIPT_LIST_HXX
