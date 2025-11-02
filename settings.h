// SPDX-FileCopyrightText: 2016-2025 Thorsten Roth
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <QColor>
#include <QHash>
#include <QSettings>

class Settings : public QObject {
  Q_OBJECT

 public:
  static Settings *instance();
  void setSharePath(const QString &sPath);
  auto getSharePath() const -> QString;
  auto getConfigPath() const -> QString;

  // Players
  auto getNumOfPlayers() -> quint8;
  void setNumOfPlayers(const quint8 nNumOfPlayers);
  auto getMaxNumOfPlayers() const -> quint8;
  auto getPlayerCpuScript(const quint8 nPlayer) -> QString;
  void setPlayerCpuScript(const quint8 nPlayer, const QString &sHumanCpu);
  void setAvailableCpuScripts(const QHash<QString, QString> &CpuScripts);
  auto getPlayerCpuScriptFile(const quint8 nPlayer) -> QString;
  auto getPlayerColor(const quint8 nPlayer) -> QString;
  void setPlayerColor(const quint8 nPlayer, const QString &sColor);

  // Game settings
  auto getStartPlayer() const -> quint8;
  void setStartPlayer(const quint8 nStartPlayer);
  auto getBoardFile() -> QString;
  void setBoardFile(const QString &sBoardFile);
  auto getTowersToWin() const -> quint8;
  void setTowersToWin(const quint8 nTowersToWin);
  auto getShowPossibleMoveTowers() const -> bool;
  void setShowPossibleMoveTowers(const bool bShowMoves);

  // Grid
  auto getGridSize() const -> quint16;
  void setGridSize(const quint16 nNewGrid);
  auto getDefaultGridSize() const -> qreal;
  auto getMaxGridSize() const -> quint16;

  // Language
  auto getGuiLanguage() -> QString;
  void setGuiLanguage(const QString &sLanguage);

  // Board style
  auto getBoardStyleFile() const -> QString;
  void setBoardStyleFile(const QString &sBoardStyleFile);
  void loadBoardStyle(const QString &sStyleFile);
  void saveBoardStyle() const;
  auto createNewBoardStyleFile(const QString &sNewFile) const -> bool;
  auto getBgColor() const -> QColor;
  void setBgColor(const QColor Color);
  auto getTextColor() const -> QColor;
  void setTextColor(const QColor Color);
  auto getTextHighlightColor() const -> QColor;
  void setTextHighlightColor(const QColor Color);
  auto getHighlightColor() const -> QColor;
  void setHighlightColor(const QColor Color);
  auto getHighlightBorderColor() const -> QColor;
  void setHighlightBorderColor(const QColor Color);
  auto getSelectedColor() const -> QColor;
  void setSelectedColor(const QColor Color);
  auto getSelectedBorderColor() const -> QColor;
  void setSelectedBorderColor(const QColor Color);
  auto getAnimateColor() const -> QColor;
  void setAnimateColor(const QColor Color);
  auto getAnimateBorderColor() const -> QColor;
  void setAnimateBorderColor(const QColor Color);
  auto getBgBoardColor() const -> QColor;
  void setBgBoardColor(const QColor Color);
  auto getGridBoardColor() const -> QColor;
  void setGridBoardColor(const QColor Color);
  auto getNeighboursColor() const -> QColor;
  void setNeighboursColor(const QColor Color);
  auto getNeighboursBorderColor() const -> QColor;
  void setNeighboursBorderColor(const QColor Color);

  static const QString CONF_FILE_EXT;
  static const QString SAVE_FILE_EXT;
  static const QString CPU_FILE_EXT;
  static const QString BOARD_FILE_EXT;
  static const QString BORD_IN_FILE_EXT;
  static const QString FIELD_IN;
  static const QString FIELD_OUT;
  static const QString FIELD_PAD;

 private:
  auto readColor(const QSettings &StyleSet, const QString &sKey,
                 const QString &sFallback) const -> QColor;
  void copyDefaultStyles();

  explicit Settings(QObject *pParent = nullptr);
  QSettings m_settings;
  const quint8 m_nMaxPlayers;
  const QStringList m_DefaultPlayerColors;
  const qreal m_nDefaultGridSize;
  const quint16 m_nMaxGridSize;
  QString m_sSharePath;
  QHash<QString, QColor> m_BoardStyle;
  QHash<QString, QString> m_CpuScripts;
};

#endif  // SETTINGS_H_
