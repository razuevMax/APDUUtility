//! \file settingswidget.cpp
//! \brief Source of settings widget class.
#include <QSettings>
#include "settingswidget.h"
#include "nativescard.h"
#include "scardexception.h"


settingsWidget::settingsWidget(QWidget* parent)
{
 ui.setupUi(this);
 setAttribute(Qt::WA_DeleteOnClose, true);
 //Load vendors command list files
 ui.defaultVendorComboBox->addItem("none");
 QDir vendorsDir(QApplication::applicationDirPath() + "/vendors/", "*.json");
 vendorsDir.setFilter(QDir::Files | QDir::NoSymLinks);
 QFileInfoList list = vendorsDir.entryInfoList();
 int count = list.count();
 for (int i = 0; i < count; ++i)
 {
  QFileInfo fileInfo = list.at(i);
  ui.defaultVendorComboBox->addItem(fileInfo.baseName());
 }
 try
 {
  cardIface->EstablishContext(Smartcards::SCOPE::User);
 }
 catch (SCardException& e)
 {
  
 }
 reloadButtonClicked();
 QSettings settings;
 QString vendorName = settings.value("vendorName","none").toString();
 QString readerName = settings.value("readerName","none").toString();
 int scope = settings.value("scope", 0).toInt();
 int shareMode = settings.value("shareMode", 0).toInt();
 int protocol = settings.value("protocol", 0).toInt();
 int index = ui.defaultReaderComboBox->findText(readerName,Qt::MatchContains);
 if (index > 0)
 {
  ui.defaultReaderComboBox->setCurrentIndex(index);
  ui.readerNameLineEdit->setText(readerName);
 }
 index = ui.defaultVendorComboBox->findText(vendorName);
 if (index > 0)
  ui.defaultVendorComboBox->setCurrentIndex(index);
 ui.scopeComboBox->setCurrentIndex(scope);
 ui.shareModeComboBox->setCurrentIndex(shareMode);
 ui.protocolComboBox->setCurrentIndex(protocol);
 connect(ui.reloadReadersButton, SIGNAL(clicked()), this, SLOT(reloadButtonClicked()));
 connect(ui.closeButton, SIGNAL(clicked()), this, SLOT(closeButtonClicked()));
 connect(ui.defaultReaderComboBox, SIGNAL(currentTextChanged(const QString&)), this, SLOT(defaultReaderComboBoxTextChanged(const QString&)));
}

settingsWidget::~settingsWidget()
{
 cardIface->ReleaseContext();
}

void settingsWidget::closeEvent(QCloseEvent* event)
{
 closeButtonClicked();
}

void settingsWidget::reloadButtonClicked()
{
 QStringList readersNames;
 try
 {
  if (!cardIface->isContextEstablished())
   cardIface->EstablishContext(Smartcards::SCOPE::User);
  readersNames = cardIface->ListReaders();
  ui.defaultReaderComboBox->clear();
  ui.defaultReaderComboBox->addItem("none");
  ui.defaultReaderComboBox->addItems(readersNames);
 }
 catch (SCardException& e)
 {
  
 }
}

void settingsWidget::closeButtonClicked()
{
 QSettings settings;
 settings.setValue("vendorName", ui.defaultVendorComboBox->currentText());
 QString readerName = ui.readerNameLineEdit->text();
 settings.setValue("readerName", readerName.isEmpty()?"none":readerName);
 settings.setValue("scope", ui.scopeComboBox->currentIndex());
 settings.setValue("shareMode", ui.shareModeComboBox->currentIndex());
 settings.setValue("protocol", ui.protocolComboBox->currentIndex());
 close();
}

void settingsWidget::defaultReaderComboBoxTextChanged(const QString& text)
{
 if (text == "none")
  ui.readerNameLineEdit->clear();
 else
  ui.readerNameLineEdit->setText(text);
}

