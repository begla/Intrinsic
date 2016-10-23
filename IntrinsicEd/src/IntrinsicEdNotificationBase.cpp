// Intrinsic
// Copyright (c) 2016 Benjamin Glatzel
//
// This program is free software : you can redistribute it and / or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

// Precompiled header file
#include "stdafx.h"
#include "stdafx_editor.h"

IntrinsicEdNotificationBase::IntrinsicEdNotificationBase(QWidget* p_Parent,
                                                         float p_TimeToLive)
    : QWidget(p_Parent), _timeToLive(p_TimeToLive)
{
  setWindowFlags(Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus |
                 Qt::WindowStaysOnTopHint | Qt::Tool);
  setAttribute(Qt::WA_ShowWithoutActivating);

  setWindowOpacity(0.0f);

  _timeAliveInSeconds = 0.0f;

  _timer = new QTimer();
  _timer->setInterval(16);

  connect(_timer, SIGNAL(timeout()), this, SLOT(onTick()));

  positionOnScreen();
  _timer->start();

  show();
}

IntrinsicEdNotificationBase::~IntrinsicEdNotificationBase() {}

void IntrinsicEdNotificationBase::positionOnScreen()
{
  const QPoint parentBottomRight =
      ((QWidget*)parent())->geometry().bottomRight();
  setGeometry(parentBottomRight.x() - size().width(),
              parentBottomRight.y() - size().height(), size().width(),
              size().height());
}

void IntrinsicEdNotificationBase::onTick()
{
  static const float fadeTime = 0.1f;

  // Keep the popup alive if the user hovers over it
  if (!geometry().contains(QCursor::pos()))
  {
    _timeAliveInSeconds += 0.016666666f;
  }
  else
  {
    _timeAliveInSeconds = fadeTime;
  }

  positionOnScreen();

  const float timeRemaining = _timeToLive - _timeAliveInSeconds;
  if (_timeAliveInSeconds <= _timeToLive - fadeTime)
  {
    // Fade in
    const float fadeFactor = _timeAliveInSeconds / fadeTime;
    setWindowOpacity(fadeFactor);
  }
  else if (timeRemaining <= fadeTime)
  {
    // Fade out and close
    const float fadeFactor = timeRemaining / fadeTime;
    setWindowOpacity(fadeFactor);

    if (timeRemaining <= 0.0f)
    {
      _timer->stop();

      // Creepy
      delete this;
    }
  }
}
