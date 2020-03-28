/* Revision History
 * -------- -------
 *  9 Feb 20 -- Code finally works as intended, doing most of what the go versions do.  It only processes one command in the lineedit box at a time, ignoring other commands.
 *               I don't remember when I first decided to attempt this; I believe it was Sep-Oct 2019.  That's ~ 4 months ago.
 *
 * 14 Feb 20 -- Fixed a minor output tweak for in WriteReg().
 *
 * 15 Feb 20 -- Changing the History/Display box to a combobox so I can click on an item and reuse it.
 *
 * 16 Feb 20 -- Could not get qtrpn to modify its ui.  So I'm creating qtrpn2, w/ a new ui
 *
 * 22 Feb 20 -- Added global ui2 so could show initial form w/ content.  And found setFocus method for lineEdit.
 *
 * 25 Feb 20 -- Added PlaceHolderText as the prompt.
 *
 * 27 Feb 20 -- Added "?" as help. And menu items for help and about.  And learned that in main.cpp, I can use w.showFullScreen() or other variants of the show() method.
 *
 * 22 Mar 20 -- Started to actually use it, and I noticed that I need a top display of X register.  So I added it.  And I enlarged the mainwindow and its list widgets
 *
 * 27 Mar 20 -- Adding toclip.
 *
 * 28 Mar 20 -- Adding fromclip.
 */


#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <vector>
#include <QFile>
#include <QFileDialog>
#include <QDataStream>
#include <QMessageBox>
#include <QDialog>
#include <QInputDialog>
#include <QDir>
#include <QtDebug>
#include <QClipboard>
#include <QThread>
// ----------------------- my stuff
#include "macros.h"
#include "hpcalcc2.h"
// in hpcalcc2.h is this
//  struct calcPairType {
//      vector<string> ss;
//      double R;
//  };

#include "tokenizec2.h"
#include "timlibc2.h"
#include "holidaycalc2.h"
#include "getcommandline2.h"
#include "makesubst.h"

void ProcessInput(string cmdstr);  // forward reference
void FUNCTION REPAINT(); // forward reference

int SigFig = -1;  // qt creator complained about a double definition of sigfig, so this one is now SigFig
struct RegisterType {
    double value;
    QString name;  // w/ my struggles about string <--> QString, I have to more careful.  I have to use a QString for I/O.
};

ARRAYOF RegisterType Storage[36];  // var Storage []RegisterType  in Go syntax
ARRAYOF double Stk[StackSize];
bool CombinedFileExists;
const char *CombinedFileName = "RPNStackStorage.sav";
QString CombinedFilenamestring;
string LastCompiledDate = __DATE__;
string LastCompiledTime = __TIME__;

enum OutputStateEnum {outputfix, outputfloat, outputgen};
OutputStateEnum OutputState = outputfix;

Ui::MainWindow *ui2; // global so the pgm can start up without blanks widgets.  Name clash when this was ui, and pgm crashed.

// constructor
/*                                           in mainwindow.h
class MainWindow : public QMainWindow {

    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);  when a declaration within a class have the same name, that's a constructor definition.
    ~MainWindow();

private slots:
    void on_action0_triggered();
    void on_action2_triggered();
    void on_action4_triggered();
    void on_action6_triggered();
    void on_action8_triggered();
    void on_actionfixed_triggered();
    void on_actionfloat_triggered();
    void on_actiongen_triggered();
    void on_actionclear_area_triggered();
    void on_lineEdit_returnPressed();
    void on_pushButton_enter_clicked();
    void on_pushButton_exit_clicked();
    void on_pushButton_quit_clicked();
    void on_comboBox_activated(const QString &arg1);
    void on_action_1_triggered();
    void on_pushButton_exit_pressed();
    void on_pushButton_quit_pressed();

private:
    Ui::MainWindow *ui;
};
*/


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) { // name same as class is the constructor
    ui->setupUi(this);

    CombinedFilenamestring = QDir::homePath() + "/" + CombinedFileName;
    QFile fh(CombinedFilenamestring);
    CombinedFileExists = fh.exists();
    if (CombinedFileExists) { // read the file into the stack and storage registers.
        bool opened = fh.open(QFile::ReadOnly);
        if (NOT opened) QMessageBox::warning(this," file not opened", "Stack and Storage combined input file could not be opened, but it exists.");
        QDataStream combinedfile(&fh);
        for (int i = 0; i < StackSize; i++) {  // read in Stk
          combinedfile >> Stk[i];
          PUSHX(Stk[i]);
        }

        for (int i = 0; i < 36; i++) {  // read in Storage registers
            QString qstr;
            double r;
            combinedfile >> r;
            combinedfile >> qstr;
            Storage[i].value = r;
            Storage[i].name = qstr;
        }
    } else {  // init the stack and storage registers to the zero value for each type.
        for (int i = 0; i < StackSize; i++) Stk[i] = 0.;  // Zero out stack

        for (int i = 0; i < 36; i++) {
            Storage[i].value = 0.;
            Storage[i].name = "";
        }
    }

    ui2 = ui;  // initialize the global ui2, for use in REPAINT();

    QStringList list = QCoreApplication::arguments();
    QString qstr, argv;
    if (NOT list.isEmpty()){
        QStringList::iterator iter;
        argv = "";
        for (iter = list.begin(); iter != list.end(); iter++){
            qstr = *iter;
            argv += qstr + " ";
        }
    }
    REPAINT();
    //ui->lineEdit->setCursorPosition(0);  // try to give the line edit box the focus.  Didn't work
    ui->lineEdit->setPlaceholderText("Input command"); // doesn't give the box focus.
    ui->lineEdit->setFocus(); // this looks promising
}

