/**
 * \file settings.cpp
 *
 * \section LICENSE
 *
 * Copyright (C) 2015-2019 Thorsten Roth <elthoro@gmx.de>
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
 * along with StackAndConquer.  If not, see <http://www.gnu.org/licenses/>.
 *
 * \section DESCRIPTION
 * Settings dialog.
 */

#include "./settings.h"

#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QIcon>
#include <QMessageBox>

#include "ui_settings.h"

Settings::Settings(const QString &sSharePath, const QString &userDataDir,
                   QWidget *pParent)
  : QDialog(pParent),
    m_pUi(new Ui::SettingsDialog()),
    m_sSharePath(sSharePath) {
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
  this->searchCpuScripts(userDataDir);

  QStringList sListStartPlayer;
  sListStartPlayer << tr("Random")
                   << tr("Player 1")
                   << tr("Player 2");
  m_pUi->cbStartPlayer->addItems(sListStartPlayer);

  connect(m_pUi->buttonBox, &QDialogButtonBox::accepted,
          this, &Settings::accept);
  connect(m_pUi->buttonBox, &QDialogButtonBox::rejected,
          this, &Settings::reject);

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

QStringList Settings::searchTranslations() const {
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

  sList << QStringLiteral("en");
  sList.sort();
  sList.push_front(QStringLiteral("auto"));
  return sList;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void Settings::searchCpuScripts(const QString &userDataDir) {
  QStringList sListAvailableCpu;
  sListAvailableCpu << QStringLiteral("Human");
  m_sListCPUs.clear();
  m_sListCPUs << QStringLiteral("Human");
  QDir cpuDir = m_sSharePath;

  // Cpu scripts in share folder
  if (cpuDir.cd(QStringLiteral("cpu"))) {
    foreach (QFileInfo file, cpuDir.entryInfoList(QDir::Files)) {
      if ("js" == file.suffix().toLower()) {
        sListAvailableCpu << file.baseName();
        m_sListCPUs << file.absoluteFilePath();
      }
    }
  }
  m_pUi->cbP1HumanCpu->addItems(sListAvailableCpu);
  m_pUi->cbP2HumanCpu->addItems(sListAvailableCpu);

  // Cpu scripts in user folder
  cpuDir.setPath(userDataDir);
  if (cpuDir.cd(QStringLiteral("cpu"))) {
    foreach (QFileInfo file, cpuDir.entryInfoList(QDir::Files)) {
      if ("js" == file.suffix().toLower()) {
        sListAvailableCpu << file.baseName();
        m_pUi->cbP1HumanCpu->addItem(QIcon(
                                       QStringLiteral(":/images/user.png")),
                                     sListAvailableCpu.last());
        m_pUi->cbP2HumanCpu->addItem(QIcon(
                                       QStringLiteral(":/images/user.png")),
                                     sListAvailableCpu.last());
        m_sListCPUs << file.absoluteFilePath();
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

  m_sNameP1 = m_pUi->leNameP1->text();
  if (m_sNameP1.trimmed().isEmpty()) {
    m_sNameP1 = tr("Player 1");
    m_pUi->leNameP1->setText(m_sNameP1);
  }
  m_pSettings->setValue(QStringLiteral("NameP1"), m_sNameP1);
  m_sNameP2 = m_pUi->leNameP2->text();
  if (m_sNameP2.trimmed().isEmpty()) {
    m_sNameP2 = tr("Player 2");
    m_pUi->leNameP2->setText(m_sNameP2);
  }
  m_pSettings->setValue(QStringLiteral("NameP2"), m_sNameP2);

  m_nStartPlayer = m_pUi->cbStartPlayer->currentIndex();
  m_pSettings->setValue(QStringLiteral("StartPlayer"), m_nStartPlayer);

  m_bShowPossibleMoveTowers = m_pUi->checkShowPossibleMoves->isChecked();
  m_pSettings->setValue(QStringLiteral("ShowPossibleMoveTowers"),
                        m_bShowPossibleMoveTowers);

  m_pSettings->beginGroup(QStringLiteral("Colors"));
  m_pSettings->setValue(QStringLiteral("BgColor"), m_bgColor.name());
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
  m_pSettings->setValue(QStringLiteral("OutlineBoardColor"),
                        m_outlineBoardColor.name());
  m_pSettings->setValue(QStringLiteral("GridBoardColor"),
                        m_gridBoardColor.name());
  m_pSettings->setValue(QStringLiteral("NeighboursColor"),
                        m_neighboursColor.name());
  m_pSettings->setValue(QStringLiteral("NeighboursBorderColor"),
                        m_neighboursBorderColor.name());
  m_pSettings->endGroup();

  QString sNewP1HumanCpu(m_pUi->cbP1HumanCpu->currentText());
  QString sNewP2HumanCpu(m_pUi->cbP2HumanCpu->currentText());
  int nNewWinTowers(m_pUi->spinNumToWin->value());

  if (sNewP1HumanCpu != m_sP1HumanCpu ||
      sNewP2HumanCpu != m_sP2HumanCpu ||
      nNewWinTowers != m_nWinTowers) {
    int nRet = QMessageBox::question(
                 nullptr, this->windowTitle(),
                 tr("Main game settings had been changed.<br>"
                    "Do you want to start a new game?"));
    if (nRet == QMessageBox::Yes) {
      m_sP1HumanCpu = sNewP1HumanCpu;
      m_sP2HumanCpu = sNewP2HumanCpu;
      m_nWinTowers = nNewWinTowers;
      m_pSettings->setValue(QStringLiteral("P1HumanCpu"), m_sP1HumanCpu);
      m_pSettings->setValue(QStringLiteral("P2HumanCpu"), m_sP2HumanCpu);
      m_pSettings->setValue(QStringLiteral("NumWinTowers"), m_nWinTowers);
      emit this->newGame(QStringList());
    } else {
      this->readSettings();
      return;
    }
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

  m_sNameP1 = m_pSettings->value(QStringLiteral("NameP1"),
                                 tr("Player 1")).toString();
  m_pUi->leNameP1->setText(m_sNameP1);
  m_sNameP2 = m_pSettings->value(QStringLiteral("NameP2"),
                                 tr("Player 2")).toString();
  m_pUi->leNameP2->setText(m_sNameP2);

  m_sP1HumanCpu = m_pSettings->value(QStringLiteral("P1HumanCpu"),
                                     QStringLiteral("Human")).toString();
  if (-1 != m_pUi->cbP1HumanCpu->findText(m_sP1HumanCpu)) {
    m_pUi->cbP1HumanCpu->setCurrentIndex(
          m_pUi->cbP1HumanCpu->findText(m_sP1HumanCpu));
  } else {
    m_pUi->cbP1HumanCpu->setCurrentIndex(
          m_pUi->cbP1HumanCpu->findText(QStringLiteral("Human")));
  }
  m_sP1HumanCpu = m_pUi->cbP1HumanCpu->currentText();

  m_sP2HumanCpu = m_pSettings->value(QStringLiteral("P2HumanCpu"),
                                     QStringLiteral("Human")).toString();
  if (-1 != m_pUi->cbP2HumanCpu->findText(m_sP2HumanCpu)) {
    m_pUi->cbP2HumanCpu->setCurrentIndex(
          m_pUi->cbP2HumanCpu->findText(m_sP2HumanCpu));
  } else {
    m_pUi->cbP2HumanCpu->setCurrentIndex(
          m_pUi->cbP2HumanCpu->findText(QStringLiteral("Human")));
  }
  m_sP2HumanCpu = m_pUi->cbP2HumanCpu->currentText();

  m_nStartPlayer = m_pSettings->value(QStringLiteral("StartPlayer"),
                                      1).toInt();
  if (m_nStartPlayer < m_pUi->cbStartPlayer->count()) {
    m_pUi->cbStartPlayer->setCurrentIndex(m_nStartPlayer);
  } else {
    m_pUi->cbStartPlayer->setCurrentIndex(1);
  }
  m_nStartPlayer = m_pUi->cbStartPlayer->currentIndex();

  m_nWinTowers = m_pSettings->value(QStringLiteral("NumWinTowers"), 1).toInt();
  m_pUi->spinNumToWin->setValue(m_nWinTowers);

  m_bShowPossibleMoveTowers = m_pSettings->value(
                                QStringLiteral("ShowPossibleMoveTowers"),
                                true).toBool();
  m_pUi->checkShowPossibleMoves->setChecked(m_bShowPossibleMoveTowers);

  m_bgColor = this->readColor(QStringLiteral("BgColor"),
                              QStringLiteral("#EEEEEC"));
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
  m_outlineBoardColor = this->readColor(QStringLiteral("OutlineBoardColor"),
                                        QStringLiteral("#2E3436"));
  m_gridBoardColor = this->readColor(QStringLiteral("GridBoardColor"),
                                     QStringLiteral("#888A85"));
  m_neighboursColor = this->readColor(QStringLiteral("NeighboursColor"),
                                      QStringLiteral("#ad7fa8"));
  m_neighboursBorderColor = this->readColor(
                              QStringLiteral("NeighboursBorderColor"),
                              QStringLiteral("#000000"));
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

QColor Settings::readColor(const QString &sKey,
                           const QString &sFallback) const {
  QString sValue = m_pSettings->value("Colors/" + sKey, sFallback).toString();
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

  QStringList sListStartPlayer;
  sListStartPlayer << tr("Random")
                   << tr("Player 1")
                   << tr("Player 2");
  m_pUi->cbStartPlayer->clear();
  m_pUi->cbStartPlayer->addItems(sListStartPlayer);
  m_pUi->cbStartPlayer->setCurrentIndex(m_nStartPlayer);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

QString Settings::getLanguage() {
  if ("auto" == m_sGuiLanguage) {
#ifdef Q_OS_UNIX
    QByteArray lang = qgetenv("LANG");
    if (!lang.isEmpty()) {
      return QLocale(lang).name();
    }
#endif
    return QLocale::system().name();
  } else if (!QFile(":/" + qApp->applicationName().toLower() +
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

QString Settings::getNameP1() const {
  return m_sNameP1;
}
QString Settings::getNameP2() const {
  return m_sNameP2;
}
quint8 Settings::getStartPlayer() const {
  return static_cast<quint8>(m_nStartPlayer);
}
quint8 Settings::getWinTowers() const {
  return static_cast<quint8>(m_nWinTowers);
}
bool Settings::getShowPossibleMoveTowers() const {
  return m_bShowPossibleMoveTowers;
}

QString Settings::getP1HumanCpu() const {
  if (-1 != m_pUi->cbP1HumanCpu->findText(m_sP1HumanCpu)) {
    return m_sListCPUs[m_pUi->cbP1HumanCpu->findText(m_sP1HumanCpu)];
  } else {
    return QStringLiteral("Human");
  }
}

QString Settings::getP2HumanCpu() const {
  if (-1 != m_pUi->cbP2HumanCpu->findText(m_sP2HumanCpu)) {
    return m_sListCPUs[m_pUi->cbP2HumanCpu->findText(m_sP2HumanCpu)];
  } else {
    return QStringLiteral("Human");
  }
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

QColor Settings::getBgColor() const {
  return m_bgColor;
}
QColor Settings::getHighlightColor() const {
  return m_highlightColor;
}
QColor Settings::getHighlightBorderColor() const {
  return m_highlightBorderColor;
}
QColor Settings::getSelectedColor() const {
  return m_selectedColor;
}
QColor Settings::getSelectedBorderColor() const {
  return m_selectedBorderColor;
}
QColor Settings::getAnimateColor() const {
  return m_animateColor;
}
QColor Settings::getAnimateBorderColor() const {
  return m_animateBorderColor;
}
QColor Settings::getBgBoardColor() const {
  return m_bgBoardColor;
}
QColor Settings::getOutlineBoardColor() const {
  return m_outlineBoardColor;
}
QColor Settings::getGridBoardColor() const {
  return m_gridBoardColor;
}
QColor Settings::GetNeighboursColor() const {
  return m_neighboursColor;
}
QColor Settings::GetNeighboursBorderColor() const {
  return m_neighboursBorderColor;
}
