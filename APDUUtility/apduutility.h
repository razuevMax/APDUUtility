//! \file APDUUtility.h
//! \brief Header file for APDU Utility main window class.
#ifndef APDUUTILITY_H
#define APDUUTILITY_H

#include <QtWidgets/QMainWindow>
#include <QStandardItemModel>
#include "ui_apduutility.h"
#include "nativescard.h"

//! \class APDUUtility
//! \brief APDU Utility main window class.
class APDUUtility : public QMainWindow
{
 Q_OBJECT
  enum MOVE 
 { 
  RIGHT_MOVE, //!< Move cursor right
  LEFT_MOVE   //!< Move cursor left
 };
public:
 //!\brief Constructor
 //!\param[in] parent Parent widget, default is zero.
 APDUUtility(QWidget *parent = 0);
 //! \brief Destructor
 ~APDUUtility();
protected:
 //! \fn APDUUtility::timerEvent( QTimerEvent* event )
 //! \brief Qt timer event function
 //! \details Contains enabled operations for buttons
 //! \param[in] event pointer to Qt timer event class, ignored.
 void timerEvent(QTimerEvent *event);
 //! \fn APDUUtility::closeEvent(QCloseEvent *event)
 //! \brief close main window event function
 //! \details Save vendor commands list 
 //! \param[in] event pointer to QCloseEvent class, ignored.
 void closeEvent(QCloseEvent *event);
 //! \fn APDUUtility::eventFilter(QObject *obj, QEvent *event)
 //! \brief event filter for parts of APDU command
 //! \details Provides insertion processing digits, move the cursor. Provides input APDU fields work team as a whole.
 //! \param[in] obj pointer to QObject receiving event.
 //! \param[in] event pointer to QEvent class.
 bool eventFilter(QObject *obj, QEvent *event);
private slots:
//! \fn void APDUUtility::showSettings(void)
//! \brief Show the settings widget.
 void showSettings(void);
 //! \fn void APDUUtility::about(void)
 //! \brief Show the about widget.
 void about(void);
 //! \fn void APDUUtility::connectButtonClicked(void)
 //! \brief Connect to selected reader.
 void connectButtonClicked(void);
 //! \fn void APDUUtility::reloadButtonClicked(void)
 //! \brief Reload readers list.
 void reloadButtonClicked(void);
 //! \fn void APDUUtility::transmitButtonClicked(void)
 //! \brief Transmit APDU command to reader.
 void transmitButtonClicked(void);
 //! \fn void APDUUtility::addNewVendorButtonClicked(void)
 //! \brief Add new vendor to vendors list.
 void addNewVendorButtonClicked(void);
 //! \fn void APDUUtility::addNewCommandButtonClicked(void)
 //! \brief Add new command to APDU command list. Clear APDU Command fields.
 void addNewCommandButtonClicked(void);
 //! \fn void APDUUtility::saveCurrentCommandButtonClicked(void)
 //! \brief Save APDU command fields into selected command.
 void saveCurrentCommandButtonClicked(void);
 //! \fn void APDUUtility::removeCommandButtonClicked(void)
 //! \brief Remove selected command from list.
 void removeCommandButtonClicked(void);
 //! \fn void APDUUtility::vendorCommandsListFileComboBoxIndexChanged(int index)
 //! \brief Provides change current vendor. Save previous vendor command list and load selected vendor command list.
 //! \param[in] index index of selected vendor.
 void vendorCommandsListFileComboBoxIndexChanged(int index);
 //! \fn void APDUUtility::APDUCommandsListViewActivated(const QModelIndex &index)
 //! \brief Provides change selected APDU command. Fill APDU command fields.
 //! \param[in] index index of selected APDU command in model.
 void APDUCommandsListViewActivated(const QModelIndex &index);
private:
 //! \fn void APDUUtility::loadVendorCommandsList(const QString& filePath)
 //! \brief Load vendor commands list from json-file.
 //! \param[in] filePath string contains vendor file path.
 void loadVendorCommandsList(const QString& filePath);
 //! \fn void APDUUtility::saveVendorCommandsList(const QString& filePath)
 //! \brief Save vendor commands list to json-file.
 //! \param[in] filePath string contains vendor file path.
 void saveVendorCommandsList(const QString& filePath);
 //! \fn void APDUUtility::clearAPDUCommand(void)
 //! \brief Clear APDU command fields.
 void clearAPDUCommand(void) const;
 //! \fn void APDUUtility::paste(QLineEdit *edit, const QString& text)
 //! \brief Provides paste data in APDU command fields.
 //! \param[in] edit line edit where data is inserted.
 //! \param[in] text inserted data. All non-hex chars ignored.
 void paste(QLineEdit *edit, const QString& text);
 //! \fn QLineEdit * APDUUtility::move(QLineEdit *edit, MOVE direction)
 //! \brief Provides move cursor in APDU command fields.
 //! \param[in] edit line edit where cursor is moved.
 //! \param[in] direction the direction of movement of the cursor.
 QLineEdit * move(QLineEdit *edit, MOVE direction);
 Ui::APDUUtilityClass ui;//!< Qt inner ui-class
 QScopedPointer<Smartcards::WinSCard> cardIface{new Smartcards::WinSCard};//!< Scoped pointer to Smart Card Interface
 QScopedPointer<QStandardItemModel> APDUCommandsListModel{new QStandardItemModel};//! Scoped pointer to QStandardItemModel for APDU commands list
 int tId{ 0 };//!< Qt timer identificator
 int lastVendorIndex{ -1 };//!< index of last selected vendor in combo box
 Smartcards::SCOPE defaultScope{ Smartcards::User };//!< Default scope for EstablishContext. Reading from settings.
 Smartcards::SHARE defaultShare{ Smartcards::Shared };//!< Default share mode for Connect. Reading from settings.
 Smartcards::PROTOCOL defaultProtocol{ Smartcards::T0orT1 };//!< Default protocol for Connect. Reading from settings.
 bool commandChaged{ false };//!< APDU command fields changed flag.
};

#endif // APDUUTILITY_H
