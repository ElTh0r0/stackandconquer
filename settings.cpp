/**
 * \file settings.cpp
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

#include "./settings.h"

#include <QColorDialog>
#include <QDebug>
#include <QDirIterator>
#include <QIcon>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QSettings>
#include <QSpinBox>

#include "ui_settings.h"

Settings::Settings(const QString &sSharePath, const QString &userDataDir,
                   const QString &sBoardExtension, const quint8 nMaxPlayers,
                   QWidget *pParent)
    : m_pUi(new Ui::SettingsDialog()),
      m_sSharePath(sSharePath),
      m_sBoardExtension(sBoardExtension),
      m_sCpuExtension(QStringLiteral(".js")),
      m_nDefaultGrid(70),  // Default stone SVG size fits to grid size of 70!
      m_nMaxGrid(200),
      m_nMaxPlayers(nMaxPlayers),
      m_DefaultPlayerColors{"#EF2929", "#FCAF3E", "#729FCF", "#8F5902"} {
  Q_UNUSED(pParent)
  m_pUi->setupUi(this);
  this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
  this->setModal(true);

#if defined __linux__
  m_pSettings = new QSettings(QSettings::NativeFormat, QSettings::UserScope,
                              qApp->applicationName().toLower(),
                              qApp->applicationName().toLower());
  m_sExt = QStringLiteral(".conf");
#else
  m_pSettings = new QSettings(QSettings::IniFormat, QSettings::UserScope,
                              qApp->applicationName().toLower(),
                              qApp->applicationName().toLower());
  m_sExt = QStringLiteral(".ini");
#endif

  this->copyDefaultStyles();
  m_pUi->cbGuiLanguage->addItems(this->searchTranslations());

  m_pUi->spinNumOfPlayers->setMaximum(m_nMaxPlayers);
  connect(m_pUi->spinNumOfPlayers,
          static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
          &Settings::changeNumOfPlayers);
  connect(m_pUi->spinNumOfPlayers,
          static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
          &Settings::changedSettings);
  // TODO(x): Remove after implementation of > 2 players
  if (m_nMaxPlayers < 3) {
    m_pUi->spinNumOfPlayers->setVisible(false);
    m_pUi->lblNumOfPlayers->setVisible(false);
  }

  connect(m_pUi->spinTowersToWin,
          static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
          &Settings::changedSettings);
  connect(m_pUi->cbStartPlayer, &QComboBox::currentTextChanged, this,
          &Settings::changedSettings);
  connect(m_pUi->cbBoard, &QComboBox::currentTextChanged, this,
          &Settings::changedSettings);

  connect(m_pUi->buttonBox, &QDialogButtonBox::accepted, this,
          &Settings::accept);
  connect(m_pUi->buttonBox, &QDialogButtonBox::rejected, this,
          &Settings::reject);

  for (int i = m_nMaxPlayers - 1; i >= 0; i--) {
    m_listColorLbls.push_front(
        new QLabel(tr("Color player %1").arg(QString::number(i + 1))));
    m_listColorEdit.push_front(new QLineEdit(this));
    m_pUi->formLayout->insertRow(2, m_listColorLbls.first(),
                                 m_listColorEdit.first());
    m_listColorEdit.first()->installEventFilter(this);
    connect(m_listColorEdit.first(), &QLineEdit::textChanged, this,
            &Settings::changedSettings);

    m_listHumCpuLbls.push_front(
        new QLabel(tr("Player %1 Human/CPU").arg(QString::number(i + 1))));
    m_listPlayerCombo.push_front(new QComboBox(this));
    m_pUi->formLayout->insertRow(2, m_listHumCpuLbls.first(),
                                 m_listPlayerCombo.first());
    connect(m_listPlayerCombo.first(), &QComboBox::currentTextChanged, this,
            &Settings::changedSettings);
  }

  this->searchCpuScripts(userDataDir);
  this->searchBoards(userDataDir);
  this->readSettings();
  QFileInfo fi(m_pSettings->fileName());
  this->searchBoardStyles(fi.absolutePath());
}

Settings::~Settings() {
  if (m_pUi) {
    delete m_pUi;
    m_pUi = nullptr;
  }
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void Settings::copyDefaultStyles() {
  QFileInfo fi(m_pSettings->fileName());
  QDir confDir(fi.absolutePath());
  if (!confDir.exists()) {
    confDir.mkpath(confDir.absolutePath());
  }

  QFile stylefile(confDir.absolutePath() + "/standard-style" + m_sExt);
  if (!stylefile.exists()) {
    if (QFile::copy(QStringLiteral(":/boardstyles/standard-style.conf"),
                    confDir.absolutePath() + "/standard-style" + m_sExt)) {
      stylefile.setPermissions(stylefile.permissions() |
                               QFileDevice::WriteUser);
    } else {
      qWarning() << "Couldn't create style file: " << stylefile.fileName();
    }
  }

  stylefile.setFileName(confDir.absolutePath() + "/dark-style" + m_sExt);
  if (!stylefile.exists()) {
    if (QFile::copy(QStringLiteral(":/boardstyles/dark-style.conf"),
                    confDir.absolutePath() + "/dark-style" + m_sExt)) {
      stylefile.setPermissions(stylefile.permissions() |
                               QFileDevice::WriteUser);
    } else {
      qWarning() << "Couldn't create style file: " << stylefile.fileName();
    }
  }
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void Settings::showEvent(QShowEvent *pEvent) {
  this->readSettings();
  m_bSettingChanged = false;
  m_pUi->tabWidget->setCurrentIndex(0);
  QDialog::showEvent(pEvent);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

auto Settings::searchTranslations() const -> QStringList {
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
  QDirIterator it2(m_sSharePath + QStringLiteral("/lang"),
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

void Settings::searchCpuScripts(const QString &sUserDataDir) {
  QDir cpuDir = m_sSharePath;

  m_sListCPUs.clear();
  m_sListCPUs << QStringLiteral("Human");
  QString sIcon(QStringLiteral(":/img/user.png"));
  if (this->window()->palette().window().color().lightnessF() < 0.5) {
    sIcon = QStringLiteral(":/img/user2.png");
  }
  for (int i = 0; i < m_listPlayerCombo.size(); i++) {
    m_listPlayerCombo[i]->addItem(QIcon(sIcon), m_sListCPUs.first());
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
      if (m_sCpuExtension == "." + file.suffix().toLower()) {
        sStrength = this->getCpuStrength(file.absoluteFilePath());
        m_sListCPUs << file.absoluteFilePath();
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
      if (m_sCpuExtension == "." + file.suffix().toLower()) {
        if (-1 != m_listPlayerCombo[0]->findText(file.baseName())) {
          qWarning() << "Duplicate CPU script name found, skipping script"
                     << file.absoluteFilePath();
          continue;
        }

        sStrength = this->getCpuStrength(file.absoluteFilePath());
        m_sListCPUs << file.absoluteFilePath();
        for (int i = 0; i < m_listPlayerCombo.size(); i++) {
          m_listPlayerCombo[i]->addItem(QIcon(sIcon),
                                        file.baseName() + sStrength);
        }
      }
    }
  }
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

auto Settings::getCpuStrength(const QString &sFilename) -> QString {
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
      }
    }
    scriptFile.close();
  }

  return sStrength;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void Settings::searchBoards(const QString &sUserDataDir) {
  QDir boardsDir = m_sSharePath;

  m_sListBoards.clear();
  // Boards in share folder
  if (boardsDir.cd(QStringLiteral("boards"))) {
    const QFileInfoList listFiles(boardsDir.entryInfoList(QDir::Files));
    for (const auto &file : listFiles) {
      if (m_sBoardExtension == "." + file.suffix().toLower()) {
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
      if (m_sBoardExtension == "." + file.suffix().toLower()) {
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

void Settings::searchBoardStyles(const QString &sStyleDir) {
  QStringList sListStyleFiles;
  QDir stylesDir(sStyleDir);
  const QFileInfoList fiListFiles(
      stylesDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files));
  for (const auto &fi : fiListFiles) {
    if (fi.fileName().endsWith("-style" + m_sExt)) {
      sListStyleFiles << fi.fileName().remove(m_sExt);
    }
  }
  sListStyleFiles.push_front(tr("Create new style..."));
  m_pUi->cbBoardStyle->addItems(sListStyleFiles);
  m_pUi->cbBoardStyle->insertSeparator(1);

  m_pUi->cbBoardStyle->setCurrentIndex(
      m_pUi->cbBoardStyle->findText(m_sBoardStyleFile));

  connect(
      m_pUi->cbBoardStyle,
      static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
      this, &Settings::changedStyle);
  connect(m_pUi->tableBoardStyle, &QTableWidget::cellDoubleClicked, this,
          &Settings::clickedStyleCell);

  this->loadBoardStyle(m_sBoardStyleFile);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void Settings::loadBoardStyle(const QString &sStyleFile) {
#if defined __linux__
  QSettings *pStyleSet =
      new QSettings(QSettings::NativeFormat, QSettings::UserScope,
                    qApp->applicationName().toLower(), sStyleFile);
#else
  QSettings *pStyleSet =
      new QSettings(QSettings::IniFormat, QSettings::UserScope,
                    qApp->applicationName().toLower(), sStyleFile);
#endif

  if (!QFile::exists(pStyleSet->fileName())) {
    qWarning() << "Could not find/open board style file:"
               << pStyleSet->fileName();
  }

  pStyleSet->beginGroup(QStringLiteral("Colors"));
  this->readStyle_SetTable(m_bgColor, pStyleSet, 0, QStringLiteral("BgColor"),
                           QStringLiteral("#EEEEEC"));

  this->readStyle_SetTable(m_bgBoardColor, pStyleSet, 1,
                           QStringLiteral("BgBoardColor"),
                           QStringLiteral("#FFFFFF"));

  this->readStyle_SetTable(m_gridBoardColor, pStyleSet, 2,
                           QStringLiteral("GridBoardColor"),
                           QStringLiteral("#888A85"));

  this->readStyle_SetTable(m_animateColor, pStyleSet, 3,
                           QStringLiteral("AnimateColor"),
                           QStringLiteral("#fce94f"));

  this->readStyle_SetTable(m_animateBorderColor, pStyleSet, 4,
                           QStringLiteral("AnimateBorderColor"),
                           QStringLiteral("#000000"));

  this->readStyle_SetTable(m_highlightColor, pStyleSet, 5,
                           QStringLiteral("HighlightColor"),
                           QStringLiteral("#8ae234"));

  this->readStyle_SetTable(m_highlightBorderColor, pStyleSet, 6,
                           QStringLiteral("HighlightBorderColor"),
                           QStringLiteral("#888A85"));

  this->readStyle_SetTable(m_neighboursColor, pStyleSet, 7,
                           QStringLiteral("NeighboursColor"),
                           QStringLiteral("#ad7fa8"));

  this->readStyle_SetTable(m_neighboursBorderColor, pStyleSet, 8,
                           QStringLiteral("NeighboursBorderColor"),
                           QStringLiteral("#000000"));

  this->readStyle_SetTable(m_selectedColor, pStyleSet, 9,
                           QStringLiteral("SelectedColor"),
                           QStringLiteral("#fce94f"));

  this->readStyle_SetTable(m_selectedBorderColor, pStyleSet, 10,
                           QStringLiteral("SelectedBorderColor"),
                           QStringLiteral("#000000"));

  this->readStyle_SetTable(m_txtColor, pStyleSet, 11,
                           QStringLiteral("TextColor"),
                           QStringLiteral("#000000"));

  this->readStyle_SetTable(m_txtHighColor, pStyleSet, 12,
                           QStringLiteral("TextHighlightColor"),
                           QStringLiteral("#FF0000"));
  pStyleSet->endGroup();
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void Settings::readStyle_SetTable(QColor &color, QSettings *pSet,
                                  const int nRow, const QString &sKey,
                                  const QString &sFallback) {
  color = this->readColor(pSet, sKey, sFallback);
  m_pUi->tableBoardStyle->item(nRow, 0)->setText(color.name());
  m_pUi->tableBoardStyle->item(nRow, 0)->setBackground(QBrush(color));
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void Settings::saveBoardStyle(const QString &sStyleFile) {
#if defined __linux__
  QSettings *pStyleSet =
      new QSettings(QSettings::NativeFormat, QSettings::UserScope,
                    qApp->applicationName().toLower(), sStyleFile);
#else
  QSettings *pStyleSet =
      new QSettings(QSettings::IniFormat, QSettings::UserScope,
                    qApp->applicationName().toLower(), sStyleFile);
#endif

  if (!QFile::exists(pStyleSet->fileName())) {
    qWarning() << "Could not find/open board style file:"
               << pStyleSet->fileName();
  }

  pStyleSet->beginGroup(QStringLiteral("Colors"));

  this->saveColor(m_bgColor, pStyleSet, 0, QStringLiteral("BgColor"));

  this->saveColor(m_bgBoardColor, pStyleSet, 1, QStringLiteral("BgBoardColor"));

  this->saveColor(m_gridBoardColor, pStyleSet, 2,
                  QStringLiteral("GridBoardColor"));

  this->saveColor(m_animateColor, pStyleSet, 3, QStringLiteral("AnimateColor"));

  this->saveColor(m_animateBorderColor, pStyleSet, 4,
                  QStringLiteral("AnimateBorderColor"));

  this->saveColor(m_highlightColor, pStyleSet, 5,
                  QStringLiteral("HighlightColor"));

  this->saveColor(m_highlightBorderColor, pStyleSet, 6,
                  QStringLiteral("HighlightBorderColor"));

  this->saveColor(m_neighboursColor, pStyleSet, 7,
                  QStringLiteral("NeighboursColor"));

  this->saveColor(m_neighboursBorderColor, pStyleSet, 8,
                  QStringLiteral("NeighboursBorderColor"));

  this->saveColor(m_selectedColor, pStyleSet, 9,
                  QStringLiteral("SelectedColor"));

  this->saveColor(m_selectedBorderColor, pStyleSet, 10,
                  QStringLiteral("SelectedBorderColor"));

  this->saveColor(m_txtColor, pStyleSet, 11, QStringLiteral("TextColor"));

  this->saveColor(m_txtHighColor, pStyleSet, 12,
                  QStringLiteral("TextHighlightColor"));

  pStyleSet->endGroup();
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void Settings::saveColor(QColor &color, QSettings *pSet, const int nRow,
                         const QString &sKey) {
  QColor cTmp = color;
#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
  color.setNamedColor(m_pUi->tableBoardStyle->item(nRow, 0)->text());
#else
  color = QColor::fromString(m_pUi->tableBoardStyle->item(nRow, 0)->text());
#endif
  if (cTmp != color) this->changedSettings();
  pSet->setValue(sKey, color.name());
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void Settings::accept() {
  QString sOldGuiLang = m_sGuiLanguage;
  m_sGuiLanguage = m_pUi->cbGuiLanguage->currentText();
  m_pSettings->setValue(QStringLiteral("GuiLanguage"), m_sGuiLanguage);

  if (sOldGuiLang != m_sGuiLanguage) {
    emit changeLang(this->getLanguage());
  }

  m_bShowPossibleMoveTowers = m_pUi->checkShowPossibleMoves->isChecked();
  m_pSettings->setValue(QStringLiteral("ShowPossibleMoveTowers"),
                        m_bShowPossibleMoveTowers);

  // Save style before check for changed settings
  QString sTmp(m_sBoardStyleFile);
  m_sBoardStyleFile = m_pUi->cbBoardStyle->currentText();
  if (sTmp != m_sBoardStyleFile) this->changedSettings();
  m_pSettings->setValue(QStringLiteral("BoardStyle"), m_sBoardStyleFile);
  this->saveBoardStyle(m_sBoardStyleFile);

  int nRet = QMessageBox::No;
  if (m_bSettingChanged) {
    nRet = QMessageBox::question(nullptr, this->windowTitle(),
                                 tr("Main game settings had been changed.<br>"
                                    "Do you want to start a new game?"));
    if (nRet != QMessageBox::Yes) {
      this->readSettings();
      QDialog::accept();
      return;
    }
  }

  // General
  m_nNumOfPlayers = m_pUi->spinNumOfPlayers->value();
  // TODO(x): Enable after implementation of > 2 players
  // m_pSettings->setValue(QStringLiteral("NumOfPlayers"), m_nNumOfPlayers);

  m_nStartPlayer = m_pUi->cbStartPlayer->currentIndex();
  m_pSettings->setValue(QStringLiteral("StartPlayer"), m_nStartPlayer);

  m_nTowersToWin = m_pUi->spinTowersToWin->value();
  m_pSettings->setValue(QStringLiteral("NumWinTowers"), m_nTowersToWin);

  m_sBoard = m_sListBoards.at(m_pUi->cbBoard->currentIndex());
  m_pSettings->setValue(QStringLiteral("Board"), m_sBoard);

  // Players
  for (int i = 0; i < m_Players.size(); i++) {
    m_Players[i].clear();
  }
  m_Players.clear();
  for (quint8 i = 0; i < m_nMaxPlayers; i++) {
    QMap<QString, QString> tmpMap;
    tmpMap[QStringLiteral("HumanCpu")] = m_listPlayerCombo[i]->currentText();

    QColor color(m_listColorEdit[i]->text());
    if (!color.isValid()) {
      qWarning() << "User chose invalid stone color:"
                 << m_listColorEdit[i]->text();
      if (i < m_DefaultPlayerColors.size()) {
#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
        color.setNamedColor(m_DefaultPlayerColors[i]);
#else
        color = QColor::fromString(m_DefaultPlayerColors[i]);
#endif
      } else {
        qWarning() << "Fallback player color missing!";
#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
        color.setNamedColor(m_DefaultPlayerColors[0]);
#else
        color = QColor::fromString(m_DefaultPlayerColors[0]);
#endif
      }
    }
    tmpMap[QStringLiteral("Color")] = color.name();
    m_Players << tmpMap;

    m_pSettings->beginGroup("Player" + QString::number(i + 1));
    m_pSettings->setValue(QStringLiteral("HumanCpu"),
                          tmpMap[QStringLiteral("HumanCpu")]);
    m_pSettings->setValue(QStringLiteral("Color"),
                          tmpMap[QStringLiteral("Color")]);
    m_pSettings->remove(QStringLiteral("Name"));
    m_pSettings->endGroup();
  }

  // Remove deprecated settings
  m_pSettings->remove(QStringLiteral("Colors"));
  m_pSettings->remove(QStringLiteral("NameP1"));
  m_pSettings->remove(QStringLiteral("NameP2"));
  m_pSettings->remove(QStringLiteral("P1HumanCpu"));
  m_pSettings->remove(QStringLiteral("P2HumanCpu"));

  if (nRet == QMessageBox::Yes) {
    emit this->newGame(QString());
  }

  QDialog::accept();
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void Settings::reject() {
  this->readSettings();
  QDialog::reject();
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void Settings::readSettings() {
  m_sGuiLanguage =
      m_pSettings->value(QStringLiteral("GuiLanguage"), QStringLiteral("auto"))
          .toString();
  if (-1 != m_pUi->cbGuiLanguage->findText(m_sGuiLanguage)) {
    m_pUi->cbGuiLanguage->setCurrentIndex(
        m_pUi->cbGuiLanguage->findText(m_sGuiLanguage));
  } else {
    m_pUi->cbGuiLanguage->setCurrentIndex(
        m_pUi->cbGuiLanguage->findText(QStringLiteral("auto")));
  }
  m_sGuiLanguage = m_pUi->cbGuiLanguage->currentText();

  m_nGridSize =
      m_pSettings->value(QStringLiteral("GridSize"), m_nDefaultGrid).toInt();
  if (m_nGridSize < m_nDefaultGrid) {
    m_nGridSize = m_nDefaultGrid;
  } else if (m_nGridSize > m_nMaxGrid) {
    m_nGridSize = m_nMaxGrid;
  }
  m_sBoardStyleFile = m_pSettings
                          ->value(QStringLiteral("BoardStyle"),
                                  QStringLiteral("standard-style"))
                          .toString();

  m_nNumOfPlayers =
      m_pSettings->value(QStringLiteral("NumOfPlayers"), 2).toInt();
  if (m_nNumOfPlayers > m_nMaxPlayers) {
    m_nNumOfPlayers = m_nMaxPlayers;
  }
  m_pUi->spinTowersToWin->setValue(m_nNumOfPlayers);
  this->changeNumOfPlayers();

  for (int i = 0; i < m_Players.size(); i++) {
    m_Players[i].clear();
  }
  m_Players.clear();

  for (quint8 i = 0; i < m_nMaxPlayers; i++) {
    m_pSettings->beginGroup("Player" + QString::number(i + 1));
    QMap<QString, QString> map;
    map[QStringLiteral("HumanCpu")] =
        m_pSettings->value(QStringLiteral("HumanCpu"), QStringLiteral("Human"))
            .toString();

    if (-1 != m_listPlayerCombo[i]->findText(map[QStringLiteral("HumanCpu")])) {
      m_listPlayerCombo[i]->setCurrentIndex(
          m_listPlayerCombo[i]->findText(map[QStringLiteral("HumanCpu")]));
    } else {
      m_listPlayerCombo[i]->setCurrentIndex(
          m_listPlayerCombo[i]->findText(QStringLiteral("Human")));
    }
    map[QStringLiteral("HumanCpu")] = m_listPlayerCombo[i]->currentText();

    if (m_DefaultPlayerColors.size() < m_nMaxPlayers) {
      qWarning() << "Fallback player color missing!";
      map[QStringLiteral("Color")] =
          this->readColor(m_pSettings, QStringLiteral("Color"),
                          m_DefaultPlayerColors[0])
              .name();
    } else {
      map[QStringLiteral("Color")] =
          this->readColor(m_pSettings, QStringLiteral("Color"),
                          m_DefaultPlayerColors[i])
              .name();
    }
    m_listColorEdit[i]->setText(map[QStringLiteral("Color")]);
    QPalette *palette = new QPalette();
    palette->setColor(QPalette::Base, map[QStringLiteral("Color")]);
    m_listColorEdit.at(i)->setPalette(*palette);

    m_Players << map;
    m_pSettings->endGroup();
  }

  m_nStartPlayer = m_pSettings->value(QStringLiteral("StartPlayer"), 1).toInt();
  this->updateStartCombo();
  m_nStartPlayer = m_pUi->cbStartPlayer->currentIndex();

  m_nTowersToWin =
      m_pSettings->value(QStringLiteral("NumWinTowers"), 1).toInt();
  m_pUi->spinTowersToWin->setValue(m_nTowersToWin);

  m_bShowPossibleMoveTowers =
      m_pSettings->value(QStringLiteral("ShowPossibleMoveTowers"), true)
          .toBool();
  m_pUi->checkShowPossibleMoves->setChecked(m_bShowPossibleMoveTowers);

  const QString sDefaultBoard(m_sSharePath + "/boards/Square_5x5" +
                              m_sBoardExtension);
  m_sBoard =
      m_pSettings->value(QStringLiteral("Board"), sDefaultBoard).toString();
  if (!QFile::exists(m_sBoard)) {
    qWarning() << "Board from conf file not found:" << m_sBoard;
    qWarning() << "Switching to default board!";
    m_sBoard = sDefaultBoard;
  }
  if (m_sListBoards.contains(m_sBoard)) {
    m_pUi->cbBoard->setCurrentIndex(m_sListBoards.indexOf(m_sBoard));
  } else {
    if (m_sListBoards.contains(sDefaultBoard)) {
      m_pUi->cbBoard->setCurrentIndex(m_sListBoards.indexOf(sDefaultBoard));
    } else {
      if (!m_sListBoards.isEmpty()) {
        m_sBoard = m_sListBoards.at(0);
        m_pUi->cbBoard->setCurrentIndex(0);
      } else {
        QMessageBox::warning(nullptr, tr("Error"),
                             tr("Boards folder seems empty!"));
        qWarning() << "Boards folder(s) empty (share folder and user folder)!";
        return;
      }
    }
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Settings::changedStyle(int nIndex) {
  QString sFileName(QLatin1String(""));

  if (0 == nIndex) {  // Create new style
    bool bOk;
    QFileInfo fi(m_pSettings->fileName());
    QFileInfo fiStyle(fi.absolutePath() + "/" + m_sBoardStyleFile + m_sExt);

    sFileName =
        QInputDialog::getText(nullptr, tr("New style"),
                              tr("Please insert name of "
                                 "new style file:"),
                              QLineEdit::Normal, QLatin1String(""), &bOk);
    if (!bOk || sFileName.isEmpty()) {
      // Reset selection
      m_pUi->cbBoardStyle->setCurrentIndex(
          m_pUi->cbBoardStyle->findText(m_sBoardStyleFile));
      return;
    }

    sFileName = sFileName + "-style";
    QFile fileStyle(fiStyle.absolutePath() + "/" + sFileName + m_sExt);

    if (fileStyle.exists()) {
      // Reset selection
      m_pUi->cbBoardStyle->setCurrentIndex(
          m_pUi->cbBoardStyle->findText(m_sBoardStyleFile));

      QMessageBox::warning(nullptr, tr("Error"), tr("File already exists."));
      qWarning() << "Style file already exists:" << fileStyle.fileName();
      return;
    }
    bOk = QFile::copy(fiStyle.absoluteFilePath(), fileStyle.fileName());
    if (!bOk) {
      // Reset selection
      m_pUi->cbBoardStyle->setCurrentIndex(
          m_pUi->cbBoardStyle->findText(m_sBoardStyleFile));

      QMessageBox::warning(nullptr, tr("Error"),
                           tr("Could not create new style."));
      qWarning() << "Could not create new style file:";
      qWarning() << "Org:" << fiStyle.absoluteFilePath();
      qWarning() << "Copy:" << fileStyle.fileName();
      return;
    }
    m_pUi->cbBoardStyle->addItem(sFileName);
    m_pUi->cbBoardStyle->setCurrentIndex(
        m_pUi->cbBoardStyle->findText(sFileName));
  } else {  // Load existing style file
    sFileName = m_pUi->cbBoardStyle->currentText();
  }

  this->loadBoardStyle(sFileName);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Settings::clickedStyleCell(int nRow, int nCol) {
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

bool Settings::eventFilter(QObject *pObj, QEvent *pEvent) {
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

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto Settings::readColor(QSettings *pSet, const QString &sKey,
                         const QString &sFallback) const -> QColor {
  QString sValue = pSet->value(sKey, sFallback).toString();
  QColor color(sFallback);

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
  color.setNamedColor(sValue);
#else
  color = QColor::fromString(sValue);
#endif
  if (!color.isValid()) {
    qWarning() << "Found invalid color for key" << sKey;
#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
    color.setNamedColor(sFallback);
#else
    color = QColor::fromString(sFallback);
#endif
  }
  return color;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void Settings::updateUiLang() {
  m_pUi->retranslateUi(this);

  // Widgets, which had not been created through UI have to be handled manually
  for (int i = 0; i < m_nMaxPlayers; i++) {
    m_listColorLbls[i]->setText(
        tr("Color player %1").arg(QString::number(i + 1)));
    m_listHumCpuLbls[i]->setText(
        tr("Player %1 Human/CPU").arg(QString::number(i + 1)));
  }

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

  this->updateStartCombo();
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

auto Settings::getLanguage() -> QString {
  if ("auto" == m_sGuiLanguage) {
#ifdef Q_OS_UNIX
    QByteArray lang = qgetenv("LANG");
    if (!lang.isEmpty()) {
      return QLocale(QString::fromLatin1(lang)).name();
    }
#endif
    return QLocale::system().name();
  }
  if (!QFile(":/" + qApp->applicationName().toLower() + "_" + m_sGuiLanguage +
             ".qm")
           .exists() &&
      !QFile(m_sSharePath + "/lang/" + qApp->applicationName().toLower() + "_" +
             m_sGuiLanguage + ".qm")
           .exists()) {
    m_sGuiLanguage = QStringLiteral("en");
    m_pSettings->setValue(QStringLiteral("GuiLanguage"), m_sGuiLanguage);
    return m_sGuiLanguage;
  }
  return m_sGuiLanguage;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void Settings::changeNumOfPlayers() {
  m_nNumOfPlayers = m_pUi->spinNumOfPlayers->value();
  for (int i = 0; i < m_nMaxPlayers; i++) {
    if (i < m_nNumOfPlayers) {
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
  this->updateStartCombo();
  this->adjustSize();
}

void Settings::updateStartCombo() {
  QStringList sListStartPlayer;
  sListStartPlayer.reserve(m_pUi->spinNumOfPlayers->value() + 1);
  sListStartPlayer << tr("Random");
  for (int i = 1; i <= m_pUi->spinNumOfPlayers->value(); i++) {
    sListStartPlayer << tr("Player") + " " + QString::number(i);
  }
  m_pUi->cbStartPlayer->clear();
  m_pUi->cbStartPlayer->addItems(sListStartPlayer);

  if (m_nStartPlayer < m_pUi->cbStartPlayer->count()) {
    m_pUi->cbStartPlayer->setCurrentIndex(m_nStartPlayer);
  } else {
    m_nStartPlayer = 1;
    m_pUi->cbStartPlayer->setCurrentIndex(m_nStartPlayer);
  }
}

void Settings::changedSettings() { m_bSettingChanged = true; }

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

auto Settings::getPlayerCpuScript(const quint8 nPlayer) const -> QString {
  if ((nPlayer - 1) < m_Players.size() &&
      (nPlayer - 1) < m_listPlayerCombo.size()) {
    if (-1 != m_listPlayerCombo[nPlayer - 1]->findText(
                  m_Players[nPlayer - 1][QStringLiteral("HumanCpu")])) {
      QString s(m_sListCPUs[m_listPlayerCombo[nPlayer - 1]->findText(
          m_Players[nPlayer - 1][QStringLiteral("HumanCpu")])]);
      if ("Human" == s) {
        s.clear();
      }
      return s;
    }
    return QLatin1String("");
  }
  qWarning() << "Array length exceeded! m_Player:" << m_Players.size()
             << "- m_listPlayerCombo:" << m_listPlayerCombo.size()
             << "- requested (nPlayer - 1):" << nPlayer - 1;
  return QLatin1String("");
}

auto Settings::getPlayerColor(const quint8 nPlayer) const -> QString {
  if (nPlayer < m_Players.size()) {
    return m_Players[nPlayer][QStringLiteral("Color")];
  }
  qWarning() << "Player array length exceeded! Size:" << m_Players.size()
             << "- requested nPlayer:" << nPlayer;
  return m_DefaultPlayerColors[0];
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

auto Settings::getNumOfPlayers() const -> quint8 { return m_nNumOfPlayers; }

auto Settings::getMaxNumOfPlayers() const -> quint8 { return m_nMaxPlayers; }

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

auto Settings::getBoardFile() const -> QString {
  if (!QFile::exists(m_sBoard)) {
    QMessageBox::warning(nullptr, this->windowTitle(),
                         tr("Selected board could not be found!"));
    qWarning() << "Board not found:" << m_sBoard;
    return QString();
  }
  return m_sBoard;
}

auto Settings::getStartPlayer() const -> quint8 {
  return static_cast<quint8>(m_nStartPlayer);
}
auto Settings::getTowersToWin() const -> quint8 {
  return static_cast<quint8>(m_nTowersToWin);
}
auto Settings::getShowPossibleMoveTowers() const -> bool {
  return m_bShowPossibleMoveTowers;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

auto Settings::getGridSize() const -> quint16 { return m_nGridSize; }
void Settings::setGridSize(const quint16 nNewGrid) {
  if (nNewGrid < m_nDefaultGrid) {
    m_nGridSize = m_nDefaultGrid;
  } else if (nNewGrid > m_nMaxGrid) {
    m_nGridSize = m_nMaxGrid;
  } else {
    m_nGridSize = nNewGrid;
  }
  m_pSettings->setValue(QStringLiteral("GridSize"), m_nGridSize);
}

auto Settings::getDefaultGrid() const -> qreal { return m_nDefaultGrid; }
auto Settings::getMaxGrid() const -> quint16 { return m_nMaxGrid; }

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

auto Settings::getBgColor() const -> QColor { return m_bgColor; }
auto Settings::getTextColor() const -> QColor { return m_txtColor; }
auto Settings::getTextHighlightColor() const -> QColor {
  return m_txtHighColor;
}
auto Settings::getHighlightColor() const -> QColor { return m_highlightColor; }
auto Settings::getHighlightBorderColor() const -> QColor {
  return m_highlightBorderColor;
}
auto Settings::getSelectedColor() const -> QColor { return m_selectedColor; }
auto Settings::getSelectedBorderColor() const -> QColor {
  return m_selectedBorderColor;
}
auto Settings::getAnimateColor() const -> QColor { return m_animateColor; }
auto Settings::getAnimateBorderColor() const -> QColor {
  return m_animateBorderColor;
}
auto Settings::getBgBoardColor() const -> QColor { return m_bgBoardColor; }
auto Settings::getGridBoardColor() const -> QColor { return m_gridBoardColor; }
auto Settings::getNeighboursColor() const -> QColor {
  return m_neighboursColor;
}
auto Settings::getNeighboursBorderColor() const -> QColor {
  return m_neighboursBorderColor;
}
