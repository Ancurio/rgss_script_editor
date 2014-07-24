
#include "ruby_data.hxx"

#include <QtCore/QFileInfo>
#include <QtGui/QApplication>
#include <QtCore/QTextCodec>
#include <QtCore/QTextStream>

#include <iostream>

using namespace std;

int saveArchive(QString const& file, ScriptArchive &archive) {
  /* Determine marshal format */
  QFileInfo finfo(file);
  QString fileExt = finfo.suffix();
  ScriptArchive::Format format;

  if (fileExt == "rxdata")
    format = ScriptArchive::XP;
  else if (fileExt == "rvdata")
    format = ScriptArchive::XP;
  else if (fileExt == "rvdata2")
    format = ScriptArchive::VXAce;
  else {
    cerr << "Unrecognized file extension: " << fileExt.toUtf8().constData() << endl;
    return 1;
  }

  QFile archiveFile(file);
  if (!archiveFile.open(QFile::WriteOnly)) {
    cerr << "Cannot open for writing: " << file.toUtf8().constData() << endl;
    return 1;
  }

  try {
    archive.write(archiveFile, format);
    archiveFile.close();
  }
  catch (const QByteArray &) {
    cerr << "Cannot save: " << file.toUtf8().constData() << endl;
    return 1;
  }

  return 0;
}

int import(QString src_folder, ScriptArchive &archive) {
	if (src_folder.isEmpty())
		return 1;

	/* Open index */
	QFile indFile(src_folder + "/index");
	if (!indFile.open(QFile::ReadOnly)) {
		cerr << "Cannot open index file" << endl;
		return 1;
	}

	QVector<ScriptArchive::Script> scripts;

	QTextStream indStream(&indFile);
	int scIdx = 0;

	while (!indStream.atEnd())
	{
		QString scName = indStream.readLine();

		QString scFilename = QString("%1").arg(scIdx, 3, 10, QLatin1Char('0'));
		QFile scFile(src_folder + "/" + scFilename);

		if (!scFile.open(QFile::ReadOnly)) {
			cerr << QString("Cannot open script \"%1\" (%2)").arg(scName, scFilename).toUtf8().constData() << endl;
			return 1;
		}

		QByteArray scData = scFile.readAll();
		scFile.close();

		ScriptArchive::Script script;
		script.magic = 0;
		script.name = scName;
		script.data = scData;

		scripts.append(script);

		++scIdx;
	}

	archive.scripts = scripts;

	return 0;
}

int main(int argc, char* argv[]) {
	if (argc < 3) {
		puts("Wrong number of arguments (cli <input directory> <output filename>)");
		return 1;
	}
	const QString src_folder = QString::fromLocal8Bit(argv[1]);
	const QString out_file = QString::fromLocal8Bit(argv[2]);
	ScriptArchive archive;
	import(src_folder, archive);
	saveArchive(out_file, archive);
	return 0;
}