MainWindow::~MainWindow() {
    delete ui;
}

void FUNCTION ToClip() {
    QClipboard *clipboard = QApplication::clipboard();

    double R = READX();
    string str = to_string(R);
    QString qstr = QString::fromStdString(str);
    clipboard->setText(qstr, QClipboard::Clipboard);
    if (clipboard->supportsSelection()) {
        clipboard->setText(qstr, QClipboard::Selection);
    }

    #if defined (Q_OS_LINUX)
        QThread::msleep(1);
    #endif
}

void FUNCTION FromClip() {
    QClipboard *clipboard = QApplication::clipboard();

    QString qstr = clipboard->text();
    //string str = qstr.toStdString();  I don't think this is needed
    //double R = std::atof(str.c_str());
    double R = qstr.toDouble();
    PUSHX(R);

}

void WriteStack(Ui::MainWindow *ui) {

    calcPairType calcpair; // var calcpair calcPairType  using Go syntax
    ARRAYOF QString qstack[StackSize];

    IF OutputState == outputfix THEN
            // _, stackslice = GetResult("DUMPFIXED");
            calcpair = GetResult("DUMPFIXED");
    ELSIF OutputState == outputfloat THEN
            // _, stackslice = GetResult("DUMPFLOAT");
            calcpair = GetResult("DUMPFLOAT");
    ELSIF OutputState == outputgen THEN
            // _, stackslice = GetResult("DUMP");
            calcpair = GetResult("DUMP");
    ENDIF;

    ui->listWidget_stack->clear();
    // assign stack of qstrings and write the string vectors listWidget_Stack
    for (int i = 0; i < StackSize; i++) {
        qstack[i] = qstack[i].fromStdString(calcpair.ss[i]);
        ui->listWidget_stack->addItem(qstack[i]);
    }
} // WriteStack()

int FUNCTION GetRegIdx(char c) {
    int idx = 0;
    char ch;

    ch = CAP(c);
    if ((ch >= '0') AND (ch <= '9')) {
        idx = ch - '0';
    } else if ((ch >= 'A') AND (ch <= 'Z'))   {
        idx = ch - 'A' + 10;
    }
    return idx;
} // GetRegIdx

char FUNCTION GetRegChar(int i) {
    char ch = '\0';

    if ((i >= 0) AND (i <= 9)) {
        ch = '0' + i;
    } else if ((i >= 10) AND (i < 36)) {
        ch = 'A' + i - 10;
    }
    return ch;
} // GetRegChar

void WriteReg(Ui::MainWindow *ui) {
/*
 struct RegisterType {
    double value;
    QString name;
 };
ARRAYOF RegisterType Storage[36];  // var Storage []RegisterType  in Go syntax
enum OutputStateEnum {outputfix, outputfloat, outputgen};
OutputStateEnum OutputState = outputfix;
*/
  stringstream stream;

  stream << "The following registers are not zero." << endl;
  for (int i = 0; i < 36; i++) {
      if (Storage[i].value != 0.) {
          stream << "Reg [" << GetRegChar(i) << "]: ";
          stream << Storage[i].name.toStdString() << "= ";
          if (OutputState == outputfix) {
              stream.setf(ios::fixed);
              stream.width(15);
              stream.precision(SigFig);
              stream << Storage[i].value << endl;

          } else if (OutputState == outputfloat) {
              stream.setf(ios::scientific);
              stream.width(15);
              stream.precision(SigFig);
              stream << Storage[i].value << endl;

          } else {
              string str;
              str = to_string(Storage[i].value);
              stream << str << endl;

          }

      }
  }
  stream.flush();
  ui->listWidget_registers->clear();
  QString qstr = QString::fromStdString(stream.str());
  ui->listWidget_registers->addItem(qstr);
} // WriteReg()

