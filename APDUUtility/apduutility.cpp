//! \file apduutility.cpp
//! \brief Source of APDU Utility main window class.
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QInputDialog>
#include <QClipboard>

#include "apduutility.h"
#include "nativescard.h"
#include "scardexception.h"
#include "settingswidget.h"

APDUUtility::APDUUtility(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    QSettings settings;
    defaultScope = static_cast<Smartcards::SCOPE>(settings.value("scope", 0).toInt());
    defaultShare = static_cast<Smartcards::SHARE>(settings.value("shareMode", 0).toInt());
    defaultProtocol = static_cast<Smartcards::PROTOCOL>(settings.value("protocol", 0).toInt());
    ui.APDUCommandsListView->setModel(APDUCommandsListModel.data());
    //Load vendors command list files
    QDir vendorsDir(QApplication::applicationDirPath() + "/vendors/","*.json");
    vendorsDir.setFilter(QDir::Files | QDir::NoSymLinks);
    QFileInfoList list = vendorsDir.entryInfoList();
    ui.vendorCommandsListFileComboBox->blockSignals(true);
    int count = list.count();
    for (int i = 0; i < count; ++i) 
    {
     QFileInfo fileInfo = list.at(i);
     ui.vendorCommandsListFileComboBox->addItem(fileInfo.baseName());
    }
    if(count>0)
     vendorCommandsListFileComboBoxIndexChanged(0);
    ui.vendorCommandsListFileComboBox->blockSignals(false);
    try
    {
     cardIface->EstablishContext(defaultScope);
    }
    catch (SCardException& e)
    {
     ui.statusBar->showMessage(e.errorString());
    }
    ui.CLALineEdit->installEventFilter(this);
    ui.INSLineEdit->installEventFilter(this);
    ui.P1LineEdit->installEventFilter(this);
    ui.P2LineEdit->installEventFilter(this);
    ui.LELineEdit->installEventFilter(this);
    ui.dataPlainTextEdit->installEventFilter(this);
    reloadButtonClicked();
    QString vendorName = settings.value("vendorName", "none").toString();
    QString readerName = settings.value("readerName", "none").toString();
    int index = ui.readersNamesComboBox->findText(readerName, Qt::MatchContains);
    if (index > 0)
     ui.readersNamesComboBox->setCurrentIndex(index);
    index = ui.vendorCommandsListFileComboBox->findText(vendorName);
    if (index > 0)
     ui.vendorCommandsListFileComboBox->setCurrentIndex(index);
    tId = startTimer(1000);
    connect(ui.actionSettings, SIGNAL(triggered()), this, SLOT(showSettings()));
    connect(ui.actionAbout, SIGNAL(triggered()), this, SLOT(about()));
    connect(ui.actionAbout_Qt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(ui.connectButton, SIGNAL(clicked()), this, SLOT(connectButtonClicked()));
    connect(ui.reloadButton, SIGNAL(clicked()), this, SLOT(reloadButtonClicked()));
    connect(ui.transmitButton, SIGNAL(clicked()), this, SLOT(transmitButtonClicked()));
    connect(ui.addNewVendorButton, SIGNAL(clicked()), this, SLOT(addNewVendorButtonClicked()));
    connect(ui.addNewCommandButton, SIGNAL(clicked()), this, SLOT(addNewCommandButtonClicked()));
    connect(ui.saveCurrentCommandButton, SIGNAL(clicked()), this, SLOT(saveCurrentCommandButtonClicked()));
    connect(ui.removeCommandButton, SIGNAL(clicked()), this, SLOT(removeCommandButtonClicked()));
    connect(ui.vendorCommandsListFileComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(vendorCommandsListFileComboBoxIndexChanged(int)));
    connect(ui.APDUCommandsListView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(APDUCommandsListViewActivated(const QModelIndex&)));
}

APDUUtility::~APDUUtility()
{
 if (tId != 0)
  killTimer(tId);
 cardIface->ReleaseContext();
}

void APDUUtility::timerEvent(QTimerEvent* event)
{
 //Check readers list
 ui.connectButton->setEnabled((ui.readersNamesComboBox->count()>0));
 ui.transmitButton->setEnabled(!ui.CLALineEdit->text().isEmpty() && !ui.INSLineEdit->text().isEmpty());
 //Check APDU commands list
 QModelIndex index = ui.APDUCommandsListView->currentIndex();
 if(index.isValid())
 {
  ui.saveCurrentCommandButton->setEnabled(true);
  ui.removeCommandButton->setEnabled(true);
 }
 else
 {
  ui.saveCurrentCommandButton->setEnabled(false);
  ui.removeCommandButton->setEnabled(false);
 }

}

void APDUUtility::closeEvent(QCloseEvent* event)
{
 if (commandChaged)
 {
  QString vendorFilePath = QApplication::applicationDirPath() + "/vendors/" + ui.vendorCommandsListFileComboBox->itemText(lastVendorIndex) + ".json";
  saveVendorCommandsList(vendorFilePath);
 }
}

bool APDUUtility::eventFilter(QObject* obj, QEvent* event)
{
 if (event->type() == QEvent::KeyPress) {
  QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
  if(obj==ui.dataPlainTextEdit)
  {
   QPlainTextEdit *edit= static_cast<QPlainTextEdit*>(obj);
   QTextCursor cursor = edit->textCursor();
   if(cursor.position()==0 && (keyEvent->matches(QKeySequence::MoveToPreviousChar) || (keyEvent->key() == Qt::Key_Backspace)))
   {
    ui.LELineEdit->setFocus();
    ui.LELineEdit->end(false);
    return true;
   }
  }
  else
  {
   QLineEdit *edit = static_cast<QLineEdit*>(obj);
   if (keyEvent->matches(QKeySequence::Paste))
   {
    paste(edit, QApplication::clipboard()->text());
    return true;
   }
   QString newText = keyEvent->text();
   if ((edit->cursorPosition() == 1) && !newText.isEmpty() && (keyEvent->key() != Qt::Key_Backspace))
   {
    edit->setText(edit->text().left(1).append(newText));
    move(edit, RIGHT_MOVE);
    return true;
   }
   if ((edit->cursorPosition() == 1) && keyEvent->matches(QKeySequence::MoveToNextChar))
   {
    move(edit, RIGHT_MOVE);
    return true;
   }
   if ((edit->cursorPosition() == 0) && (keyEvent->matches(QKeySequence::MoveToPreviousChar) || (keyEvent->key() == Qt::Key_Backspace)))
   {
    edit = move(edit, LEFT_MOVE);
    return true;
   }
  }
 }
 // standard event processing
 return QObject::eventFilter(obj, event);
}

void APDUUtility::showSettings()
{
 settingsWidget *settings = new settingsWidget;
 settings->show();
}

void APDUUtility::about()
{
 QMessageBox::about(this, tr("About APDU Utility"),
  tr("The <b>APDU Utility</b> - program to work with smart cards in accordance with ISO 7816 protocol by sending an APDU command."
  "<br>Created - Razuev Maxim. GitHub - <a href='https://github.com/razuevMax'>https://github.com/razuevMax</a>"));
}

void APDUUtility::connectButtonClicked()
{
 if (cardIface->isConnected())
  cardIface->Disconnect(Smartcards::DISCONNECT::Leave);
 QString readerName = ui.readersNamesComboBox->currentText();
 ui.statusBar->clearMessage();
 if (!readerName.isEmpty())
 {
  DWORD state, protocol;
  try
  {
   if (cardIface->Connect(readerName, defaultShare, defaultProtocol) != Smartcards::SUCCESS)
    readerName = "none";
   else
    ui.ATRLabel->setText(cardIface->GetCardStatus(state, protocol).toHex());
  }
  catch (SCardException& e)
  {
   readerName="none";
   ui.statusBar->showMessage(e.errorString());
  }
 }
 ui.readerNameLabel->setText(readerName);
}

void APDUUtility::reloadButtonClicked()
{
 QStringList readersNames;
 try
 {
  if(!cardIface->isContextEstablished())
   cardIface->EstablishContext(Smartcards::SCOPE::User);
  readersNames = cardIface->ListReaders();
  ui.readersNamesComboBox->clear();
  ui.readersNamesComboBox->addItems(readersNames);
 }
 catch (SCardException& e)
 {
  ui.statusBar->showMessage(e.errorString());
 }
}

void APDUUtility::transmitButtonClicked()
{
 bool ok = false;
 BYTE CLA = ui.CLALineEdit->text().toUShort(&ok, 16);
 BYTE INS = ui.INSLineEdit->text().toUShort(&ok, 16);
 BYTE P1= ui.P1LineEdit->text().toUShort(&ok, 16);
 BYTE P2 = ui.P2LineEdit->text().toUShort(&ok, 16);
 BYTE Le = ui.LELineEdit->text().toUShort(&ok, 16);
 QByteArray data = QByteArray::fromHex(ui.dataPlainTextEdit->toPlainText().toLocal8Bit());
 Smartcards::APDUCommand comm(CLA, INS, P1, P2, data, Le);
 Smartcards::APDUResponse resp;
 ui.statusBar->clearMessage();
 try
 {
  resp = cardIface->Transmit(comm);
 }
 catch(SCardException& e)
 {
  ui.statusBar->showMessage(e.errorString());
 }
 ui.SW1LineEdit->setText(QString::number(resp.getSW1(), 16));
 ui.SW2LineEdit->setText(QString::number(resp.getSW2(), 16));
 ui.resultDataPlainTextEdit->setPlainText(resp.getData().toHex());
}

void APDUUtility::addNewVendorButtonClicked()
{
 QString vendor = QInputDialog::getText(this, "Vendor name", "Set vendor name:");
 if (vendor.isEmpty())
  return;
 APDUCommandsListModel->clear();
 ui.vendorCommandsListFileComboBox->blockSignals(true);
 ui.vendorCommandsListFileComboBox->addItem(vendor);
 lastVendorIndex = ui.vendorCommandsListFileComboBox->findText(vendor);
 ui.vendorCommandsListFileComboBox->setCurrentIndex(lastVendorIndex);
 ui.vendorCommandsListFileComboBox->blockSignals(false);
}

void APDUUtility::addNewCommandButtonClicked()
{
 clearAPDUCommand();
 QModelIndex index = ui.APDUCommandsListView->currentIndex();
 QString commandName = "newCommand";
 int count=APDUCommandsListModel->findItems(commandName, Qt::MatchStartsWith).count();
 QStandardItem *newItem = new QStandardItem(count>0?commandName+"_"+QString::number(count):commandName);
 newItem->setData(QVariant::fromValue<Smartcards::APDUCommand>(Smartcards::APDUCommand(0, 0, 0, 0)));
 APDUCommandsListModel->insertRow(index.isValid()?index.row():APDUCommandsListModel->rowCount(), newItem);
 commandChaged = true;
}

void APDUUtility::saveCurrentCommandButtonClicked()
{
 bool ok;
 BYTE CLA = ui.CLALineEdit->text().toUShort(&ok, 16);
 BYTE INS = ui.INSLineEdit->text().toUShort(&ok, 16);
 BYTE P1 = ui.P1LineEdit->text().toUShort(&ok, 16);
 BYTE P2 = ui.P2LineEdit->text().toUShort(&ok, 16);
 BYTE Le = ui.LELineEdit->text().toUShort(&ok, 16);
 QByteArray data = QByteArray::fromHex(ui.dataPlainTextEdit->toPlainText().toLocal8Bit());
 Smartcards::APDUCommand command(CLA, INS, P1, P2, data, Le);
 APDUCommandsListModel->itemFromIndex(ui.APDUCommandsListView->currentIndex())->setData(QVariant::fromValue<Smartcards::APDUCommand>(command));
 commandChaged = true;
}

void APDUUtility::removeCommandButtonClicked()
{
 APDUCommandsListModel->removeRow(ui.APDUCommandsListView->currentIndex().row());
 commandChaged = true;
}

void APDUUtility::vendorCommandsListFileComboBoxIndexChanged(int index)
{
 if(lastVendorIndex>=0 && commandChaged)
 {
  QString vendorFilePath = QApplication::applicationDirPath() + "/vendors/" + ui.vendorCommandsListFileComboBox->itemText(lastVendorIndex)+".json";
  saveVendorCommandsList(vendorFilePath);
 }
 commandChaged = false;
 if(index>=0)
 {
  QString vendorFilePath = QApplication::applicationDirPath()+"/vendors/" + ui.vendorCommandsListFileComboBox->itemText(index) + ".json";
  loadVendorCommandsList(vendorFilePath);
 }
 lastVendorIndex = index;
}

void APDUUtility::APDUCommandsListViewActivated(const QModelIndex& index)
{
 QStandardItem *item = APDUCommandsListModel->itemFromIndex(index);
 Smartcards::APDUCommand command(item->data().value<Smartcards::APDUCommand>());
 ui.CLALineEdit->setText(QString::number(command.getClass(),16));
 ui.INSLineEdit->setText(QString::number(command.getIns(), 16));
 ui.P1LineEdit->setText(QString::number(command.getP1(), 16));
 ui.P2LineEdit->setText(QString::number(command.getP2(), 16));
 ui.LELineEdit->setText(QString::number(command.getLe(), 16));
 ui.dataPlainTextEdit->setPlainText(QString(command.getData().toHex()));
}

void APDUUtility::loadVendorCommandsList(const QString& filePath)
{
 QFile loadFile(filePath);
 if (!loadFile.open(QIODevice::ReadOnly)) 
 {
  QString err = loadFile.errorString();
  ui.statusBar->showMessage("Couldn't open vendor commands list file for read.\n"+err);
  return;
 }
 APDUCommandsListModel->clear();
 QByteArray fileData = loadFile.readAll();
 QJsonDocument loadDoc(QJsonDocument::fromJson(fileData));
 QJsonObject docObject = loadDoc.object();
 QStringList APDUCommandsNames = docObject.keys();
 for(auto APDUObjectIterator=docObject.constBegin();APDUObjectIterator!=docObject.constEnd();APDUObjectIterator++)
 {
  QStandardItem *item = new QStandardItem(APDUObjectIterator.key());
  QJsonObject APDUObject = APDUObjectIterator.value().toObject();
  Smartcards::APDUCommand command(QByteArray::fromHex(APDUObject.value("CLA").toString().toLocal8Bit()).at(0), 
   QByteArray::fromHex(APDUObject.value("INS").toString().toLocal8Bit()).at(0), 
   QByteArray::fromHex(APDUObject.value("P1").toString().toLocal8Bit()).at(0), 
   QByteArray::fromHex(APDUObject.value("P2").toString().toLocal8Bit()).at(0), 
   QByteArray::fromHex(APDUObject.value("Data").toString().toLocal8Bit()), 
   QByteArray::fromHex(APDUObject.value("Le").toString().toLocal8Bit()).at(0));
  item->setData(QVariant::fromValue<Smartcards::APDUCommand>(command));
  APDUCommandsListModel->appendRow(item);
 }
}

void APDUUtility::saveVendorCommandsList(const QString& filePath)
{
 QFile saveFile(filePath);
 if (!saveFile.open(QIODevice::WriteOnly|QIODevice::Truncate))
 {
  QString err = saveFile.errorString();
  ui.statusBar->showMessage("Couldn't open vendor commands list file for save.\n" + err);
  return;
 }
 QJsonObject mainObj;
 int count = APDUCommandsListModel->rowCount();
 for(int row=0;row<count;row++)
 {
  QStandardItem *item = APDUCommandsListModel->item(row);
  Smartcards::APDUCommand command(item->data().value<Smartcards::APDUCommand>());
  QJsonObject APDUObject;
  APDUObject["CLA"] = QString::number(command.getClass(),16);
  APDUObject["INS"] = QString::number(command.getIns(), 16);
  APDUObject["P1"] = QString::number(command.getP1(), 16);
  APDUObject["P2"] = QString::number(command.getP2(), 16);
  APDUObject["Le"] = QString::number(command.getLe(), 16);
  APDUObject["Data"] = QString(command.getData().toHex());
  mainObj[item->text()] = APDUObject;
 }
 QJsonDocument saveDoc(mainObj);
 saveFile.write(saveDoc.toJson());
}

void APDUUtility::clearAPDUCommand(void) const
{
 ui.CLALineEdit->setText("00");
 ui.INSLineEdit->setText("00");
 ui.P1LineEdit->setText("00");
 ui.P2LineEdit->setText("00");
 ui.LELineEdit->setText("00");
 ui.dataPlainTextEdit->clear();
}

void APDUUtility::paste(QLineEdit* edit, const QString& text)
{
 QVector<QLineEdit*> editVector{ui.CLALineEdit,ui.INSLineEdit,ui.P1LineEdit,ui.P2LineEdit,ui.LELineEdit};
 QLineEdit **currEdit= std::find(editVector.begin(),editVector.end(), edit);
 int count = 2;
 QString pastedText = (*currEdit)->text().left((*currEdit)->cursorPosition())+text;
 do
 {
  (*currEdit)->setFocus();
  (*currEdit)->setText(pastedText.left(count));
  pastedText.remove(0, count);
  count = pastedText.count() >= 2 ? 2 : pastedText.count();
  currEdit++;
 } while (currEdit != editVector.end() && !pastedText.isEmpty());
 if (pastedText.count() > 0)
 {
  ui.dataPlainTextEdit->moveCursor(QTextCursor::Start);
  ui.dataPlainTextEdit->textCursor().insertText(pastedText);
  ui.dataPlainTextEdit->setFocus();
 }
}

QLineEdit * APDUUtility::move(QLineEdit* edit, MOVE direction)
{
 QVector<QLineEdit*> editVector{ ui.CLALineEdit,ui.INSLineEdit,ui.P1LineEdit,ui.P2LineEdit,ui.LELineEdit };
 QLineEdit **currEdit = std::find(editVector.begin(), editVector.end(), edit);
 switch(direction)
 {
 case LEFT_MOVE:
  if(edit!=ui.CLALineEdit)
  {
   currEdit--;
   (*currEdit)->setFocus();
   //(*currEdit)->setCursorPosition(1);
   (*currEdit)->end(false);
   return *currEdit;
  }
  break;
 case RIGHT_MOVE:
  if(edit != ui.LELineEdit)
  {
   currEdit++;
   (*currEdit)->setFocus();
   (*currEdit)->home(false);
   return *currEdit;
  }
  else
  {
   ui.dataPlainTextEdit->setFocus();
   ui.dataPlainTextEdit->moveCursor(QTextCursor::Start);
  }
  break;
 }
 return nullptr;
}
