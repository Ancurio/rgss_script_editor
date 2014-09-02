#include "pinned_script_list.hxx"

PinnedScriptList::PinnedScriptList(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

bool PinnedScriptList::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
  ScriptArchive *sa = static_cast<ScriptArchive*>(sourceModel());

  return scripts.contains(sa->indexToScript(sa->index(source_row)));
}

void PinnedScriptList::addScript(const Script &script)
{
  scripts.insert(&script);
}

void PinnedScriptList::removeScript(const Script &script)
{
  scripts.remove(&script);
}

void PinnedScriptList::clear()
{
  scripts.clear();
}

bool PinnedScriptList::isEmpty()
{
  return scripts.isEmpty();
}