void WriteHelp(QWidget *parent) {  // this param is intended so that 'this' can be used in the QMessageBox call.
    stringstream stream;
    calcPairType calcpairvar;
    vector<string> stringslice;
    vector<string>::iterator iterate;

    calcpairvar = GetResult("HELP");

    IF NOT calcpairvar.ss.empty() THEN
      for (iterate = calcpairvar.ss.begin(); iterate != calcpairvar.ss.end(); iterate++) {
         stream << *iterate  << endl;
      }

      calcpairvar.ss.clear();
      stream.flush();

      QString qs = QString::fromStdString(stream.str());
      QMessageBox::information(parent,"Help", qs);
    ENDIF;
}

void FUNCTION repaint(Ui::MainWindow *ui) {
  WriteStack(ui);
  WriteReg(ui);
} // repaint()

void FUNCTION REPAINT() {
    repaint(ui2);
}

QString FUNCTION GetNameString(QWidget *parent) {
  QString prompt = "Input name, will make - or = into a space : ";
  bool ok;

  QString text = QInputDialog::getText(parent,"Enter Name for Storage Register", prompt,QLineEdit::Normal,"",&ok);
  if (NOT ok) {
      return "";
  }
  if ((text == "t") OR (text == "today")) {
      MDYType mdy = TIME2MDY();
      string datestring, dstr;
      datestring = MDY2STR(mdy.m,mdy.d,mdy.y, dstr);
      text = text.fromStdString(datestring);
      return text;
  }
  return text;  // compiler flagged this as possible end of function without a return.  I don't think this line can be reached, but it makes the compiler happy, so it's here.
}

string FUNCTION toupper(string s) {
    string upper = "";
    string::iterator iter;
    for (iter = s.begin(); iter != s.end(); iter++) {
        upper += CAP(*iter);
    }
    return upper;
}

