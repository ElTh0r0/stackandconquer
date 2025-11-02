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
 * Settings class.
 */

#include "./settings.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>

#if defined __linux__
const QString Settings::CONF_FILE_EXT = QStringLiteral(".conf");
#else
const QString Settings::CONF_FILE_EXT = QStringLiteral(".ini");
#endif
const QString Settings::SAVE_FILE_EXT(QStringLiteral(".stacksav"));
const QString Settings::CPU_FILE_EXT(QStringLiteral(".js"));
const QString Settings::BOARD_FILE_EXT(QStringLiteral(".stackboard"));
const QString Settings::BORD_IN_FILE_EXT(QStringLiteral(".in"));
const QString Settings::FIELD_IN(QStringLiteral("0"));
const QString Settings::FIELD_OUT(QStringLiteral("#"));
const QString Settings::FIELD_PAD(QStringLiteral("-"));

Settings *Settings::instance() {
  static Settings _instance;
  return &_instance;
}

Settings::Settings(QObject *pParent)
    : QObject(pParent),
#if defined __linux__
      m_settings(QSettings::NativeFormat, QSettings::UserScope,
                 qApp->applicationName().toLower(),
                 qApp->applicationName().toLower()),
#else
      m_settings(QSettings::IniFormat, QSettings::UserScope,
                 qApp->applicationName().toLower(),
                 qApp->applicationName().toLower()),
#endif
      m_nMaxPlayers(2),
      m_DefaultPlayerColors{"#EF2929", "#FCAF3E", "#729FCF", "#8F5902"},
      m_nDefaultGridSize(70),  // Default stone SVG size fits to grid size of 70
      m_nMaxGridSize(200) {
  this->copyDefaultStyles();
  this->loadBoardStyle(this->getBoardStyleFile());

  // Remove deprecated settings
  m_settings.remove(QStringLiteral("Colors"));
  m_settings.remove(QStringLiteral("NameP1"));
  m_settings.remove(QStringLiteral("NameP2"));
  m_settings.remove(QStringLiteral("P1HumanCpu"));
  m_settings.remove(QStringLiteral("P2HumanCpu"));
}

// ----------------------------------------------------------------------------

void Settings::setSharePath(const QString &sPath) { m_sSharePath = sPath; }

auto Settings::getSharePath() const -> QString { return m_sSharePath; }

