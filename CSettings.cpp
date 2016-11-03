/**
 * \file CSettings.cpp
 *
 * \section LICENSE
 *
 * Copyright (C) 2015-2016 Thorsten Roth <elthoro@gmx.de>
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

#include <QDebug>
#include <QDir>
#include <QMessageBox>

#include "./CSettings.h"
#include "ui_CSettings.h"

CSettings::CSettings(const QString &sSharePath, QWidget *pParent)
  : QDialog(pParent),
    m_sSharePath(sSharePath) {
  qDebug() << "Calling" << Q_FUNC_INFO;

  m_pUi = new Ui::CSettingsDialog();
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

  m_sListMouseButtons << trUtf8("Left") << trUtf8("Middle") << trUtf8("Right")
                      << trUtf8("First X") << trUtf8("Second X");
  m_listMouseButtons << Qt::LeftButton << Qt::MidButton << Qt::RightButton
                     << Qt::XButton1 << Qt::XButton2;
  m_pUi->cbPlaceTower->addItems(m_sListMouseButtons);
  m_pUi->cbSelectTower->addItems(m_sListMouseButtons);

  connect(m_pUi->buttonBox, SIGNAL(accepted()),
          this, SLOT(accept()));

  this->readSettings();
}

CSettings::~CSettings() {
  if (m_pUi) {
    delete m_pUi;
    m_pUi = NULL;
  }
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void CSettings::accept() {
  qDebug() << "Calling" << Q_FUNC_INFO;

  QString sOldGuiLang = m_sGuiLanguage;
  m_sGuiLanguage = m_pUi->cbGuiLanguage->currentText();
  m_pSettings->setValue("GuiLanguage", m_sGuiLanguage);

  if (sOldGuiLang != m_sGuiLanguage) {
    QMessageBox::information(0, this->windowTitle(),
                             trUtf8("The game has to be restarted for "
                                    "applying the changes."));
  }

  m_sNameP1 = m_pUi->leNameP1->text();
  m_pSettings->setValue("NameP1", m_sNameP1);
  m_sNameP2 = m_pUi->leNameP2->text();
  m_pSettings->setValue("NameP2", m_sNameP2);

  m_sP2HumanCpu = m_pUi->cbP2HumanCpu->currentText();
  m_pSettings->setValue("P2HumanCpu", m_sP2HumanCpu);

  m_nStartPlayer = m_pUi->cbStartPlayer->currentIndex();
  m_pSettings->setValue("StartPlayer", m_nStartPlayer);

  m_nWinTowers = m_pUi->spinNumToWin->value();
  m_pSettings->setValue("NumWinTowers", m_nWinTowers);

  m_bShowPossibleMoveTowers = m_pUi->checkShowPossibleMoves->isChecked();
  m_pSettings->setValue("ShowPossibleMoveTowers", m_bShowPossibleMoveTowers);

  m_listMouseControls[0] =
      m_listMouseButtons[m_pUi->cbPlaceTower->currentIndex()];
  m_listMouseControls[1] =
      m_listMouseButtons[m_pUi->cbSelectTower->currentIndex()];
  m_pSettings->beginGroup("MouseControls");
  m_pSettings->setValue("PlaceTower", m_listMouseControls[0]);
  m_pSettings->setValue("SelectTower", m_listMouseControls[1]);
  m_pSettings->endGroup();

  if (m_listMouseControls[0] == m_listMouseControls[1]) {
    QMessageBox::warning(0, this->windowTitle(),
                             trUtf8("Please change your settings. Same mouse "
                                    "button is used for several actions."));
    return;
  }

  m_pSettings->beginGroup("Colors");
  m_pSettings->setValue("BgColor", m_bgColor.name());
  m_pSettings->setValue("HighlightColor", m_highlightColor.name());
  m_pSettings->setValue("HighlightBorderColor", m_highlightBorderColor.name());
  m_pSettings->setValue("SelectedColor", m_selectedColor.name());
  m_pSettings->setValue("SelectedBorderColor", m_selectedBorderColor.name());
  m_pSettings->setValue("AnimateColor", m_animateColor.name());
  m_pSettings->setValue("AnimateBorderColor", m_animateBorderColor.name());
  m_pSettings->setValue("BgBoardColor", m_bgBoardColor.name());
  m_pSettings->setValue("OutlineBoardColor", m_outlineBoardColor.name());
  m_pSettings->setValue("GridBoardColor", m_gridBoardColor.name());
  m_pSettings->setValue("NeighboursColor", m_neighboursColor.name());
  m_pSettings->setValue("NeighboursBorderColor", m_neighboursBorderColor.name());
  m_pSettings->endGroup();

  QDialog::accept();
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void CSettings::readSettings() {
  m_sGuiLanguage = m_pSettings->value("GuiLanguage", "auto").toString();
  QStringList sListGuiLanguages;
  sListGuiLanguages << "auto" << "en";
  QDir appDir(m_sSharePath + "/lang");
  QFileInfoList fiListFiles = appDir.entryInfoList(
                                QDir::NoDotAndDotDot | QDir::Files);
  foreach (QFileInfo fi, fiListFiles) {
    if ("qm" == fi.suffix() && fi.baseName().startsWith(qAppName().toLower() + "_")) {
      sListGuiLanguages << fi.baseName().remove(qAppName().toLower() + "_");
    }
  }
  m_pUi->cbGuiLanguage->addItems(sListGuiLanguages);
  if (-1 != m_pUi->cbGuiLanguage->findText(m_sGuiLanguage)) {
    m_pUi->cbGuiLanguage->setCurrentIndex(
          m_pUi->cbGuiLanguage->findText(m_sGuiLanguage));
  } else {
    m_pUi->cbGuiLanguage->setCurrentIndex(
          m_pUi->cbGuiLanguage->findText("auto"));
  }
  m_sGuiLanguage = m_pUi->cbGuiLanguage->currentText();

  m_sNameP1 = m_pSettings->value("NameP1", trUtf8("Player 1")).toString();
  m_pUi->leNameP1->setText(m_sNameP1);
  m_sNameP2 = m_pSettings->value("NameP2", trUtf8("Player 2")).toString();
  m_pUi->leNameP2->setText(m_sNameP2);

  m_sP2HumanCpu = m_pSettings->value("P2HumanCpu", "Human").toString();
  QStringList sListP2HumanCpu;
  sListP2HumanCpu << "Human";
  m_pUi->cbP2HumanCpu->addItems(sListP2HumanCpu);
  if (-1 != m_pUi->cbP2HumanCpu->findText(m_sP2HumanCpu)) {
    m_pUi->cbP2HumanCpu->setCurrentIndex(
          m_pUi->cbP2HumanCpu->findText(m_sP2HumanCpu));
  } else {
    m_pUi->cbP2HumanCpu->setCurrentIndex(
          m_pUi->cbP2HumanCpu->findText("Human"));
  }
  m_sP2HumanCpu = m_pUi->cbP2HumanCpu->currentText();

  QStringList sListStartPlayer;
  sListStartPlayer << trUtf8("Random")
                   << trUtf8("Player 1")
                   << trUtf8("Player 2");
  m_nStartPlayer = m_pSettings->value("StartPlayer", 1).toUInt();
  m_pUi->cbStartPlayer->addItems(sListStartPlayer);
  if (m_nStartPlayer < m_pUi->cbStartPlayer->count()) {
    m_pUi->cbStartPlayer->setCurrentIndex(m_nStartPlayer);
  } else {
    m_pUi->cbStartPlayer->setCurrentIndex(1);
  }
  m_nStartPlayer = m_pUi->cbStartPlayer->currentText().toUInt();

  m_nWinTowers = m_pSettings->value("NumWinTowers", 1).toUInt();
  m_pUi->spinNumToWin->setValue(m_nWinTowers);

  m_bShowPossibleMoveTowers = m_pSettings->value("ShowPossibleMoveTowers",
                                                 true).toBool();
  m_pUi->checkShowPossibleMoves->setChecked(m_bShowPossibleMoveTowers);

  m_listMouseControls.clear();
  m_listMouseControls << 0 << 0;
  m_pSettings->beginGroup("MouseControls");
  m_listMouseControls[0] = m_pSettings->value("PlaceTower",
                                              Qt::LeftButton).toUInt();
  m_listMouseControls[1] = m_pSettings->value("SelectTower",
                                              Qt::RightButton).toUInt();
  m_pUi->cbPlaceTower->setCurrentIndex(
        m_listMouseButtons.indexOf(m_listMouseControls[0]));
  m_pUi->cbSelectTower->setCurrentIndex(
        m_listMouseButtons.indexOf(m_listMouseControls[1]));
  m_pSettings->endGroup();

  m_bgColor = this->readColor("BgColor", "#EEEEEC");
  m_highlightColor = this->readColor("HighlightColor", "#8ae234");
  m_highlightBorderColor = this->readColor("HighlightBorderColor", "#888A85");
  m_selectedColor = this->readColor("SelectedColor", "#fce94f");
  m_selectedBorderColor = this->readColor("SelectedBorderColor", "#000000");
  m_animateColor = this->readColor("AnimateColor", "#fce94f");
  m_animateBorderColor = this->readColor("AnimateBorderColor", "#000000");
  m_bgBoardColor = this->readColor("BgBoardColor", "#FFFFFF");
  m_outlineBoardColor = this->readColor("OutlineBoardColor", "#2E3436");
  m_gridBoardColor = this->readColor("GridBoardColor", "#888A85");
  m_neighboursColor = this->readColor("NeighboursColor", "#ad7fa8");
  m_neighboursBorderColor = this->readColor("NeighboursBorderColor", "#000000");
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

QColor CSettings::readColor(const QString sKey, const QString sFallback) {
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

QString CSettings::getNameP1() {
  return m_sNameP1;
}
QString CSettings::getNameP2() {
  return m_sNameP2;
}
QString CSettings::getP2HumanCpu() {
  return m_sP2HumanCpu;
}
quint8 CSettings::getStartPlayer() {
  return m_nStartPlayer;
}
quint8 CSettings::getWinTowers() {
  return m_nWinTowers;
}
bool CSettings::getShowPossibleMoveTowers() {
  return m_bShowPossibleMoveTowers;
}
QList<quint8> CSettings::getMouseControls() const {
  return m_listMouseControls;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

QColor CSettings::getBgColor() const {
  return m_bgColor;
}
QColor CSettings::getHighlightColor() const {
  return m_highlightColor;
}
QColor CSettings::getHighlightBorderColor() const {
  return m_highlightBorderColor;
}
QColor CSettings::getSelectedColor() const {
  return m_selectedColor;
}
QColor CSettings::getSelectedBorderColor() const {
  return m_selectedBorderColor;
}
QColor CSettings::getAnimateColor() const {
  return m_animateColor;
}
QColor CSettings::getAnimateBorderColor() const {
  return m_animateBorderColor;
}
QColor CSettings::getBgBoardColor() const {
  return m_bgBoardColor;
}
QColor CSettings::getOutlineBoardColor() const {
  return m_outlineBoardColor;
}
QColor CSettings::getGridBoardColor() const {
  return m_gridBoardColor;
}
QColor CSettings::GetNeighboursColor() const {
  return m_neighboursColor;
}
QColor CSettings::GetNeighboursBorderColor() const {
  return m_neighboursBorderColor;
}
