#pragma once

#include "common/common_pch.h"

#include <QList>
#include <QString>
#include <QStringList>
#include <QVariant>

#include "mkvtoolnix-gui/util/config_file.h"

#define MTXCFG_VERSION 1

class QTemporaryFile;

namespace mtx { namespace gui { namespace Merge {

class InvalidSettingsX: public exception {
public:
  virtual const char *what() const throw() {
    return "invalid settings in file";
  }
};

class Attachment;
class MuxConfig;
class SourceFile;
class Track;

using AttachmentPtr = std::shared_ptr<Attachment>;
using MuxConfigPtr  = std::shared_ptr<MuxConfig>;
using SourceFilePtr = std::shared_ptr<SourceFile>;
using TrackPtr      = std::shared_ptr<Track>;

class MuxConfig {
public:
  struct Loader {
    Util::ConfigFile &settings;
    QHash<qulonglong, SourceFile *> &objectIDToSourceFile;
    QHash<qulonglong, Track *> &objectIDToTrack;

    Loader(Util::ConfigFile &p_settings,
           QHash<qulonglong, SourceFile *> &p_objectIDToSourceFile,
           QHash<qulonglong, Track *> &p_objectIDToTrack)
      : settings(p_settings)
      , objectIDToSourceFile(p_objectIDToSourceFile)
      , objectIDToTrack(p_objectIDToTrack)
    {
    }
  };

  enum SplitMode {
    DoNotSplit = 0,
    SplitAfterSize,
    SplitAfterDuration,
    SplitAfterTimestamps,
    SplitByParts,
    SplitByPartsFrames,
    SplitByFrames,
    SplitAfterChapters,
  };

  enum class ChapterGenerationMode {
    None = 0,
    WhenAppending,
    Intervals,
  };

public:
  QString m_configFileName;

  QList<SourceFilePtr> m_files;
  QList<Track *> m_tracks;
  QList<AttachmentPtr> m_attachments;

  QString m_firstInputFileName;
  QString m_title, m_destination, m_destinationAuto, m_globalTags, m_segmentInfo, m_splitOptions;
  QString m_segmentUIDs, m_previousSegmentUID, m_nextSegmentUID, m_chapters, m_chapterLanguage, m_chapterCharacterSet, m_chapterCueNameFormat, m_additionalOptions;
  SplitMode m_splitMode;
  unsigned int m_splitMaxFiles;
  bool m_linkFiles, m_webmMode;

  ChapterGenerationMode m_chapterGenerationMode;
  QString m_chapterGenerationNameTemplate, m_chapterGenerationInterval;

public:
  MuxConfig(QString const &fileName = QString{""});
  MuxConfig(MuxConfig const &other);
  virtual ~MuxConfig();
  MuxConfig &operator =(MuxConfig const &other);

  virtual void load(Util::ConfigFile &settings);
  virtual void load(QString const &fileName = QString{""});
  virtual void save(Util::ConfigFile &settings) const;
  virtual void save(QString const &fileName = QString{""});
  virtual void reset();

  QString toString() const;
  QStringList buildMkvmergeOptions() const;

  virtual bool hasSourceFileWithTitle() const;

  virtual void debugDumpFileList() const;
  virtual void debugDumpTrackList() const;

protected:
  QHash<SourceFile *, unsigned int> buildFileNumbers() const;
  QStringList buildTrackOrder(QHash<SourceFile *, unsigned int> const &fileNumbers) const;
  QStringList buildAppendToMapping(QHash<SourceFile *, unsigned int> const &fileNumbers) const;

public:
  static MuxConfigPtr loadSettings(QString const &fileName);
  static void saveProperties(Util::ConfigFile &settings, QVariantMap const &properties);
  static void loadProperties(Util::ConfigFile &settings, QVariantMap &properties);
  static void debugDumpSpecificTrackList(QList<Track *> const &tracks);
  static QString settingsType();
  static QString determineFirstInputFileName(QList<SourceFilePtr> const &files);
};

template<typename T>
void
loadSettingsGroup(char const *group,
                  QList<std::shared_ptr<T> > &container,
                  MuxConfig::Loader &l) {
  l.settings.beginGroup(group);

  int numberOfEntries = std::max(l.settings.value("numberOfEntries").toInt(), 0);
  for (int idx = 0; idx < numberOfEntries; ++idx) {
    container << std::make_shared<T>();
    l.settings.beginGroup(QString::number(idx));
    container.back()->loadSettings(l);
    l.settings.endGroup();
  }

  l.settings.endGroup();
}

template<typename T>
void
saveSettingsGroup(char const *group,
                  QList<std::shared_ptr<T> > const &container,
                  Util::ConfigFile &settings) {
  settings.beginGroup(group);

  int numberOfEntries = container.size();
  settings.setValue("numberOfEntries", numberOfEntries);

  for (int idx = 0; idx < numberOfEntries; ++idx) {
    settings.beginGroup(QString::number(idx));
    container.at(idx)->saveSettings(settings);
    settings.endGroup();
  }

  settings.endGroup();
}

}}}