auto Settings::getConfigPath() const -> QString {
  QFileInfo fi(m_settings.fileName());
  return fi.absolutePath();
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void Settings::copyDefaultStyles() {
  QFileInfo fi(m_settings.fileName());
  QDir confDir(fi.absolutePath());
  if (!confDir.exists()) {
    confDir.mkpath(confDir.absolutePath());
  }

  QFile stylefile(confDir.absolutePath() + "/standard-style" + CONF_FILE_EXT);
  if (!stylefile.exists()) {
    if (QFile::copy(
            QStringLiteral(":/boardstyles/standard-style.conf"),
            confDir.absolutePath() + "/standard-style" + CONF_FILE_EXT)) {
      stylefile.setPermissions(stylefile.permissions() |
                               QFileDevice::WriteUser);
    } else {
      qWarning() << "Couldn't create style file: " << stylefile.fileName();
    }
  }

  stylefile.setFileName(confDir.absolutePath() + "/dark-style" + CONF_FILE_EXT);
  if (!stylefile.exists()) {
    if (QFile::copy(QStringLiteral(":/boardstyles/dark-style.conf"),
                    confDir.absolutePath() + "/dark-style" + CONF_FILE_EXT)) {
      stylefile.setPermissions(stylefile.permissions() |
                               QFileDevice::WriteUser);
    } else {
      qWarning() << "Couldn't create style file: " << stylefile.fileName();
    }
  }
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// PLAYERS

auto Settings::getNumOfPlayers() -> quint8 {
  int nNumOfPlayers =
      m_settings.value(QStringLiteral("NumOfPlayers"), 2).toInt();
  if (nNumOfPlayers > this->getMaxNumOfPlayers()) {
    nNumOfPlayers = this->getMaxNumOfPlayers();
    this->setNumOfPlayers(nNumOfPlayers);
  }
  return nNumOfPlayers;
}

void Settings::setNumOfPlayers(const quint8 nNumOfPlayers) {
  if (nNumOfPlayers > this->getMaxNumOfPlayers()) {
    m_settings.setValue(QStringLiteral("NumOfPlayers"),
                        this->getMaxNumOfPlayers());
  } else {
    m_settings.setValue(QStringLiteral("NumOfPlayers"), nNumOfPlayers);
  }
}

auto Settings::getMaxNumOfPlayers() const -> quint8 { return m_nMaxPlayers; }

// ----------------------------------------------------------------------------

auto Settings::getPlayerCpuScript(const quint8 nPlayer) -> QString {
  if (nPlayer < this->getMaxNumOfPlayers()) {
    m_settings.beginGroup(QStringLiteral("Player") +
                          QString::number(nPlayer + 1));
    QString sHumanCpu =
        m_settings.value(QStringLiteral("HumanCpu"), QStringLiteral("Human"))
            .toString();
    m_settings.endGroup();

    if ("Human" == sHumanCpu) {
      sHumanCpu.clear();
    }
    return sHumanCpu;
  }

  qWarning()
      << "Player array length exceeded - requested CPU script for nPlayer:"
      << nPlayer;
  qWarning() << "Max players:" << this->getMaxNumOfPlayers();
  return QLatin1String("");
}

void Settings::setPlayerCpuScript(const quint8 nPlayer,
                                  const QString &sHumanCpu) {
  m_settings.beginGroup(QStringLiteral("Player") +
                        QString::number(nPlayer + 1));
  m_settings.setValue(QStringLiteral("HumanCpu"), sHumanCpu);
  m_settings.endGroup();
}

void Settings::setAvailableCpuScripts(
    const QHash<QString, QString> &CpuScripts) {
  m_CpuScripts = CpuScripts;
}

auto Settings::getPlayerCpuScriptFile(const quint8 nPlayer) -> QString {
  if (nPlayer < this->getMaxNumOfPlayers()) {
    if (m_CpuScripts.contains(this->getPlayerCpuScript(nPlayer))) {
      return m_CpuScripts.value(this->getPlayerCpuScript(nPlayer));
    } else if (!this->getPlayerCpuScript(nPlayer).isEmpty()) {
      qWarning() << "m_CpuScripts doesn't contain"
                 << this->getPlayerCpuScript(nPlayer);
    }
  } else {
    qWarning() << "Player array length exceeded - requested CPU script file "
                  "for nPlayer:"
               << nPlayer;
  }

  return QLatin1String("");
}

// ----------------------------------------------------------------------------

auto Settings::getPlayerColor(const quint8 nPlayer) -> QString {
  if (nPlayer < this->getMaxNumOfPlayers() &&
      nPlayer < m_DefaultPlayerColors.size()) {
    m_settings.beginGroup(QStringLiteral("Player") +
                          QString::number(nPlayer + 1));
    QString sColor = this->readColor(m_settings, QStringLiteral("Color"),
                                     m_DefaultPlayerColors[nPlayer])
                         .name();
    m_settings.endGroup();
    return sColor;
  }

  qWarning() << "Player array length exceeded - requested color for nPlayer:"
             << nPlayer;
  qWarning() << "Max players:" << this->getMaxNumOfPlayers()
             << "Default colors:" << m_DefaultPlayerColors;
  return m_DefaultPlayerColors[0];
}

void Settings::setPlayerColor(const quint8 nPlayer, const QString &sColor) {
  QColor color(sColor);
  if (!color.isValid()) {
    qWarning() << "User chose an invalid stone color:" << sColor;
    if (nPlayer < m_DefaultPlayerColors.size()) {
#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
      color.setNamedColor(m_DefaultPlayerColors[nPlayer]);
#else
      color = QColor::fromString(m_DefaultPlayerColors[nPlayer]);
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

  m_settings.beginGroup(QStringLiteral("Player") +
                        QString::number(nPlayer + 1));
  m_settings.setValue(QStringLiteral("Color"), color.name());
  m_settings.endGroup();
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// GAME SETTINGS

auto Settings::getStartPlayer() const -> quint8 {
  return static_cast<quint8>(
      m_settings.value(QStringLiteral("StartPlayer"), 1).toInt());
}

void Settings::setStartPlayer(const quint8 nStartPlayer) {
  m_settings.setValue(QStringLiteral("StartPlayer"), nStartPlayer);
}

// ----------------------------------------------------------------------------

auto Settings::getBoardFile() -> QString {
  static const QString sDefaultBoard(
      m_sSharePath + QStringLiteral("/boards/Square_5x5") + BOARD_FILE_EXT);

  QString sBoard =
      m_settings.value(QStringLiteral("Board"), sDefaultBoard).toString();
  if (!QFile::exists(sBoard)) {
    qWarning() << "Board from conf file not found:" << sBoard;
    if (sBoard != sDefaultBoard) {
      if (QFile::exists(sDefaultBoard)) {
        qWarning() << "Switching to default board!";
        sBoard = sDefaultBoard;
        this->setBoardFile(sBoard);
      } else {
        qWarning() << "No valid board found!";
        sBoard.clear();
      }
    }
  }

  return sBoard;
}
void Settings::setBoardFile(const QString &sBoardFile) {
  m_settings.setValue(QStringLiteral("Board"), sBoardFile);
}

// ----------------------------------------------------------------------------

auto Settings::getTowersToWin() const -> quint8 {
  return static_cast<quint8>(
      m_settings.value(QStringLiteral("NumWinTowers"), 1).toInt());
}

void Settings::setTowersToWin(const quint8 nTowersToWin) {
  m_settings.setValue(QStringLiteral("NumWinTowers"), nTowersToWin);
}

// ----------------------------------------------------------------------------

auto Settings::getShowPossibleMoveTowers() const -> bool {
  return m_settings.value(QStringLiteral("ShowPossibleMoveTowers"), true)
      .toBool();
}

void Settings::setShowPossibleMoveTowers(const bool bShowMoves) {
  m_settings.setValue(QStringLiteral("ShowPossibleMoveTowers"), bShowMoves);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// GRID

auto Settings::getGridSize() const -> quint16 {
  int nGridSize =
      m_settings.value(QStringLiteral("GridSize"), this->getDefaultGridSize())
          .toInt();

  if (nGridSize < this->getDefaultGridSize()) {
    nGridSize = this->getDefaultGridSize();
  } else if (nGridSize > this->getMaxGridSize()) {
    nGridSize = this->getMaxGridSize();
  }

  return nGridSize;
}

void Settings::setGridSize(const quint16 nNewGrid) {
  int nGridSize;

  if (nNewGrid < m_nDefaultGridSize) {
    nGridSize = m_nDefaultGridSize;
  } else if (nNewGrid > m_nMaxGridSize) {
    nGridSize = m_nMaxGridSize;
  } else {
    nGridSize = nNewGrid;
  }

  m_settings.setValue(QStringLiteral("GridSize"), nGridSize);
}

auto Settings::getDefaultGridSize() const -> qreal {
  return m_nDefaultGridSize;
}

auto Settings::getMaxGridSize() const -> quint16 { return m_nMaxGridSize; }

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// LANGUAGE

auto Settings::getGuiLanguage() -> QString {
  QString sGuiLanguage =
      m_settings.value(QStringLiteral("GuiLanguage"), QStringLiteral("auto"))
          .toString()
          .toLower();

  // Automatically detected language
  if ("auto" == sGuiLanguage) {
#ifdef Q_OS_UNIX
    QByteArray lang = qgetenv("LANG");
    if (!lang.isEmpty()) {
      return QLocale(QString::fromLatin1(lang)).name();
    }
#endif
    return QLocale::system().name();
  }

  // Specific language selected
  if (!QFile(":/" + qApp->applicationName().toLower() + "_" + sGuiLanguage +
             ".qm")
           .exists() &&
      !QFile(m_sSharePath + "/lang/" + qApp->applicationName().toLower() + "_" +
             sGuiLanguage + ".qm")
           .exists()) {
    sGuiLanguage = QStringLiteral("en");
    this->setGuiLanguage(sGuiLanguage);
  }

  return sGuiLanguage;
}

void Settings::setGuiLanguage(const QString &sLanguage) {
  m_settings.setValue(QStringLiteral("GuiLanguage"), sLanguage);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// BOARD STYLE

auto Settings::getBoardStyleFile() const -> QString {
  return m_settings
      .value(QStringLiteral("BoardStyle"), QStringLiteral("standard-style"))
      .toString();
}

void Settings::setBoardStyleFile(const QString &sBoardStyleFile) {
  m_settings.setValue(QStringLiteral("BoardStyle"), sBoardStyleFile);
  this->loadBoardStyle(sBoardStyleFile);
}

auto Settings::createNewBoardStyleFile(const QString &sNewFile) const -> bool {
  QFile currentStyleFile(this->getConfigPath() + "/" +
                         this->getBoardStyleFile() + CONF_FILE_EXT);
  QString newStyleFile(this->getConfigPath() + "/" + sNewFile + CONF_FILE_EXT);

  bool bRet = currentStyleFile.copy(newStyleFile);
  if (!bRet) {
    qWarning() << "Could not create new style file:";
    qWarning() << "Org:" << currentStyleFile.fileName();
    qWarning() << "Copy:" << newStyleFile;
  }
  return bRet;
}

// ---------------------------------------------------------------------------

void Settings::loadBoardStyle(const QString &sStyleFile) {
#if defined __linux__
  QSettings StyleSet(QSettings::NativeFormat, QSettings::UserScope,
                     qApp->applicationName().toLower(), sStyleFile);
#else
  QSettings StyleSet(QSettings::IniFormat, QSettings::UserScope,
                     qApp->applicationName().toLower(), sStyleFile);
#endif
  m_BoardStyle.clear();

  if (!QFile::exists(StyleSet.fileName())) {
    qWarning() << "Could not find/open board style file:"
               << StyleSet.fileName();
    qWarning() << "Loading fallback style colors!";
  }

  StyleSet.beginGroup(QStringLiteral("Colors"));
  m_BoardStyle[QStringLiteral("BgColor")] =
      readColor(StyleSet, QStringLiteral("BgColor"), QStringLiteral("#EEEEEC"));

  m_BoardStyle[QStringLiteral("BgBoardColor")] = readColor(
      StyleSet, QStringLiteral("BgBoardColor"), QStringLiteral("#FFFFFF"));

  m_BoardStyle[QStringLiteral("GridBoardColor")] = readColor(
      StyleSet, QStringLiteral("GridBoardColor"), QStringLiteral("#888A85"));

  m_BoardStyle[QStringLiteral("AnimateColor")] = readColor(
      StyleSet, QStringLiteral("AnimateColor"), QStringLiteral("#fce94f"));

  m_BoardStyle[QStringLiteral("AnimateBorderColor")] =
      readColor(StyleSet, QStringLiteral("AnimateBorderColor"),
                QStringLiteral("#000000"));

  m_BoardStyle[QStringLiteral("HighlightColor")] = readColor(
      StyleSet, QStringLiteral("HighlightColor"), QStringLiteral("#8ae234"));

  m_BoardStyle[QStringLiteral("HighlightBorderColor")] =
      readColor(StyleSet, QStringLiteral("HighlightBorderColor"),
                QStringLiteral("#888A85"));

  m_BoardStyle[QStringLiteral("NeighboursColor")] = readColor(
      StyleSet, QStringLiteral("NeighboursColor"), QStringLiteral("#ad7fa8"));

  m_BoardStyle[QStringLiteral("NeighboursBorderColor")] =
      readColor(StyleSet, QStringLiteral("NeighboursBorderColor"),
                QStringLiteral("#000000"));

  m_BoardStyle[QStringLiteral("SelectedColor")] = readColor(
      StyleSet, QStringLiteral("SelectedColor"), QStringLiteral("#fce94f"));

  m_BoardStyle[QStringLiteral("SelectedBorderColor")] =
      readColor(StyleSet, QStringLiteral("SelectedBorderColor"),
                QStringLiteral("#000000"));

  m_BoardStyle[QStringLiteral("TextColor")] = readColor(
      StyleSet, QStringLiteral("TextColor"), QStringLiteral("#000000"));

  m_BoardStyle[QStringLiteral("TextHighlightColor")] =
      readColor(StyleSet, QStringLiteral("TextHighlightColor"),
                QStringLiteral("#FF0000"));
  StyleSet.endGroup();
}

// ----------------------------------------------------------------------------

void Settings::saveBoardStyle() const {
#if defined __linux__
  QSettings StyleSet(QSettings::NativeFormat, QSettings::UserScope,
                     qApp->applicationName().toLower(),
                     this->getBoardStyleFile());
#else
  QSettings StyleSet(QSettings::IniFormat, QSettings::UserScope,
                     qApp->applicationName().toLower(),
                     this->getBoardStyleFile());
#endif

  StyleSet.beginGroup(QStringLiteral("Colors"));
  StyleSet.setValue(QStringLiteral("BgColor"),
                    m_BoardStyle[QStringLiteral("BgColor")].name());
  StyleSet.setValue(QStringLiteral("BgBoardColor"),
                    m_BoardStyle[QStringLiteral("BgBoardColor")].name());
  StyleSet.setValue(QStringLiteral("GridBoardColor"),
                    m_BoardStyle[QStringLiteral("GridBoardColor")].name());
  StyleSet.setValue(QStringLiteral("AnimateColor"),
                    m_BoardStyle[QStringLiteral("AnimateColor")].name());
  StyleSet.setValue(QStringLiteral("AnimateBorderColor"),
                    m_BoardStyle[QStringLiteral("AnimateBorderColor")].name());
  StyleSet.setValue(QStringLiteral("HighlightColor"),
                    m_BoardStyle[QStringLiteral("HighlightColor")].name());
  StyleSet.setValue(
      QStringLiteral("HighlightBorderColor"),
      m_BoardStyle[QStringLiteral("HighlightBorderColor")].name());
  StyleSet.setValue(QStringLiteral("NeighboursColor"),
                    m_BoardStyle[QStringLiteral("NeighboursColor")].name());
  StyleSet.setValue(
      QStringLiteral("NeighboursBorderColor"),
      m_BoardStyle[QStringLiteral("NeighboursBorderColor")].name());
  StyleSet.setValue(QStringLiteral("SelectedColor"),
                    m_BoardStyle[QStringLiteral("SelectedColor")].name());
  StyleSet.setValue(QStringLiteral("SelectedBorderColor"),
                    m_BoardStyle[QStringLiteral("SelectedBorderColor")].name());
  StyleSet.setValue(QStringLiteral("TextColor"),
                    m_BoardStyle[QStringLiteral("TextColor")].name());
  StyleSet.setValue(QStringLiteral("TextHighlightColor"),
                    m_BoardStyle[QStringLiteral("TextHighlightColor")].name());
  StyleSet.endGroup();
}

// ----------------------------------------------------------------------------

auto Settings::getBgColor() const -> QColor {
  return m_BoardStyle[QStringLiteral("BgColor")];
}
void Settings::setBgColor(const QColor Color) {
  m_BoardStyle[QStringLiteral("BgColor")] = Color;
}

auto Settings::getTextColor() const -> QColor {
  return m_BoardStyle[QStringLiteral("TextColor")];
}
void Settings::setTextColor(const QColor Color) {
  m_BoardStyle[QStringLiteral("TextColor")] = Color;
}
auto Settings::getTextHighlightColor() const -> QColor {
  return m_BoardStyle[QStringLiteral("TextHighlightColor")];
}
void Settings::setTextHighlightColor(const QColor Color) {
  m_BoardStyle[QStringLiteral("TextHighlightColor")] = Color;
}

auto Settings::getHighlightColor() const -> QColor {
  return m_BoardStyle[QStringLiteral("HighlightColor")];
}
void Settings::setHighlightColor(const QColor Color) {
  m_BoardStyle[QStringLiteral("HighlightColor")] = Color;
}
auto Settings::getHighlightBorderColor() const -> QColor {
  return m_BoardStyle[QStringLiteral("HighlightBorderColor")];
}
void Settings::setHighlightBorderColor(const QColor Color) {
  m_BoardStyle[QStringLiteral("HighlightBorderColor")] = Color;
}

auto Settings::getSelectedColor() const -> QColor {
  return m_BoardStyle[QStringLiteral("SelectedColor")];
}
void Settings::setSelectedColor(const QColor Color) {
  m_BoardStyle[QStringLiteral("SelectedColor")] = Color;
}
auto Settings::getSelectedBorderColor() const -> QColor {
  return m_BoardStyle[QStringLiteral("SelectedBorderColor")];
}
void Settings::setSelectedBorderColor(const QColor Color) {
  m_BoardStyle[QStringLiteral("SelectedBorderColor")] = Color;
}

auto Settings::getAnimateColor() const -> QColor {
  return m_BoardStyle[QStringLiteral("AnimateColor")];
}
void Settings::setAnimateColor(const QColor Color) {
  m_BoardStyle[QStringLiteral("AnimateColor")] = Color;
}
auto Settings::getAnimateBorderColor() const -> QColor {
  return m_BoardStyle[QStringLiteral("AnimateBorderColor")];
}
void Settings::setAnimateBorderColor(const QColor Color) {
  m_BoardStyle[QStringLiteral("AnimateBorderColor")] = Color;
}

auto Settings::getBgBoardColor() const -> QColor {
  return m_BoardStyle[QStringLiteral("BgBoardColor")];
}
void Settings::setBgBoardColor(const QColor Color) {
  m_BoardStyle[QStringLiteral("BgBoardColor")] = Color;
}
auto Settings::getGridBoardColor() const -> QColor {
  return m_BoardStyle[QStringLiteral("GridBoardColor")];
}
void Settings::setGridBoardColor(const QColor Color) {
  m_BoardStyle[QStringLiteral("GridBoardColor")] = Color;
}

auto Settings::getNeighboursColor() const -> QColor {
  return m_BoardStyle[QStringLiteral("NeighboursColor")];
}
void Settings::setNeighboursColor(const QColor Color) {
  m_BoardStyle[QStringLiteral("NeighboursColor")] = Color;
}
auto Settings::getNeighboursBorderColor() const -> QColor {
  return m_BoardStyle[QStringLiteral("NeighboursBorderColor")];
}
void Settings::setNeighboursBorderColor(const QColor Color) {
  m_BoardStyle[QStringLiteral("NeighboursBorderColor")] = Color;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

auto Settings::readColor(const QSettings &StyleSet, const QString &sKey,
                         const QString &sFallback) const -> QColor {
  QString sValue = StyleSet.value(sKey, sFallback).toString();
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
