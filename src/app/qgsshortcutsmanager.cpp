/***************************************************************************
    qgsshortcutsmanager.cpp
    ---------------------
    begin                : May 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsshortcutsmanager.h"

#include <QSettings>

QgsShortcutsManager* QgsShortcutsManager::mInstance = nullptr;

QgsShortcutsManager::QgsShortcutsManager( QObject *parent ) : QObject( parent )
{
}

QgsShortcutsManager::~QgsShortcutsManager()
{
  mInstance = nullptr;
}

QgsShortcutsManager* QgsShortcutsManager::instance( QObject *parent )
{
  if ( !mInstance )
    mInstance = new QgsShortcutsManager( parent );
  return mInstance;
}

bool QgsShortcutsManager::registerAction( QAction* action, const QString& defaultShortcut )
{
  mActions.insert( action, defaultShortcut );
  connect( action, SIGNAL( destroyed() ), this, SLOT( actionDestroyed() ) );

  QString actionText = action->text();
  actionText.remove( '&' ); // remove the accelerator

  // load overridden value from settings
  QSettings settings;
  QString shortcut = settings.value( "/shortcuts/" + actionText, defaultShortcut ).toString();

  action->setShortcut( shortcut );

  return true;
}

bool QgsShortcutsManager::unregisterAction( QAction* action )
{
  mActions.remove( action );
  return true;
}

QList<QAction*> QgsShortcutsManager::listActions()
{
  return mActions.keys();
}

QString QgsShortcutsManager::actionDefaultShortcut( QAction* action )
{
  if ( !mActions.contains( action ) )
    return QString();

  return mActions.value( action );
}

bool QgsShortcutsManager::setActionShortcut( QAction* action, const QString& shortcut )
{
  action->setShortcut( shortcut );

  QString actionText = action->text();
  actionText.remove( '&' ); // remove the accelerator

  // save to settings
  QSettings settings;
  settings.setValue( "/shortcuts/" + actionText, shortcut );
  return true;
}

QAction* QgsShortcutsManager::actionForShortcut( const QKeySequence& s )
{
  if ( s.isEmpty() )
    return nullptr;

  for ( ActionsHash::const_iterator it = mActions.constBegin(); it != mActions.constEnd(); ++it )
  {
    if ( it.key()->shortcut() == s )
      return it.key();
  }

  return nullptr;
}

QAction* QgsShortcutsManager::actionByName( const QString& name )
{
  for ( ActionsHash::const_iterator it = mActions.constBegin(); it != mActions.constEnd(); ++it )
  {
    if ( it.key()->text() == name )
      return it.key();
  }

  return nullptr;
}

void QgsShortcutsManager::registerAllChildrenActions( QObject* object )
{
  Q_FOREACH ( QObject* child, object->children() )
  {
    if ( child->inherits( "QAction" ) )
    {
      QAction* a = qobject_cast<QAction*>( child );
      registerAction( a, a->shortcut() );
    }
  }
}

void QgsShortcutsManager::actionDestroyed()
{
  mActions.remove( static_cast<QAction*>( sender() ) );
}
