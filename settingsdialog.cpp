/**
 * \file settingsdialog.cpp
 *
 * \section LICENSE
 *
 * Copyright (C) 2015-present Thorsten Roth
 *
 * This file is part of StackAndConquer.
 *
 * StackAndConquer is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StackAndConquer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with StackAndConquer.  If not, see <https://www.gnu.org/licenses/>.
 *
 * \section DESCRIPTION
 * Settings dialog.
 */

#include "./settingsdialog.h"

#include <QColorDialog>
#include <QDebug>
#include <QDirIterator>
#include <QIcon>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QSpinBox>

#include "ui_settings.h"

SettingsDialog::SettingsDialog(QWidget *pParent, const QString &userDataDir)
    : QDialog(pParent),
      m_pUi(new Ui::SettingsDialog()),
      m_pSettings(Settings::instance()) {
  m_pUi->setupUi(this);
  this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
  this->setModal(true);

  m_pUi->cbGuiLanguage->addItems(this->searchTranslations());

  m_pUi->spinNumOfPlayers->setMaximum(m_pSettings->getMaxNumOfPlayers());
  connect(m_pUi->spinNumOfPlayers,
          static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
          &SettingsDialog::changeNumOfPlayers);
  connect(m_pUi->spinNumOfPlayers,
          static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
          &SettingsDialog::changedSettings);
  // TODO(x): Remove after implementation of > 2 players
  if (m_pSettings->getMaxNumOfPlayers() < 3) {
    m_pUi->spinNumOfPlayers->setVisible(false);
    m_pUi->lblNumOfPlayers->setVisible(false);
  }

  connect(m_pUi->spinTowersToWin,
          static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
          &SettingsDialog::changedSettings);
  connect(m_pUi->cbStartPlayer, &QComboBox::currentTextChanged, this,
          &SettingsDialog::changedSettings);
  connect(m_pUi->cbBoard, &QComboBox::currentTextChanged, this,
          &SettingsDialog::changedSettings);

  connect(m_pUi->buttonBox, &QDialogButtonBox::accepted, this,
          &SettingsDialog::accept);
  connect(m_pUi->buttonBox, &QDialogButtonBox::rejected, this,
          &SettingsDialog::reject);

  for (int i = m_pSettings->getMaxNumOfPlayers() - 1; i >= 0; i--) {
    m_listColorLbls.push_front(
        new QLabel(tr("Color player %1").arg(QString::number(i + 1))));
    m_listColorEdit.push_front(new QLineEdit(this));
    m_pUi->formLayout->insertRow(2, m_listColorLbls.first(),
                                 m_listColorEdit.first());
    m_listColorEdit.first()->installEventFilter(this);
    connect(m_listColorEdit.first(), &QLineEdit::textChanged, this,
            &SettingsDialog::changedSettings);

    m_listHumCpuLbls.push_front(
        new QLabel(tr("Player %1 Human/CPU").arg(QString::number(i + 1))));
    m_listPlayerCombo.push_front(new QComboBox(this));
    m_pUi->formLayout->insertRow(2, m_listHumCpuLbls.first(),
                                 m_listPlayerCombo.first());
    connect(m_listPlayerCombo.first(), &QComboBox::currentTextChanged, this,
            &SettingsDialog::changedSettings);
  }

  this->searchCpuScripts(userDataDir);
  this->searchBoards(userDataDir);
  this->readSettings();
  this->searchBoardStyles(m_pSettings->getConfigPath());
}

