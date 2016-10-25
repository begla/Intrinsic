// Copyright 2016 Benjamin Glatzel
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
