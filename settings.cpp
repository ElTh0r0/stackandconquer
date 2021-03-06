/**
 * \file settings.cpp
 *
 * \section LICENSE
 *
 * Copyright (C) 2015-2021 Thorsten Roth
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
#include <QLineEdit>
#include <QIcon>
#include <QMessageBox>
#include <QSettings>
#include <QSpinBox>

#include "ui_settings.h"

Settings::Settings(const QString &sSharePath, const QString &userDataDir,
                   const quint8 nMaxPlayers, QWidget *pParent)
  : QDialog(pParent),
    m_pUi(new Ui::SettingsDialog()),
    m_sSharePath(sSharePath),
    m_nDefaultGrid(70),  // Default stone SVG size fits to grid size of 70!
    m_nMaxGrid(200),
    m_nMaxPlayers(nMaxPlayers),
    m_DefaultPlayerColors{"#EF2929", "#FCAF3E", "#729FCF", "#8F5902"} {
  m_pUi->setupUi(this);
  this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
  this->setModal(true);

#if defined _WIN32
  m_pSettings = new QSettings(QSettings::IniFormat, QSettings::UserScope,
                              qApp->applicationName().toLower(),
                              qApp->applicationName().toLower());
#else
  m_pSettings = new QSettings(QSettings::NativeFormat, QSettings::UserScope,
                              qApp->applicationName().toLower(),
                              qApp->applicationName().toLower());
#endif

  m_pUi->cbGuiLanguage->addItems(this->searchTranslations());

  m_pUi->spinNumOfPlayers->setMaximum(m_nMaxPlayers);
  connect(m_pUi->spinNumOfPlayers,
          static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
          this, &Settings::changeNumOfPlayers);
  connect(m_pUi->spinNumOfPlayers,
          static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
          this, &Settings::changedSettings);
  // TODO(x): Remove after implementation of > 2 players
  if (m_nMaxPlayers < 3) {
    m_pUi->spinNumOfPlayers->setVisible(false);
    m_pUi->lblNumOfPlayers->setVisible(false);
  }

  connect(m_pUi->spinNumToWin,
          static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
          this, &Settings::changedSettings);
  connect(m_pUi->cbStartPlayer, &QComboBox::currentTextChanged,
          this, &Settings::changedSettings);

  connect(m_pUi->buttonBox, &QDialogButtonBox::accepted,
          this, &Settings::accept);
  connect(m_pUi->buttonBox, &QDialogButtonBox::rejected,
          this, &Settings::reject);

  for (int i = m_nMaxPlayers-1; i >= 0; i--) {
    m_listColorLbls.push_front(
          new QLabel(tr("Color player %1").arg(QString::number(i+1))));
    m_listColorEdit.push_front(new QLineEdit(this));
    m_pUi->formLayout->insertRow(2, m_listColorLbls.first(),
                                 m_listColorEdit.first());
    m_listColorEdit.first()->installEventFilter(this);
    connect(m_listColorEdit.first(), &QLineEdit::textChanged,
            this, &Settings::changedSettings);

    m_listHumCpuLbls.push_front(
          new QLabel(tr("Player %1 Human/CPU").arg(QString::number(i+1))));
    m_listPlayerCombo.push_front(new QComboBox(this));
    m_pUi->formLayout->insertRow(2, m_listHumCpuLbls.first(),
                                 m_listPlayerCombo.first());
    connect(m_listPlayerCombo.first(), &QComboBox::currentTextChanged,
            this, &Settings::changedSettings);
  }

  this->searchCpuScripts(userDataDir);
  this->readSettings();
}

Settings::~Settings() {
  if (m_pUi) {
    delete m_pUi;
    m_pUi = nullptr;
  }
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void Settings::showEvent(QShowEvent *pEvent) {
  this->readSettings();
  m_bSettingChanged = false;
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
      sList << sTmp.remove(
                 qApp->applicationName().toLower() + "_").remove(
                 QStringLiteral(".qm"));
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
      sTmp = sTmp.remove(
               qApp->applicationName().toLower() + "_") .remove(
               QStringLiteral(".qm"));
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

void Settings::searchCpuScripts(const QString &userDataDir) {
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

  // Cpu scripts in share folder
  if (cpuDir.cd(QStringLiteral("cpu"))) {
    const QFileInfoList listFiles(cpuDir.entryInfoList(QDir::Files));
    sIcon = QStringLiteral(":/img/computer.png");
    if (this->window()->palette().window().color().lightnessF() < 0.5) {
      sIcon = QStringLiteral(":/img/computer2.png");
    }
    for (const auto &file : listFiles) {
      if ("js" == file.suffix().toLower()) {
        m_sListCPUs << file.absoluteFilePath();
        for (int i = 0; i < m_listPlayerCombo.size(); i++) {
          m_listPlayerCombo[i]->addItem(QIcon(sIcon), file.baseName());
        }
      }
    }
  }

  // Cpu scripts in user folder
  cpuDir.setPath(userDataDir);
  if (cpuDir.cd(QStringLiteral("cpu"))) {
    const QFileInfoList listFiles(cpuDir.entryInfoList(QDir::Files));
    sIcon = QStringLiteral(":/img/code.png");
    if (this->window()->palette().window().color().lightnessF() < 0.5) {
      sIcon = QStringLiteral(":/img/code2.png");
    }
    int nIndex = -1;
    for (const auto &file : listFiles) {
      if ("js" == file.suffix().toLower()) {
        if (-1 != m_listPlayerCombo[0]->findText(file.baseName())) {
          qWarning() << "Duplicate CPU script name found, skipping script" <<
                        file.absoluteFilePath();
          continue;
        }

        nIndex = (nIndex < 0) ? m_sListCPUs.length() : nIndex;
        m_sListCPUs << file.absoluteFilePath();
        for (int i = 0; i < m_listPlayerCombo.size(); i++) {
          m_listPlayerCombo[i]->addItem(QIcon(sIcon), file.baseName());
        }
      }
    }

    // Separator between game CPU scripts and user scripts
    if (nIndex > 0) {
      for (int i = 0; i < m_listPlayerCombo.size(); i++) {
        m_listPlayerCombo[i]->insertSeparator(nIndex);
      }
    }
  }
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

  int nRet = QMessageBox::No;
  if (m_bSettingChanged) {
    nRet = QMessageBox::question(
             nullptr, this->windowTitle(),
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
  // TODO(x): Enable after implenmentation of > 2 players
  // m_pSettings->setValue(QStringLiteral("NumOfPlayers"), m_nNumOfPlayers);

  m_nStartPlayer = m_pUi->cbStartPlayer->currentIndex();
  m_pSettings->setValue(QStringLiteral("StartPlayer"), m_nStartPlayer);

  m_nWinTowers = m_pUi->spinNumToWin->value();
  m_pSettings->setValue(QStringLiteral("NumWinTowers"), m_nWinTowers);

  // Colors
  m_pSettings->beginGroup(QStringLiteral("Colors"));
  m_pSettings->setValue(QStringLiteral("BgColor"), m_bgColor.name());
  m_pSettings->setValue(QStringLiteral("TextColor"), m_txtColor.name());
  m_pSettings->setValue(QStringLiteral("TextHighlightColor"),
                        m_txtHighColor.name());
  m_pSettings->setValue(QStringLiteral("HighlightColor"),
                        m_highlightColor.name());
  m_pSettings->setValue(QStringLiteral("HighlightBorderColor"),
                        m_highlightBorderColor.name());
  m_pSettings->setValue(QStringLiteral("SelectedColor"),
                        m_selectedColor.name());
  m_pSettings->setValue(QStringLiteral("SelectedBorderColor"),
                        m_selectedBorderColor.name());
  m_pSettings->setValue(QStringLiteral("AnimateColor"),
                        m_animateColor.name());
  m_pSettings->setValue(QStringLiteral("AnimateBorderColor"),
                        m_animateBorderColor.name());
  m_pSettings->setValue(QStringLiteral("BgBoardColor"),
                        m_bgBoardColor.name());
  m_pSettings->setValue(QStringLiteral("GridBoardColor"),
                        m_gridBoardColor.name());
  m_pSettings->setValue(QStringLiteral("NeighboursColor"),
                        m_neighboursColor.name());
  m_pSettings->setValue(QStringLiteral("NeighboursBorderColor"),
                        m_neighboursBorderColor.name());
  m_pSettings->endGroup();

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
      qWarning() << "User chose invalid stone color:" <<
                    m_listColorEdit[i]->text();
      if (i < m_DefaultPlayerColors.size()) {
        color.setNamedColor(m_DefaultPlayerColors[i]);
      } else {
        qWarning() << "Fallback player color missing!";
        color.setNamedColor(m_DefaultPlayerColors[0]);
      }
    }
    tmpMap[QStringLiteral("Color")] = color.name();
    m_Players << tmpMap;

    m_pSettings->beginGroup("Player" + QString::number(i+1));
    m_pSettings->setValue(QStringLiteral("HumanCpu"),
                          tmpMap[QStringLiteral("HumanCpu")]);
    m_pSettings->setValue(QStringLiteral("Color"),
                          tmpMap[QStringLiteral("Color")]);
    m_pSettings->remove(QStringLiteral("Name"));
    m_pSettings->endGroup();
  }

  // Remove deprecated settings
  m_pSettings->remove(QStringLiteral("Colors/OutlineBoardColor"));
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
  m_sGuiLanguage = m_pSettings->value(QStringLiteral("GuiLanguage"),
                                      QStringLiteral("auto")).toString();
  if (-1 != m_pUi->cbGuiLanguage->findText(m_sGuiLanguage)) {
    m_pUi->cbGuiLanguage->setCurrentIndex(
          m_pUi->cbGuiLanguage->findText(m_sGuiLanguage));
  } else {
    m_pUi->cbGuiLanguage->setCurrentIndex(
          m_pUi->cbGuiLanguage->findText(QStringLiteral("auto")));
  }
  m_sGuiLanguage = m_pUi->cbGuiLanguage->currentText();

  m_nGridSize = m_pSettings->value(QStringLiteral("GridSize"),
                                   m_nDefaultGrid).toInt();
  if (m_nGridSize < m_nDefaultGrid) {
    m_nGridSize = m_nDefaultGrid;
  } else if (m_nGridSize > m_nMaxGrid) {
    m_nGridSize = m_nMaxGrid;
  }

  m_nNumOfPlayers = m_pSettings->value(
                      QStringLiteral("NumOfPlayers"), 2).toInt();
  if (m_nNumOfPlayers > m_nMaxPlayers) {
    m_nNumOfPlayers = m_nMaxPlayers;
  }
  m_pUi->spinNumToWin->setValue(m_nNumOfPlayers);
  this->changeNumOfPlayers();

  for (int i = 0; i < m_Players.size(); i++) {
    m_Players[i].clear();
  }
  m_Players.clear();

  for (quint8 i = 0; i < m_nMaxPlayers; i++) {
    m_pSettings->beginGroup("Player" + QString::number(i+1));
    QMap<QString, QString> map;
    map[QStringLiteral("HumanCpu")] = m_pSettings->value(
                                        QStringLiteral("HumanCpu"),
                                        QStringLiteral("Human")).toString();

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
      map[QStringLiteral("Color")] = this->readColor(QStringLiteral("Color"),
                                     m_DefaultPlayerColors[0]).name();
    } else {
      map[QStringLiteral("Color")] = this->readColor(QStringLiteral("Color"),
                                     m_DefaultPlayerColors[i]).name();
    }
    m_listColorEdit[i]->setText(map[QStringLiteral("Color")]);
    QPalette *palette = new QPalette();
    palette->setColor(QPalette::Base, map[QStringLiteral("Color")]);
    m_listColorEdit.at(i)->setPalette(*palette);

    m_Players << map;
    m_pSettings->endGroup();
  }

  m_nStartPlayer = m_pSettings->value(QStringLiteral("StartPlayer"),
                                      1).toInt();
  this->updateStartCombo();
  m_nStartPlayer = m_pUi->cbStartPlayer->currentIndex();

  m_nWinTowers = m_pSettings->value(QStringLiteral("NumWinTowers"), 1).toInt();
  m_pUi->spinNumToWin->setValue(m_nWinTowers);

  m_bShowPossibleMoveTowers = m_pSettings->value(
                                QStringLiteral("ShowPossibleMoveTowers"),
                                true).toBool();
  m_pUi->checkShowPossibleMoves->setChecked(m_bShowPossibleMoveTowers);

  m_pSettings->beginGroup(QStringLiteral("Colors"));
  m_bgColor = this->readColor(QStringLiteral("BgColor"),
                              QStringLiteral("#EEEEEC"));
  m_txtColor = this->readColor(QStringLiteral("TextColor"),
                               QStringLiteral("#000000"));
  m_txtHighColor = this->readColor(QStringLiteral("TextHighlightColor"),
                                   QStringLiteral("#FF0000"));
  m_highlightColor = this->readColor(QStringLiteral("HighlightColor"),
                                     QStringLiteral("#8ae234"));
  m_highlightBorderColor = this->readColor(
                             QStringLiteral("HighlightBorderColor"),
                             QStringLiteral("#888A85"));
  m_selectedColor = this->readColor(QStringLiteral("SelectedColor"),
                                    QStringLiteral("#fce94f"));
  m_selectedBorderColor = this->readColor(QStringLiteral("SelectedBorderColor"),
                                          QStringLiteral("#000000"));
  m_animateColor = this->readColor(QStringLiteral("AnimateColor"),
                                   QStringLiteral("#fce94f"));
  m_animateBorderColor = this->readColor(QStringLiteral("AnimateBorderColor"),
                                         QStringLiteral("#000000"));
  m_bgBoardColor = this->readColor(QStringLiteral("BgBoardColor"),
                                   QStringLiteral("#FFFFFF"));
  m_gridBoardColor = this->readColor(QStringLiteral("GridBoardColor"),
                                     QStringLiteral("#888A85"));
  m_neighboursColor = this->readColor(QStringLiteral("NeighboursColor"),
                                      QStringLiteral("#ad7fa8"));
  m_neighboursBorderColor = this->readColor(
                              QStringLiteral("NeighboursBorderColor"),
                              QStringLiteral("#000000"));
  m_pSettings->endGroup();
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

auto Settings::readColor(const QString &sKey,
                         const QString &sFallback) const -> QColor {
  QString sValue = m_pSettings->value(sKey, sFallback).toString();
  QColor color(sFallback);

  color.setNamedColor(sValue);
  if (!color.isValid()) {
    color.setNamedColor(sFallback);
    qWarning() << "Found invalid color for key" << sKey;
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
          tr("Color player %1").arg(QString::number(i+1)));
    m_listHumCpuLbls[i]->setText(
          tr("Player %1 Human/CPU").arg(QString::number(i+1)));
  }

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
  if (!QFile(":/" + qApp->applicationName().toLower() +
             "_" + m_sGuiLanguage + ".qm").exists() &&
      !QFile(m_sSharePath + "/lang/" +
             qApp->applicationName().toLower() +
             "_" + m_sGuiLanguage + ".qm").exists()) {
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

void Settings::changedSettings() {
  m_bSettingChanged = true;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

auto Settings::getPlayerCpuScript(const quint8 nPlayer) const -> QString {
  if ((nPlayer - 1) < m_Players.size() &&
      (nPlayer - 1) < m_listPlayerCombo.size()) {
    if (-1 != m_listPlayerCombo[nPlayer-1]->findText(
          m_Players[nPlayer-1][QStringLiteral("HumanCpu")])) {
      QString s(m_sListCPUs[m_listPlayerCombo[nPlayer-1]->findText(
            m_Players[nPlayer-1][QStringLiteral("HumanCpu")])]);
      if ("Human" == s) {
        s.clear();
      }
      return s;
    }
    return QLatin1String("");
  }
  qWarning() << "Array length exceeded! m_Player:" << m_Players.size() <<
                "- m_listPlayerCombo:" << m_listPlayerCombo.size() <<
                "- requested (nPlayer - 1):" << nPlayer-1;
  return QLatin1String("");
}

auto Settings::getPlayerColor(const quint8 nPlayer) const -> QString {
  if (nPlayer < m_Players.size()) {
    return m_Players[nPlayer][QStringLiteral("Color")];
  }
  qWarning() << "Player array length exceeded! Size:" <<
                m_Players.size() << "- requested nPlayer:" << nPlayer;
  return m_DefaultPlayerColors[0];
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

auto Settings::getNumOfPlayers() const -> quint8 {
  return m_nNumOfPlayers;
}

auto Settings::getMaxNumOfPlayers() const -> quint8 {
  return m_nMaxPlayers;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

auto Settings::getBoardFile() const -> QString {
  // TODO(x): Rewrite if boards can be chosen
  QString sBoard(QLatin1String(""));
  if (QFile::exists(m_sSharePath + "/boards")) {
    sBoard = m_sSharePath + "/boards/square_5x5.stackboard";
    // sBoard = m_sSharePath + "/boards/triangle.stackboard";
    // sBoard = m_sSharePath + "/boards/square_4x2.stackboard";
  } else {
    qWarning() << "Games share path does not exist:" << m_sSharePath;
  }
  return sBoard;
}

auto Settings::getStartPlayer() const -> quint8 {
  return static_cast<quint8>(m_nStartPlayer);
}
auto Settings::getWinTowers() const -> quint8 {
  return static_cast<quint8>(m_nWinTowers);
}
auto Settings::getShowPossibleMoveTowers() const -> bool {
  return m_bShowPossibleMoveTowers;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

auto Settings::getGridSize() const -> quint16 {
  return m_nGridSize;
}
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

auto Settings::getDefaultGrid() const -> qreal {
  return m_nDefaultGrid;
}
auto Settings::getMaxGrid() const -> quint16 {
  return m_nMaxGrid;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

auto Settings::getBgColor() const -> QColor {
  return m_bgColor;
}
auto Settings::getTextColor() const -> QColor {
  return m_txtColor;
}
auto Settings::getTextHighlightColor() const -> QColor {
  return m_txtHighColor;
}
auto Settings::getHighlightColor() const -> QColor {
  return m_highlightColor;
}
auto Settings::getHighlightBorderColor() const -> QColor {
  return m_highlightBorderColor;
}
auto Settings::getSelectedColor() const -> QColor {
  return m_selectedColor;
}
auto Settings::getSelectedBorderColor() const -> QColor {
  return m_selectedBorderColor;
}
auto Settings::getAnimateColor() const -> QColor {
  return m_animateColor;
}
auto Settings::getAnimateBorderColor() const -> QColor {
  return m_animateBorderColor;
}
auto Settings::getBgBoardColor() const -> QColor {
  return m_bgBoardColor;
}
auto Settings::getGridBoardColor() const -> QColor {
  return m_gridBoardColor;
}
auto Settings::getNeighboursColor() const -> QColor {
  return m_neighboursColor;
}
auto Settings::getNeighboursBorderColor() const -> QColor {
  return m_neighboursBorderColor;
}