void FUNCTION ProcessInput(QWidget *parent, Ui::MainWindow *ui, string cmdstr) {
    string LastCompiled = LastCompiledDate + " " + LastCompiledTime;
    calcPairType calcpair;
    vector<string> stringslice;
    vector<string>::iterator iter;

    // Write cmdstr to the combobox, which was the history box, ie, listWidget_History.
    QString qs = QString::fromStdString(cmdstr);
    ui->listWidget_hx->addItem(qs);
    ui->comboBox->addItem(qs);
    ui->listWidget_X->clear();


    if ((cmdstr.compare("help") == 0) OR (cmdstr.compare("?") EQ 0)  ){   // help
        WriteHelp(parent);
    } else if (cmdstr.find("sto") EQ 0) {  // stoN
        int i = 0;
        char ch = '\0';
        if (cmdstr.length() > 3) {
            ch = cmdstr.at(3);  // the 4th character, right after the 'o' in 'sto'
            i = GetRegIdx(ch);
        }
        Storage[i].value = READX();
        if (i > 0) Storage[i].name = GetNameString(parent);

    } else if (cmdstr.find("rcl") EQ 0) {  // rclN
        int i = 0;
        if (cmdstr.length() > 3) {
            char ch = cmdstr.at(3); // the 4th char.
            i = GetRegIdx(ch);
        }
        double R = Storage[i].value;
        PUSHX(R);

    } else if (cmdstr.find("name") EQ 0) {
        int i = 0;
        if (cmdstr.length() > 4 ) {
            char ch = cmdstr.at(4); // the 5th char.
            i = GetRegIdx(ch);
        }
        Storage[i].name = GetNameString(parent);

    } else if ((cmdstr.compare("dump") EQ 0) OR (cmdstr.find("sho") EQ 0)) {
        // do nothing
    } else if (cmdstr.compare("about") EQ 0) {
        calcpair = GetResult("ABOUT");
        for (iter = calcpair.ss.begin(); iter != calcpair.ss.end(); iter++) {
            QString qstr = iter->c_str(); // recall that the -> operator is a type of dereference operator.
            ui->listWidget_output->addItem(qstr);
        }

        string aboutmsg = "MainWindow Pgm last compiled " + LastCompiledDate + " " + LastCompiledTime;
        QString qaboutmsg = aboutmsg.c_str();  // only works when c-string.

        ui->listWidget_output->addItem(qaboutmsg);

    } else if (cmdstr.compare("toclip") EQ 0) {
        ToClip();

    } else if (cmdstr.compare("fromclip") EQ 0) {
        FromClip();

    } else if (cmdstr.compare("debug") EQ 0) {
            GETSTACK(Stk);
            stringstream stream, stream1, stream2, stream3;
            vector<double> stackmatrixrow;

            stream << "Raw Stk[] with X first is : ";

            FOR int i = X; i < StackSize; i++ DO
              string str = to_string(Stk[i]);
              stream << str  << "  ";
            ENDFOR;

            QString qoutput = QString::fromStdString(stream.str());
            ui->listWidget_output->addItem(qoutput);

            stackmatrixrow = GetStackMatrixRow(X);
            stream1 << "First row of StackMatrix is :";

            FOR int i = X; i < StackSize; i++ DO
              string str = to_string(stackmatrixrow.at(i));
              stream1 << str  << "  ";
            ENDFOR;

            qoutput = "";
            qoutput = QString::fromStdString(stream1.str());
            ui->listWidget_output->addItem(qoutput);

            stackmatrixrow = GetStackMatrixRow(Y);
            stream2 << "Second row of StackMatrix is :";

            FOR int i = X; i < StackSize; i++ DO
              string str = to_string(stackmatrixrow.at(i));
              stream2 << str  << "  ";
            ENDFOR;

            qoutput = "";
            qoutput = QString::fromStdString(stream2.str());
            ui->listWidget_output->addItem(qoutput);

            stackmatrixrow = GetStackMatrixRow(Z);
            stream3 << "3rd row of StackMatrix is : ";

            FOR int i = X; i < StackSize; i++ DO
              string str = to_string(stackmatrixrow.at(i));
              stream3 << str << "  ";
            ENDFOR;
            stream3 << endl;

            qoutput = "";
            qoutput = QString::fromStdString(stream3.str());
            ui->listWidget_output->addItem(qoutput);


    } else {
        calcpair = GetResult(cmdstr);

        if (NOT calcpair.ss.empty()) {
            for (iter = calcpair.ss.begin(); iter != calcpair.ss.end(); iter++) {
                QString qstr = iter->c_str(); // recall that the -> operator is a type of dereference operator.
                ui->listWidget_output->addItem(qstr);
            }
        }




    } // else from if input "help"

    repaint(ui);

    double R = READX();
    QString qR1, qR2, qR3, qRfix, qRfloat, qRgen;
    string str = to_string(R);
    str = CropNStr(str);
    if (calcpair.R > 10000) str = AddCommas(str);
    qR1 = QString("%1").arg(str.c_str());
    qR2 = QString::fromStdString(str);
    qR3 = QString("%1").arg(R);
    qRgen = QString("%1").arg(R,5,'g',SigFig);// params are: double,int fieldwidth=0, char format='g', int precision= -1 ,QChar fillchar = QLatin1Char(' ').
    qRfix = QString("%1").arg(R,2,'f',SigFig);  // more general form of the conversion which can use 'e', 'f' and sigfig.
    qRfloat = QString("%1").arg(R,9,'e',SigFig);
    QString qoutputline = qR1 + "          " + qR2 + "           " + qRgen;
    //qoutputline = "qR1= " + qR1 + ", qR2= " + qR2 + ", qR3= " + qR3 + ", qRgen= " + qRgen + ", qRfix= " + qRfix + ", qRfloat= " + qRfloat;

    ui->listWidget_X->addItem(qoutputline);


    // clear the input lineedit before returning to it
    ui->lineEdit->clear();
}




void MainWindow::on_action_1_triggered() {
    SigFig = -1;
    GetResult("sig");
    ProcessInput(this,ui,"sho");
}

void MainWindow::on_action0_triggered() {
    SigFig = 0;
    GetResult("sig0");
    ProcessInput(this,ui,"sho");
}

void MainWindow::on_action2_triggered() {
    SigFig = 2;
    GetResult("sig2");
    ProcessInput(this,ui,"sho");

}

void MainWindow::on_action4_triggered() {
    SigFig = 4;
    GetResult("sig4");
    ProcessInput(this,ui,"sho");
}

void MainWindow::on_action6_triggered() {
    SigFig = 6;
    GetResult("sig6");
    ProcessInput(this,ui,"sho");
}

void MainWindow::on_action8_triggered() {
    SigFig = 8;
    GetResult("sig8");
    ProcessInput(this,ui,"sho");
}

void MainWindow::on_actionfixed_triggered() {
    OutputState = outputfix;
    ProcessInput(this,ui,"sho");
}

void MainWindow::on_actionfloat_triggered() {
    OutputState = outputfloat;
    ProcessInput(this,ui,"sho");
}