SettingsDialog::~SettingsDialog() {
  if (m_pUi) {
    delete m_pUi;
    m_pUi = nullptr;
  }
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void SettingsDialog::showEvent(QShowEvent *pEvent) {
  this->readSettings();
  m_bSettingChanged = false;
  m_pUi->tabWidget->setCurrentIndex(0);
  QDialog::showEvent(pEvent);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

auto SettingsDialog::searchTranslations() const -> QStringList {
  QStringList sList;

  // Translations build in resources
  QDirIterator it(QStringLiteral(":"), QStringList() << QStringLiteral("*.qm"),
                  QDir::NoDotAndDotDot | QDir::Files);
  while (it.hasNext()) {
    it.next();
    QString sTmp = it.fileName();
    // qDebug() << sTmp;

    if (sTmp.startsWith(qApp->applicationName().toLower() + "_") &&
        sTmp.endsWith(QStringLiteral(".qm"))) {
      sList << sTmp.remove(qApp->applicationName().toLower() + "_")
                   .remove(QStringLiteral(".qm"));
    }
  }

  // Check for additional translation files in share folder
  QDirIterator it2(m_pSettings->getSharePath() + QStringLiteral("/lang"),
                   QStringList() << QStringLiteral("*.qm"),
                   QDir::NoDotAndDotDot | QDir::Files);
  while (it2.hasNext()) {
    it2.next();
    QString sTmp = it2.fileName();
    // qDebug() << sTmp;

    if (sTmp.startsWith(qApp->applicationName().toLower() + "_")) {
      sTmp = sTmp.remove(qApp->applicationName().toLower() + "_")
                 .remove(QStringLiteral(".qm"));
      if (!sList.contains(sTmp)) {
        sList << sTmp;
      }
    }
  }

  sList.sort();
  sList.push_front(QStringLiteral("auto"));
  return sList;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void SettingsDialog::searchCpuScripts(const QString &sUserDataDir) {
  QDir cpuDir = m_pSettings->getSharePath();

  QHash<QString, QString> CpuScripts;
  CpuScripts[QStringLiteral("Human")] = "";
  QString sIcon(QStringLiteral(":/img/user.png"));
  if (this->window()->palette().window().color().lightnessF() < 0.5) {
    sIcon = QStringLiteral(":/img/user2.png");
  }
  for (int i = 0; i < m_listPlayerCombo.size(); i++) {
    m_listPlayerCombo[i]->addItem(QIcon(sIcon), QStringLiteral("Human"));
  }

  QString sStrength;
  // Cpu scripts in share folder
  if (cpuDir.cd(QStringLiteral("cpu"))) {
    const QFileInfoList listFiles(cpuDir.entryInfoList(QDir::Files));
    sIcon = QStringLiteral(":/img/computer.png");
    if (this->window()->palette().window().color().lightnessF() < 0.5) {
      sIcon = QStringLiteral(":/img/computer2.png");
    }
    for (const auto &file : listFiles) {
      if (Settings::CPU_FILE_EXT == "." + file.suffix().toLower()) {
        sStrength = this->getCpuStrength(file.absoluteFilePath());
        CpuScripts[file.baseName() + sStrength] = file.absoluteFilePath();
        for (int i = 0; i < m_listPlayerCombo.size(); i++) {
          m_listPlayerCombo[i]->addItem(QIcon(sIcon),
                                        file.baseName() + sStrength);
        }
      }
    }
  }

  // Cpu scripts in user folder
  cpuDir.setPath(sUserDataDir);
  if (cpuDir.cd(QStringLiteral("cpu"))) {
    const QFileInfoList listFiles(cpuDir.entryInfoList(QDir::Files));
    sIcon = QStringLiteral(":/img/code.png");
    if (this->window()->palette().window().color().lightnessF() < 0.5) {
      sIcon = QStringLiteral(":/img/code2.png");
    }
    for (const auto &file : listFiles) {
      if (Settings::CPU_FILE_EXT == "." + file.suffix().toLower()) {
        if (-1 != m_listPlayerCombo[0]->findText(file.baseName())) {
          qWarning() << "Duplicate CPU script name found, skipping script"
                     << file.absoluteFilePath();
          continue;
        }

        sStrength = this->getCpuStrength(file.absoluteFilePath());
        CpuScripts[file.baseName() + sStrength] = file.absoluteFilePath();
        for (int i = 0; i < m_listPlayerCombo.size(); i++) {
          m_listPlayerCombo[i]->addItem(QIcon(sIcon),
                                        file.baseName() + sStrength);
        }
      }
    }
  }

  m_pSettings->setAvailableCpuScripts(CpuScripts);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

auto SettingsDialog::getCpuStrength(const QString &sFilename) -> QString {
  QString sStrength("");
  QFile scriptFile(sFilename);
  if (scriptFile.open(QIODevice::ReadOnly)) {
    QTextStream in(&scriptFile);
    QString sLine;
    while (in.readLineInto(&sLine)) {
      if (sLine.trimmed().startsWith(("// CPU strength:")) ||
          sLine.trimmed().startsWith(("* CPU strength:"))) {
        sLine = sLine.remove("// CPU strength:")
                    .remove("* CPU strength:")
                    .trimmed();
        if (!sLine.isEmpty()) {
          sStrength = " (" + sLine + ")";
        }
        break;
      }
    }
    scriptFile.close();
  }

  return sStrength;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void SettingsDialog::searchBoards(const QString &sUserDataDir) {
  QDir boardsDir = m_pSettings->getSharePath();

  m_sListBoards.clear();
  // Boards in share folder
  if (boardsDir.cd(QStringLiteral("boards"))) {
    const QFileInfoList listFiles(boardsDir.entryInfoList(QDir::Files));
    for (const auto &file : listFiles) {
      if (Settings::BOARD_FILE_EXT == "." + file.suffix().toLower()) {
        m_sListBoards << file.absoluteFilePath();
        m_pUi->cbBoard->addItem(file.baseName());
      }
    }
  }

  // Boards in user folder
  boardsDir.setPath(sUserDataDir);
  if (boardsDir.cd(QStringLiteral("boards"))) {
    const QFileInfoList listFiles(boardsDir.entryInfoList(QDir::Files));
    QString sIcon = QStringLiteral(":/img/code.png");
    if (this->window()->palette().window().color().lightnessF() < 0.5) {
      sIcon = QStringLiteral(":/img/code2.png");
    }
    for (const auto &file : listFiles) {
      if (Settings::BOARD_FILE_EXT == "." + file.suffix().toLower()) {
        if (-1 != m_pUi->cbBoard->findText(file.baseName())) {
          qWarning() << "Duplicate baord name found, skipping board"
                     << file.absoluteFilePath();
          continue;
        }

        m_sListBoards << file.absoluteFilePath();
        m_pUi->cbBoard->addItem(QIcon(sIcon), file.baseName());
      }
    }
  }

  if (m_sListBoards.size() < 2) {
    m_pUi->cbBoard->setEnabled(false);
  }
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void SettingsDialog::searchBoardStyles(const QString &sStyleDir) {
  QStringList sListStyleFiles;
  QDir stylesDir(sStyleDir);
  const QFileInfoList fiListFiles(
      stylesDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files));
  for (const auto &fi : fiListFiles) {
    if (fi.fileName().endsWith("-style" + Settings::CONF_FILE_EXT)) {
      sListStyleFiles << fi.fileName().remove(Settings::CONF_FILE_EXT);
    }
  }
  sListStyleFiles.push_front(tr("Create new style..."));
  m_pUi->cbBoardStyle->addItems(sListStyleFiles);
  m_pUi->cbBoardStyle->insertSeparator(1);

  m_pUi->cbBoardStyle->setCurrentIndex(
      m_pUi->cbBoardStyle->findText(m_pSettings->getBoardStyleFile()));

  connect(
      m_pUi->cbBoardStyle,
      static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
      this, &SettingsDialog::changedStyle);
  connect(m_pUi->tableBoardStyle, &QTableWidget::cellDoubleClicked, this,
          &SettingsDialog::clickedStyleCell);

  this->fillStyleTable();
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void SettingsDialog::fillStyleTable() {
  m_pUi->tableBoardStyle->item(0, 0)->setText(m_pSettings->getBgColor().name());
  m_pUi->tableBoardStyle->item(0, 0)->setBackground(
      QBrush(m_pSettings->getBgColor()));

  m_pUi->tableBoardStyle->item(1, 0)->setText(
      m_pSettings->getBgBoardColor().name());
  m_pUi->tableBoardStyle->item(1, 0)->setBackground(
      QBrush(m_pSettings->getBgBoardColor()));

  m_pUi->tableBoardStyle->item(2, 0)->setText(
      m_pSettings->getGridBoardColor().name());
  m_pUi->tableBoardStyle->item(2, 0)->setBackground(
      QBrush(m_pSettings->getGridBoardColor()));

  m_pUi->tableBoardStyle->item(3, 0)->setText(
      m_pSettings->getAnimateColor().name());
  m_pUi->tableBoardStyle->item(3, 0)->setBackground(
      QBrush(m_pSettings->getAnimateColor()));

  m_pUi->tableBoardStyle->item(4, 0)->setText(
      m_pSettings->getAnimateBorderColor().name());
  m_pUi->tableBoardStyle->item(4, 0)->setBackground(
      QBrush(m_pSettings->getAnimateBorderColor()));

  m_pUi->tableBoardStyle->item(5, 0)->setText(
      m_pSettings->getHighlightColor().name());
  m_pUi->tableBoardStyle->item(5, 0)->setBackground(
      QBrush(m_pSettings->getHighlightColor()));

  m_pUi->tableBoardStyle->item(6, 0)->setText(
      m_pSettings->getHighlightBorderColor().name());
  m_pUi->tableBoardStyle->item(6, 0)->setBackground(
      QBrush(m_pSettings->getHighlightBorderColor()));

  m_pUi->tableBoardStyle->item(7, 0)->setText(
      m_pSettings->getNeighboursColor().name());
  m_pUi->tableBoardStyle->item(7, 0)->setBackground(
      QBrush(m_pSettings->getNeighboursColor()));

  m_pUi->tableBoardStyle->item(8, 0)->setText(
      m_pSettings->getNeighboursBorderColor().name());
  m_pUi->tableBoardStyle->item(8, 0)->setBackground(
      QBrush(m_pSettings->getNeighboursBorderColor()));

  m_pUi->tableBoardStyle->item(9, 0)->setText(
      m_pSettings->getSelectedColor().name());
  m_pUi->tableBoardStyle->item(9, 0)->setBackground(
      QBrush(m_pSettings->getSelectedColor()));

  m_pUi->tableBoardStyle->item(10, 0)->setText(
      m_pSettings->getSelectedBorderColor().name());
  m_pUi->tableBoardStyle->item(10, 0)->setBackground(
      QBrush(m_pSettings->getSelectedBorderColor()));

  m_pUi->tableBoardStyle->item(11, 0)->setText(
      m_pSettings->getTextColor().name());
  m_pUi->tableBoardStyle->item(11, 0)->setBackground(
      QBrush(m_pSettings->getTextColor()));

  m_pUi->tableBoardStyle->item(12, 0)->setText(
      m_pSettings->getTextHighlightColor().name());
  m_pUi->tableBoardStyle->item(12, 0)->setBackground(
      QBrush(m_pSettings->getTextHighlightColor()));
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void SettingsDialog::accept() {
  QString sOldValue = m_pSettings->getGuiLanguage();
  QString sNewValue = m_pUi->cbGuiLanguage->currentText();
  m_pSettings->setGuiLanguage(sNewValue);
  if (sOldValue != sNewValue) {
    emit changeLang(m_pSettings->getGuiLanguage());
  }

  m_pSettings->setShowPossibleMoveTowers(
      m_pUi->checkShowPossibleMoves->isChecked());

  // Save style before check for changed settings
  sOldValue = m_pSettings->getBoardStyleFile();
  sNewValue = m_pUi->cbBoardStyle->currentText();
  if (sOldValue != sNewValue) this->changedSettings();
  m_pSettings->setBoardStyleFile(sNewValue);
  this->saveBoardStyle();

  int nRet = QMessageBox::No;
  if (m_bSettingChanged) {
    nRet = QMessageBox::question(this, this->windowTitle(),
                                 tr("Main game settings had been changed.<br>"
                                    "Do you want to start a new game?"));
    if (nRet != QMessageBox::Yes) {
      this->readSettings();
      QDialog::accept();
      return;
    }
  }

  // General
  m_pSettings->setNumOfPlayers(m_pUi->spinNumOfPlayers->value());
  m_pSettings->setStartPlayer(m_pUi->cbStartPlayer->currentIndex());
  m_pSettings->setTowersToWin(m_pUi->spinTowersToWin->value());
  m_pSettings->setBoardFile(m_sListBoards.at(m_pUi->cbBoard->currentIndex()));

  // Players
  for (quint8 nPlayer = 0; nPlayer < m_pSettings->getMaxNumOfPlayers();
       nPlayer++) {
    m_pSettings->setPlayerCpuScript(nPlayer,
                                    m_listPlayerCombo[nPlayer]->currentText());
    m_pSettings->setPlayerColor(nPlayer, m_listColorEdit[nPlayer]->text());
  }

  if (nRet == QMessageBox::Yes) {
    emit this->newGame(QString());
  }

  QDialog::accept();
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void SettingsDialog::reject() {
  this->readSettings();
  QDialog::reject();
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void SettingsDialog::readSettings() {
  if (-1 != m_pUi->cbGuiLanguage->findText(m_pSettings->getGuiLanguage())) {
    m_pUi->cbGuiLanguage->setCurrentIndex(
        m_pUi->cbGuiLanguage->findText(m_pSettings->getGuiLanguage()));
  } else {
    m_pUi->cbGuiLanguage->setCurrentIndex(
        m_pUi->cbGuiLanguage->findText(QStringLiteral("auto")));
    m_pSettings->setGuiLanguage(m_pUi->cbGuiLanguage->currentText());
  }

  m_pUi->spinNumOfPlayers->setValue(m_pSettings->getNumOfPlayers());
  this->changeNumOfPlayers();

  for (quint8 nPlayer = 0; nPlayer < m_pSettings->getMaxNumOfPlayers();
       nPlayer++) {
    // Human / CPU script
    QString sValue = m_pSettings->getPlayerCpuScript(nPlayer);
    if (-1 != m_listPlayerCombo[nPlayer]->findText(sValue)) {
      m_listPlayerCombo[nPlayer]->setCurrentIndex(
          m_listPlayerCombo[nPlayer]->findText(sValue));
    } else {
      m_listPlayerCombo[nPlayer]->setCurrentIndex(
          m_listPlayerCombo[nPlayer]->findText(QStringLiteral("Human")));
      m_pSettings->setPlayerCpuScript(nPlayer, QStringLiteral("Human"));
    }

    // Player color
    sValue = m_pSettings->getPlayerColor(nPlayer);
    m_listColorEdit[nPlayer]->setText(sValue);
    QPalette *palette = new QPalette();
    palette->setColor(QPalette::Base, sValue);
    m_listColorEdit.at(nPlayer)->setPalette(*palette);
  }

  this->updateStartPlayerCombo();
  m_pUi->spinTowersToWin->setValue(m_pSettings->getTowersToWin());
  m_pUi->checkShowPossibleMoves->setChecked(
      m_pSettings->getShowPossibleMoveTowers());

  QString sBoard = m_pSettings->getBoardFile();
  if (m_sListBoards.contains(sBoard)) {
    m_pUi->cbBoard->setCurrentIndex(m_sListBoards.indexOf(sBoard));
  } else {
    if (!m_sListBoards.isEmpty()) {
      m_pSettings->setBoardFile(m_sListBoards.at(0));
      m_pUi->cbBoard->setCurrentIndex(0);
    } else {
      QMessageBox::warning(this, tr("Error"), tr("Boards folder seems empty!"));
      qWarning() << "Boards folder(s) empty (share folder and user folder)!";
      return;
    }
  }

  m_pUi->cbBoardStyle->setCurrentIndex(
      m_pUi->cbBoardStyle->findText(m_pSettings->getBoardStyleFile()));
  m_pSettings->loadBoardStyle(m_pSettings->getBoardStyleFile());
  this->fillStyleTable();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void SettingsDialog::changedStyle(int nIndex) {
  QString sFileName(QLatin1String(""));

  if (0 == nIndex) {  // Create new style
    bool bOk;
    sFileName =
        QInputDialog::getText(this, tr("New style"),
                              tr("Please insert name of "
                                 "new style file:"),
                              QLineEdit::Normal, QLatin1String(""), &bOk);
    if (!bOk || sFileName.isEmpty()) {
      // Reset selection
      m_pUi->cbBoardStyle->setCurrentIndex(
          m_pUi->cbBoardStyle->findText(m_pSettings->getBoardStyleFile()));
      return;
    }

    if (!sFileName.toLower().endsWith(QStringLiteral("-style"))) {
      sFileName += QStringLiteral("-style");
    }

    if (m_pUi->cbBoardStyle->findText(sFileName) !=
        -1) {  // Style already exists
      // Reset selection
      m_pUi->cbBoardStyle->setCurrentIndex(
          m_pUi->cbBoardStyle->findText(m_pSettings->getBoardStyleFile()));

      QMessageBox::warning(this, tr("Error"), tr("File already exists."));
      qWarning() << "Style file already exists:" << sFileName;
      return;
    }

    bOk = m_pSettings->createNewBoardStyleFile(sFileName);
    if (!bOk) {
      // Reset selection
      m_pUi->cbBoardStyle->setCurrentIndex(
          m_pUi->cbBoardStyle->findText(m_pSettings->getBoardStyleFile()));

      QMessageBox::warning(this, tr("Error"),
                           tr("Could not create new style."));
      return;
    }
    m_pUi->cbBoardStyle->addItem(sFileName);
    m_pUi->cbBoardStyle->setCurrentIndex(
        m_pUi->cbBoardStyle->findText(sFileName));
  } else {  // Load existing style file
    sFileName = m_pUi->cbBoardStyle->currentText();
  }

  m_pSettings->loadBoardStyle(sFileName);
  this->fillStyleTable();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void SettingsDialog::clickedStyleCell(int nRow, int nCol) {
  if (0 == nCol) {
    QColor initialColor(m_pUi->tableBoardStyle->item(nRow, nCol)->text());
    QColor newColor = QColorDialog::getColor(initialColor);
    if (newColor.isValid()) {
      m_pUi->tableBoardStyle->item(nRow, nCol)->setText(newColor.name());
      m_pUi->tableBoardStyle->item(nRow, nCol)->setBackground(QBrush(newColor));
    } else if (newColor.name().isEmpty()) {
      m_pUi->tableBoardStyle->item(nRow, nCol)->setText(QLatin1String(""));
    }
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

bool SettingsDialog::eventFilter(QObject *pObj, QEvent *pEvent) {
  for (int i = 0; i < m_listColorEdit.size(); i++) {
    if (pObj == m_listColorEdit.at(i) &&
        pEvent->type() == QEvent::MouseButtonPress) {
      QColor newColor = QColorDialog::getColor(m_listColorEdit.at(i)->text());
      if (newColor.isValid()) {
        m_listColorEdit.at(i)->setText(newColor.name());
        QPalette *palette = new QPalette();
        palette->setColor(QPalette::Base, newColor);
        m_listColorEdit.at(i)->setPalette(*palette);
      }
      break;
    }
  }
  return QDialog::eventFilter(pObj, pEvent);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void SettingsDialog::saveBoardStyle() {
  m_pSettings->setBgColor(
      this->getStyleColorFromTable(m_pSettings->getBgColor(), 0));

  m_pSettings->setBgBoardColor(
      this->getStyleColorFromTable(m_pSettings->getBgBoardColor(), 1));

  m_pSettings->setGridBoardColor(
      this->getStyleColorFromTable(m_pSettings->getGridBoardColor(), 2));

  m_pSettings->setAnimateColor(
      this->getStyleColorFromTable(m_pSettings->getAnimateColor(), 3));

  m_pSettings->setAnimateBorderColor(
      this->getStyleColorFromTable(m_pSettings->getAnimateBorderColor(), 4));

  m_pSettings->setHighlightColor(
      this->getStyleColorFromTable(m_pSettings->getHighlightColor(), 5));

  m_pSettings->setHighlightBorderColor(
      this->getStyleColorFromTable(m_pSettings->getHighlightBorderColor(), 6));

  m_pSettings->setNeighboursColor(
      this->getStyleColorFromTable(m_pSettings->getNeighboursColor(), 7));

  m_pSettings->setNeighboursBorderColor(
      this->getStyleColorFromTable(m_pSettings->getNeighboursBorderColor(), 8));

  m_pSettings->setSelectedColor(
      this->getStyleColorFromTable(m_pSettings->getSelectedColor(), 9));

  m_pSettings->setSelectedBorderColor(
      this->getStyleColorFromTable(m_pSettings->getSelectedBorderColor(), 10));

  m_pSettings->setTextColor(
      this->getStyleColorFromTable(m_pSettings->getTextColor(), 11));

  m_pSettings->setTextHighlightColor(
      this->getStyleColorFromTable(m_pSettings->getTextHighlightColor(), 12));

  m_pSettings->saveBoardStyle();
}

auto SettingsDialog::getStyleColorFromTable(QColor color, const int nRow)
    -> QColor {
  QColor cTmp = color;
#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
  oldColor.setNamedColor(m_pUi->tableBoardStyle->item(nRow, 0)->text());
#else
  color = QColor::fromString(m_pUi->tableBoardStyle->item(nRow, 0)->text());
#endif
  if (cTmp != color) this->changedSettings();

  return color;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void SettingsDialog::updateUiLang() {
  m_pUi->retranslateUi(this);

  // Widgets, which had not been created through UI have to be handled manually
  for (int i = 0; i < m_pSettings->getMaxNumOfPlayers(); i++) {
    m_listColorLbls[i]->setText(
        tr("Color player %1").arg(QString::number(i + 1)));
    m_listHumCpuLbls[i]->setText(
        tr("Player %1 Human/CPU").arg(QString::number(i + 1)));
  }

  m_pUi->cbBoardStyle->setItemText(0, tr("Create new style..."));

  QStringList sListHeader;
  sListHeader << tr("Color");
  m_pUi->tableBoardStyle->setHorizontalHeaderLabels(sListHeader);
  sListHeader.clear();
  sListHeader << tr("Main background") << tr("Board background")
              << tr("Grid color") << tr("Animate color")
              << tr("Animate border color") << tr("Highlight color")
              << tr("Highlight border color") << tr("Neighbours color")
              << tr("Neighbours border color") << tr("Selected color")
              << tr("Selected border color") << tr("Text color")
              << tr("Text highlight color");
  m_pUi->tableBoardStyle->setVerticalHeaderLabels(sListHeader);

  this->updateStartPlayerCombo();
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void SettingsDialog::changeNumOfPlayers() {
  for (int i = 0; i < m_pSettings->getMaxNumOfPlayers(); i++) {
    if (i < m_pUi->spinNumOfPlayers->value()) {
      m_listColorLbls[i]->setVisible(true);
      m_listColorEdit[i]->setVisible(true);
      m_listHumCpuLbls[i]->setVisible(true);
      m_listPlayerCombo[i]->setVisible(true);
    } else {
      m_listColorLbls[i]->setVisible(false);
      m_listColorEdit[i]->setVisible(false);
      m_listHumCpuLbls[i]->setVisible(false);
      m_listPlayerCombo[i]->setVisible(false);
    }
  }
  this->updateStartPlayerCombo();
  this->adjustSize();
}

void SettingsDialog::updateStartPlayerCombo() {
  QStringList sListStartPlayer;
  sListStartPlayer.reserve(m_pUi->spinNumOfPlayers->value() + 1);
  sListStartPlayer << tr("Random");
  for (int i = 1; i <= m_pUi->spinNumOfPlayers->value(); i++) {
    sListStartPlayer << tr("Player") + " " + QString::number(i);
  }
  m_pUi->cbStartPlayer->clear();
  m_pUi->cbStartPlayer->addItems(sListStartPlayer);

  if (m_pSettings->getStartPlayer() < m_pUi->cbStartPlayer->count()) {
    m_pUi->cbStartPlayer->setCurrentIndex(m_pSettings->getStartPlayer());
  } else {
    m_pSettings->setStartPlayer(1);
    m_pUi->cbStartPlayer->setCurrentIndex(m_pSettings->getStartPlayer());
  }
}

void SettingsDialog::changedSettings() { m_bSettingChanged = true; }
