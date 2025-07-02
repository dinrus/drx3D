// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

struct ICharacterInstance;

class QString;

QString CreateCDF(
  const QString& skeletonFilePath,
  const QString& skinFilePath,
  const QString& materialFilePath);

ICharacterInstance* CreateTemporaryCharacter(
  const QString& skeletonFilePath,
  const QString& skinFilePath,
  const QString& materialFilePath);

