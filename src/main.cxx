#include "ruby_data.hxx"
#include "qt/main_window.hxx"

#include <QtGui/QApplication>
#include <QtCore/QTextCodec>

int main(int argc, char* argv[]) {
  RubyInstance ruby_inst;
  (void)ruby_inst;

  QApplication app(argc, argv);

  QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

  RGSS_MainWindow main;
  if(argc == 2) {
    main.setScriptArchive(QString::fromUtf8(argv[1]));
  }
  main.show();

  app.setActiveWindow(&main);

  return app.exec();
}