void MainWindow::on_actiongen_triggered() {
    OutputState = outputgen;
    ProcessInput(this,ui,"sho");
}

void MainWindow::on_actionclear_area_triggered() {
    ui->listWidget_output->clear();
}

void MainWindow::on_lineEdit_returnPressed() {
    QString inlineedit = ui->lineEdit->text();
    string inbuf = inlineedit.toStdString();
    inbuf = makesubst(inbuf);
    ProcessInput(this, ui, inbuf);
}

void MainWindow::on_pushButton_enter_clicked() {
    QString inlineedit = ui->lineEdit->text();
    string inbuf = inlineedit.toStdString();
    inbuf = makesubst(inbuf);
    ProcessInput(this, ui, inbuf);
}

void MainWindow::on_pushButton_exit_clicked() {
    GETSTACK(Stk);

// need to write the file
    // QMessageBox::information(this,"filename", CombinedFilenamestring);  It's working, so I don't need this.
    QFile file(CombinedFilenamestring);
    if (! file.open(QFile::WriteOnly)) {  // need to match up what's read and what's written.
        QMessageBox::warning(this,"open failed","file could not be opened in write mode.");
        return;
    }
    QDataStream stackout(&file);

    FOR int i = T1; i >= X; i-- DO  // need to do in reverse so it's read in correctly.  This is a stack, so LIFO.
      stackout << Stk[i];
    ENDFOR;

    FOR int i = 0; i < 36; i++ DO
      stackout << Storage[i].value;
      stackout << Storage[i].name;
    ENDFOR;

    file.flush();
    file.close();

    QApplication::quit();      // same as QCoreApplication::quit();

    // if the event loop is not running, these quit() commands will not work.  In that case, need to call exit(EXIT_FAILURE);
    // exit(EXIT_FAILURE);  is this even needed?  I'm leaving it out of the exit button and will leave it in the quit button.
}

void MainWindow::on_pushButton_quit_clicked() {
    QApplication::quit();      // same as QCoreApplication::quit();

// if the event loop is not running, these quit() commands will not work.  In that case, need to call exit(EXIT_FAILURE);
    exit(EXIT_FAILURE);
}

void MainWindow::on_comboBox_activated(const QString &arg1) {
    QString qs= arg1;
//    QMessageBox::information(this, "combobox activated", qs);
//    qDebug() << "Combobox activated.  Param is: " + arg1;  It works.  I don't need this anymore.
    string inbuf = qs.toStdString();
    inbuf = makesubst(inbuf);
    ProcessInput(this, ui, inbuf);
}

/*
void MainWindow::on_comboBox_currentTextChanged(const QString &arg1) {
    QString qs= arg1;
    QMessageBox::information(this, "combobox textchanged", qs);
}

void MainWindow::on_comboBox_editTextChanged(const QString &arg1) {
    QString qs= arg1;
    QMessageBox::information(this, "combobox edit text changed", qs);

}

void MainWindow::on_comboBox_highlighted(const QString &arg1) {

}

void MainWindow::on_comboBox_textActivated(const QString &arg1) {

}

void MainWindow::on_comboBox_textHighlighted(const QString &arg1) {
}

*/

void MainWindow::on_pushButton_exit_pressed() {
    qDebug() << "in on_pushButton_exit_pressed";
    on_pushButton_exit_clicked();
}

void MainWindow::on_pushButton_quit_pressed() {
    qDebug() << "in on_pushButton_quit_pressed";
    on_pushButton_quit_clicked();
}

void MainWindow::on_actionHelp_triggered() {
    WriteHelp(this);
}

void MainWindow::on_actionAbout_triggered() {
    vector<string> stringslice;
    vector<string>::iterator iter;

    calcPairType calcpair = GetResult("ABOUT");
    for (iter = calcpair.ss.begin(); iter != calcpair.ss.end(); iter++) {
        QString qstr = iter->c_str(); // recall that the -> operator is a type of dereference operator.
        ui->listWidget_output->addItem(qstr);
    }

    string aboutmsg = "MainWindow Pgm last compiled " + LastCompiledDate + " " + LastCompiledTime;
    QString qaboutmsg = aboutmsg.c_str();
    // QString aboutmsg2 = QString::fromStdString(aboutmsg); this works
    // qaboutmsg.fromStdString(aboutmsg);  this also works

    ui->listWidget_output->addItem(qaboutmsg);

}

void MainWindow::on_actionToClip_triggered()
{
    ToClip();
}

void MainWindow::on_actionFromClip_triggered()
{
    FromClip();
    ProcessInput(this, ui, " ");  // so the stack gets updated on screen.
}
