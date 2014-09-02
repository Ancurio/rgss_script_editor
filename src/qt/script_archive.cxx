#include "script_archive.hxx"

QModelIndex ScriptArchive::index(int row, int column, const QModelIndex &parent) const
{
  if (column != 0 || parent.isValid() || row < 0 || row > scripts.count() - 1)
    return QModelIndex();

  return createIndex(row, column, (void*) &scripts[row]);
}

int ScriptArchive::rowCount(const QModelIndex &parent) const
{
  if (parent.isValid())
   return 0;

  return scripts.count();
}

QVariant ScriptArchive::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();

  const Script *sc = static_cast<const Script*>(index.internalPointer());

  switch (role)
  {
  case Qt::DisplayRole :
    if (scriptNums)
      return QString("%1: %2").arg(index.row(), scriptNumDigits, 10, QChar('0')).arg(sc->name);
    else
      return sc->name;
  default:
    return QVariant();
  }
}

bool ScriptArchive::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if (!index.isValid() || role != Qt::DisplayRole)
    return false;

  indexToScript(index)->name = value.toString();
  dataChanged(index, index);

  return true;
}

bool ScriptArchive::insertRows(int row, int count, const QModelIndex &parent)
{
  /* Multi insert not implemented */
  Q_ASSERT(count == 1);

  if (parent.isValid())
    return false;

  beginInsertRows(QModelIndex(), row, row);
  scripts.insert(row, Script());
  endInsertRows();

  emitScriptCountChanged();

  return true;
}

bool ScriptArchive::removeRows(int row, int count, const QModelIndex &parent)
{
  /* Multi remove not implemented */
  Q_ASSERT(count == 1);

  if (parent.isValid())
    return false;

  beginRemoveRows(QModelIndex(), row, row);
  scripts.removeAt(row);
  endRemoveRows();

  emitScriptCountChanged();

  return true;
}

Script *ScriptArchive::indexToScript(const QModelIndex &index) const
{
  if (!index.isValid())
    return 0;

  return static_cast<Script*>(index.internalPointer());
}

void ScriptArchive::clear()
{
  if (scripts.empty())
    return;

  beginRemoveRows(QModelIndex(), 0, scripts.count()-1);
  scripts.clear();
  endRemoveRows();

  emitScriptCountChanged();
}

void ScriptArchive::setScriptNumsVisible(bool mode)
{
  scriptNums = mode;
  dataChanged(index(0), index(scriptCount()-1));
}

void ScriptArchive::setScriptList(ScriptList &list)
{
  clear();

  if (list.empty())
    return;

  beginInsertRows(QModelIndex(), 0, list.count());
  scripts = list;
  endInsertRows();

  emitScriptCountChanged();
}

void ScriptArchive::read(QIODevice &dev)
{
  ScriptList newList = readScripts(dev);

  setScriptList(newList);
}

void ScriptArchive::write(QIODevice &dev, Script::Format format)
{
  writeScripts(scripts, dev, format);
}

void ScriptArchive::emitScriptCountChanged()
{
  scriptNumDigits = qMax(QString::number(scripts.count()-1).length(), 0);
  scriptCountChanged(scripts.count());
}
